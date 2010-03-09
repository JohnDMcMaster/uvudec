/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#ifndef UVD_BINARY_SYMBOL_H
#define UVD_BINARY_SYMBOL_H

#include "uvd_data.h"
#include "uvd_relocation.h"

#include <string>
#include <vector>

//should use RTTI for this instead?
#define UVD__SYMBOL_TYPE__UNKNOWN			0
#define UVD__SYMBOL_TYPE__FUNCTION			1
#define UVD__SYMBOL_TYPE__LABEL				2
#define UVD__SYMBOL_TYPE__ROM				3
#define UVD__SYMBOL_TYPE__VARIABLE			4

/*
A symbol as if found in an object file, not necessarily ELF format
A format easily workable in memory
*/
class UVDBinarySymbol
{
public:
	UVDBinarySymbol();
	virtual ~UVDBinarySymbol();

	/*
	UVDRelocatableElement also has a name, this has caused some conflicts
	
	*/ 
	void setSymbolName(const std::string &name);
	uv_err_t getSymbolName(std::string &name);
	virtual uv_err_t init();
	virtual uv_err_t deinit();
	
	//FIXME: this is analysis specific...should it be here?
	//If this is a symbol in our currently analyzed data, the address it presides at 
	uv_err_t getSymbolAddress(uint32_t *symbolAddress);
	uv_err_t setSymbolAddress(uint32_t symbolAddress);
	/*
	uv_err_t getOffset(uint32_t *offset);
	*/

	virtual uv_err_t getSymbolSize(uint32_t *symbolSize);
	//virtual uv_err_t setSymbolSize(uint32_t symbolSize);
	
	/*
	A relocation inside this relocation, if applicable
	Some symbols are so small this is not practical, such as variables
	A basic form
	The offset is relative to the data inside this function
	*/
	uv_err_t addRelativeRelocation(uint32_t relocatableDataOffset, uint32_t relocatableDataSize);
	//This is equivilent to above, except that the function start is subtracted from the offset
	//Provided as convienence
	uv_err_t addAbsoluteRelocation(uint32_t relocatableDataOffset, uint32_t relocatableDataSize);
	
	/*
	Add a location where this symbol is used using the programs absolute address
	Basic form
	*/
	uv_err_t addSymbolUse(uint32_t relocatableDataOffsetBytes, uint32_t relocatableDataSizeBytes);
	uv_err_t addSymbolUseByBits(uint32_t relocatableDataOffsetBytes, uint32_t relocatableDataSizeBits);
	
	/*
	Add relocations from other symbol to this one
	Used for merging analysis data
	The other symbol will contain all the areas it was referenced
	*/
	uv_err_t addRelocations(const UVDBinarySymbol *otherSymbol);

public:
	//The symbol's (function's/variable's) name
	std::string m_symbolName;
	/*
	The actual data this symbol represents, if its resolved
	If the symbol is not resolved, data will be NULL
	Used mostly by subclasses
	We must be the owner of this
	This should be a form such that it will remain valid until de-init
		Gives several options
		Use a file mapped entry and read the data as needed from disk
		Use a mapped UVDData that assumes target data will remain valid until prog close
		May consider a ref count for UVDData to make some of this easier
	*/
	UVDData *m_data;

	//The relocations contained within this symbol
	//Stuff like where g_debug is used
	//Also stored is a relocatable version of the function
	//Note that all relocation fixups should be of type UVDBinarySymbolElement as they are expected to all be symbol fixups
	UVDRelocatableData *m_relocatableData;
	//Locations where this symbol is used throughout the code, ie not (necessary) in this function
	//Done primarily because early on we only know the symbol destination and must concentrate where it is used
	std::set<UVDRelocationFixup *> m_symbolUsageLocations;

	//Computes the symbol address
	UVDRelocatableElement m_symbolAddress;
	//Computed through m_data
	//UVDRelocatableElement m_symbolSize;	
};

/*
A number of issues of being aggressive with symbols during analysis
In particular, we cannot be sure whether things are functions, labels, etc
	In particular, one may jump back to the start of a loop, which causes issues with analysis
	Must do some sort of voting mentality to get the true symbol
*/
class UVDAnalyzedBinarySymbol : public UVDBinarySymbol
{
public:
	UVDAnalyzedBinarySymbol();
	~UVDAnalyzedBinarySymbol();

	/*
	Return the best guess at what the symbol really is
	*/
	uv_err_t getBestUVDBinarySymbol(UVDBinarySymbol **symbolOut);

