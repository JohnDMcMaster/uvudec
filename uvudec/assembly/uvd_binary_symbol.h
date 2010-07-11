/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
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
Refers to the actual data, as we can have multile names representing same symbol
	May be represented as seperate symbols if more appropriete, symantecs not yet worked out
*/
class UVDBinarySymbol
{
public:
	UVDBinarySymbol();
	virtual ~UVDBinarySymbol();
	virtual uv_err_t init();
	virtual uv_err_t deinit();

	/*
	UVDRelocatableElement also has a name, this has caused some conflicts
		CHECKME: wasn't this forced to use the symbol val
	Return the primary name associated with the symbol
	At least be consistent if there are multiple
	Typically only one nam
	FLIRT stuff indicated some modules may export the same symbol under multiple names
	Seems reasonable enough
	If there are no symbol names defines, name will be returned empty and getSymbolName() will not return an error
	*/ 
	//This will set the first symbol name to this, or add one if it doesn't exist
	//For compatibility and ease of use of code that expects to work only with a primary symbol name
	void setSymbolName(const std::string &name);
	//If does not exist, add as a new symbol name
	void addSymbolName(const std::string &name);
	uv_err_t getSymbolName(std::string &name);
	uv_err_t getSymbolNames(std::set<std::string> &names);	
	
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

	//Given a filename, create a legal symbol name part out of it
	//Something like /temp/virus.bin -> virus
	static std::string mangleFileToSymbolName(const std::string &sIn);

public:
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

private:
	//The symbol's (function's/variable's) primary name
	//If set, should be contained in the symbolNames set
	std::string m_symbolName;
	std::set<std::string> m_symbolNames;
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
	uv_err_t deinit();

	uv_err_t findSymbolByAddress(uv_addr_t address, UVDBinarySymbol **symbol);

	uv_err_t findSymbol(const std::string &name, UVDBinarySymbol **symbol);
	//If this one is used for analysis, make sure its an analyzed version
	//Should be deprecated.  All analyzed symbols can easily be keyed to an address of some sort
	uv_err_t findAnalyzedSymbol(std::string &name, UVDAnalyzedBinarySymbol **symbol);
	uv_err_t findAnalyzedSymbolByAddress(uv_addr_t address, UVDAnalyzedBinarySymbol **symbol);
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
	uv_err_t addAbsoluteFunctionRelocation(uv_addr_t functionAddress,
			uint32_t relocatableDataOffset, uint32_t relocatableDataSizeBytes);
	uv_err_t addAbsoluteFunctionRelocationByBits(uint32_t functionAddress,
			uint32_t relocatableDataOffset, uint32_t relocatableDataSizeBits);
	//ie from a jump/goto
	uv_err_t addAbsoluteLabelRelocation(uv_addr_t labelAddress,
			uint32_t relocatableDataOffset, uint32_t relocatableDataSizeBytes);
	uv_err_t addAbsoluteLabelRelocationByBits(uv_addr_t labelAddress,
			uint32_t relocatableDataOffset, uint32_t relocatableDataSizeBits);

	//Add the core definition
	//May return error if addFunction() was already called
	//Will not error if it was added previously as a relocation
	uv_err_t addFunction(uv_addr_t functionAddress);
	//Do we need this?  Labels are local and imply a relocation or they are useless
	//Vectors are similar, but global.  Add a diff func for them
	//uv_err_t addLabel(uv_addr_t labelAddress);
	//TODO: implement this when needed
	//Will be when start dealing with ISR or an MCU that doesn't vector to 0 for start
	//uv_err_t addVector(uv_addr_t vectorTo, const std::string &name);

	//Find the function symbol passed in and add all relocations, if any
	uv_err_t collectRelocations(UVDBinaryFunction *function);

	uv_err_t analyzedSymbolName(uv_addr_t functionAddress, int symbolType, std::string &symbolName);
	//This should get moved to util
	uv_err_t analyzedSymbolName(std::string dataSource, uv_addr_t functionAddress, int type, std::string &symbolName);

private:
	uv_err_t doCollectRelocations(UVDBinaryFunction *function, UVDBinarySymbol *analysisSymbol);

public:
	//Symbol names must be unique
	//Needed to convert addresses to symbol names
	//Not owned by this
	UVDAnalyzer *m_analyzer;

private:
	//These represent same object with different mappings, they are owned by this
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