	//Register use as a label
	void registerLabelUsage(uint32_t usageAddress);
	//As a function
	void registerFunctionUsage(uint32_t usageAddress);
	//As a variable
	void registerVariableUsage(uint32_t usageAddress);
	
public:
	//How many of each of the particular types 
	std::vector<uint32_t> m_labelHits;
	std::vector<uint32_t> m_functionHits;
	std::vector<uint32_t> m_variableHits;
};

#if 0
Deprecated, see UVDBinaryFunctionInstance
/*
A function is a peice of code with some relocatables in it
If the function is external, it may or may not have a known value
*/
class UVDFunctionBinarySymbol : public UVDBinarySymbol
{
public:
	UVDFunctionBinarySymbol();
	~UVDFunctionBinarySymbol();
};
#endif

/*
A variable is a reserved location to store data
*/
class UVDVariableBinarySymbol : public UVDBinarySymbol
{
public:
	UVDVariableBinarySymbol();
	~UVDVariableBinarySymbol();
};

/*
A label is a location jumped to, ie a relocation based off of IP
Usually they occur only internally to functions and are not referenced outside that function
Exception: BASIC code...yuck
	In this case the decompiler is useless and you are better off reading the assembly.  Good luck! 
*/
class UVDLabelBinarySymbol : public UVDBinarySymbol
{
public:
	UVDLabelBinarySymbol();
	~UVDLabelBinarySymbol();
 };

/*
A symbol of read only data
Their value cannot be changed
ELF notes
	Do not have a true symbol after compilation
	Like anonymous symbols
	These appear in the .bss section
*/
class UVDROMBinarySymbol : public UVDBinarySymbol
{
public:
	UVDROMBinarySymbol();
	~UVDROMBinarySymbol();
};

/*
A collection of symbols belonging to a common analysis
*/
class UVDBinaryFunction;
class UVDAnalyzer;
class UVDBinarySymbolManager
{
public:
	UVDBinarySymbolManager();
	~UVDBinarySymbolManager();

	uv_err_t findSymbolByAddress(uint32_t address, UVDBinarySymbol **symbol);

	uv_err_t findSymbol(std::string &name, UVDBinarySymbol **symbol);
	//If this one is used for analysis, make sure its an analyzed version
	uv_err_t findAnalyzedSymbol(std::string &name, UVDAnalyzedBinarySymbol **symbol);
	uv_err_t addSymbol(UVDBinarySymbol *symbol);
	//Shortcuts for now
	//These will create the function/label if necessary
	/*
	ie from a call
	functionAddress: the call target of the function
	relocatableDataOffset: the address of the data that contains the function address
	relocatableDataSize: the size of the section storing the function address
		If there is not enough room to store, an error will be thrown
	*/
	uv_err_t addAbsoluteFunctionRelocation(uint32_t functionAddress,
			uint32_t relocatableDataOffset, uint32_t relocatableDataSizeBytes);
	uv_err_t addAbsoluteFunctionRelocationByBits(uint32_t functionAddress,
			uint32_t relocatableDataOffset, uint32_t relocatableDataSizeBits);
	//ie from a goto
	uv_err_t addAbsoluteLabelRelocation(uint32_t labelAddress,
			uint32_t relocatableDataOffset, uint32_t relocatableDataSizeBytes);
	uv_err_t addAbsoluteLabelRelocationByBits(uint32_t labelAddress,
			uint32_t relocatableDataOffset, uint32_t relocatableDataSizeBits);

	//Find the function symbol passed in and add all relocations, if any
	uv_err_t collectRelocations(UVDBinaryFunction *function);

private:
	uv_err_t doCollectRelocations(UVDBinaryFunction *function, UVDBinarySymbol *analysisSymbol);

public:
	//Symbol names must be unique
	//Needed to convert addresses to symbol names
	UVDAnalyzer *m_analyzer;

private:
	std::map<std::string, UVDBinarySymbol *> m_symbols;
	std::map<uint32_t, UVDBinarySymbol *> m_symbolsByAddress;
};

/*
UVDBinarySymbolElement
A relocation value based on a binary symbol
This solves a lot of consistency issues
*/
class UVDBinarySymbolElement : public UVDRelocatableElement
{
public:
	UVDBinarySymbolElement();
	UVDBinarySymbolElement(UVDBinarySymbol *binarySymbol);
	~UVDBinarySymbolElement();

	virtual uv_err_t updateDynamicValue();

	virtual uv_err_t getName(std::string &s);
	virtual uv_err_t setName(const std::string &s);
	
public:
	UVDBinarySymbol *m_binarySymbol;
};

#endif
