/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVDELF_OBJECT_H
#define UVDELF_OBJECT_H

/*
Original use was to store relocatable object files to aid in static analysis
FIXME: own class to UVDObject, needs cleanup to only conform to UVDObject interfaces
TODO: intead of searching through string tables, use handles
	Didn't want to do this before because thought I'd have to #define values
	Can make these common ones reserved and use a factory function to retrieve new indexes if needed
*/

#include "uvd/object/object.h"
#include "uvd/data/data.h"
#include "uvdelf/header.h"
#include "uvdelf/symbol.h"
#include "uvd/relocation/relocation.h"
#include "uvd/util/types.h"
#include <string>
#include <elf.h>

/*
For representing architectures TIS/GNU does not define
Wrap it in hopes this project becomes popular enough to appear in some versions of <elf.h> ;)
*/
#ifndef EM_UVUDEC
#define EM_UVUDEC					0xBAE2
#endif //EM_UVUDEC

/*
Constant data
Things like strings (const char *), program constants, lookup tables, etc
*/
class UVDElfRomData
{
public:
	//The actual data
	UVDData *m_data;
	//Relocatable element so we can figure out where it was placed
	UVDRelocatableElement *m_relocatable;
};

/*
An ELF (Executable and Linker Format) reader/writer for working with relocatable object files
Focus will be on section header since original idea was only to do object file storing/loading for reloc
*/
class UVDElf : public UVDObject
{
public:
	/*
	Creating and saving ELF objects
	*/
	//Blank ELF file
	UVDElf();
	~UVDElf();
	/*
	Factory function to create a blank conforming item
	Fills in basic header information and other peices
	*/
	//static uv_err_t getUVDElf(UVDElf **out);
	//Setup with default stuffs
	uv_err_t init(UVDData *data);
	uv_err_t initHeader();
	
	static uv_err_t getFromFile(const std::string &file, UVDElf **elf);
	uv_err_t loadFromFile(const std::string &file);
	//Get the raw binary image of this object
	uv_err_t constructBinary(UVDData **data);
	uv_err_t saveToFile(const std::string &file);

	//Deprecated
	//Construct from a UVD relocatable
	//Assumes in the .text section
	static uv_err_t getFromRelocatableData(UVDRelocatableData *relocatableData,
			const std::string &rawDataSymbolName, UVDElf **out);
	//Doest not make section assumptions
	static uv_err_t getFromRelocatableDataCore(UVDRelocatableData *relocatableData,
			const std::string &rawDataSymbolName,
			const std::string &sDataSection, const std::string &sRelocationSection,
			UVDElf **out);
	

	virtual uv_err_t addRelocation(UVDRelocationFixup *analysisRelocation);
	virtual uv_err_t addFunction(UVDBinaryFunction *function);
		
	//Add another symbol along with supporting data
	//uv_err_t addRelocatableData(UVDRelocatableData *relocatableData,
	//		const std::string &rawDataSymbolName);
	uv_err_t addRelocatableDataCore(UVDRelocatableData *relocatableData,
			const std::string &rawDataSymbolName,
			const std::string &sDataSection, const std::string &sRelocationSection);
	uv_err_t addUVDRelocationFixup(UVDRelocationFixup *fixup);





	//Set the architecture
	//Should throw error on unknown architecture?
	uv_err_t getArchitecture(int *archOut);
	void setArchitecture(int archIn);

	int getProgramHeaderTableSize();
	int getNumberProgramHeaderTableEntries();
	int getSectionHeaderTableSize();
	int getNumberSectionHeaderTableEntries();

	uv_err_t addProgramHeaderSection(UVDElfProgramHeaderEntry *section);
	uv_err_t addSectionHeaderSection(UVDElfSectionHeaderEntry *section);
	uv_err_t getSectionHeaderIndex(const UVDElfSectionHeaderEntry *section, uint32_t *index);
	uv_err_t getSectionHeaderIndexByName(const std::string &sName, uint32_t *index);
	
	//WARNING: this is currently find, not get (ie won't create on not found)
	uv_err_t getSectionHeaderByName(const std::string &sName, UVDElfSectionHeaderEntry **section);
	
	
	/*
	Generic string table functions
	Will hopefully return an error if you feed it a table that isn't a string table
	*/
	//Gaurantee string is in the table
	//If index is specified, it will be returned
	uv_err_t addStringCore(const std::string &sSection, const std::string &s);
	//This should only be called from writer operations
	uv_err_t getStringIndexCore(const std::string &sSection, const std::string &s, uint32_t *index);
	//For a given string, get an object to find its final address
	//uv_err_t getStringRelocatableElementCore(const std::string &sSection, const std::string &s, UVDRelocatableElement **relocatable, UVDRelocationManager *relocationManager);
	

	/*
	.shstrtab utilities
	Should be used for section header names
	*/
	uv_err_t addSectionHeaderString(const std::string &s);
	uv_err_t getSectionHeaderStringIndex(const std::string &s, uint32_t *index);
	//uv_err_t getSectionHeaderStringRelocatableElement(const std::string &s, UVDRelocatableElement **relocatable, UVDRelocationManager *relocationManager);
	//Get the section header for .shstrtab
	uv_err_t getSectionHeaderStringTableEntry(UVDElfStringTableSectionHeaderEntry **sectionOut);

	/*
	.strtab
	Used for symbol names
	*/
	uv_err_t addSymbolString(const std::string &s);
	uv_err_t getSymbolStringIndex(const std::string &s, uint32_t *index);
	//uv_err_t getSymbolStringRelocatableElement(const std::string &s, UVDRelocatableElement **relocatable);
	//Get the section header for .strtab
	uv_err_t getSymbolStringTableSectionHeaderEntry(UVDElfStringTableSectionHeaderEntry **sectionOut);

	/*
	.symtab
	Used for symbols
	*/
	//Get the default table holding the actual symbols
	uv_err_t getSymbolTableSectionHeaderEntry(UVDElfSymbolSectionHeaderEntry **sectionOut);
	uv_err_t setSymbolTableSectionHeaderEntry(UVDElfSymbolSectionHeaderEntry *section);
	//There are multiple of these
	//uv_err_t getRelocationSectionHeaderEntry(UVDElfRelocationSectionHeaderEntry **sectionOut);
	//Higher level functions
	//Some arbitrary symbol, it is not know what it represents
	uv_err_t addSymbol(UVDElfSymbol *symbol);
	uv_err_t findSymbol(const std::string &sName, UVDElfSymbol **symbol);
	uv_err_t getVariableSymbol(const std::string &sName, UVDElfSymbol **symbol);
	//Get a new function symbol including adding it to our object
	uv_err_t getFunctionSymbol(const std::string &sName, UVDElfSymbol **symbol);
	uv_err_t setSourceFilename(const std::string &s);
	
	/*
	.text
	Holds executable information
	Stores the analyzed functions
	*/
	uv_err_t getTextSectionHeaderEntry(UVDElfTextSectionHeaderEntry **sectionHeaderOut);
	
	
	/*
	.rodata
	Program constant data
	The string table you might uusally think of
	Not required for now since I'm only dealing with functions and not supporting data...yet
	*/
	
	//Raw data adds
	//uv_err_t addSectionHeaderData(const std::string &sSection, UVDRelocatableData *relocatableData);
	//uv_err_t addSectionHeaderData(int sectionIndex, UVDRelocatableData *relocatableData);
	
		
	//Adds constant data to .rodata
	//Strings and what not
	//Considering adding to .text instead to make processing easier...but this is more proper
	//uv_err_t addROMData(UVDRomData *romData)

	//Program data
	//Add arbitrary data to the text section
	//uv_err_t addTextData(UVDElfVariable *variable);
	//uv_err_t addFunction(UVDElfFunctionSymbol *function);
	//Register a variable to current ELF file
	//uv_err_t addVariable(UVDElfVariableSymbol *variable);

	void printDebug();

	//If the section does not exist, it will be created
	//It will not be added to the ELF object
	uv_err_t getUVDElfSectionHeaderEntry(const std::string &sSection, UVDElfSectionHeaderEntry **sectionHeaderOut);
	//Like above, but will be added to the section header table
	uv_err_t addUVDElfSectionHeaderEntry(const std::string &sSection, UVDElfSectionHeaderEntry **sectionHeaderOut);

	static uv_err_t canLoad(const UVDData *data, const UVDRuntimeHints &hints, uvd_priority_t *confidence);
	static uv_err_t tryLoad(UVDData *data, const UVDRuntimeHints &hints, UVDObject **out);

public:
	//If we loaded this, pointer to the raw data source
	//UVDData *m_data;
	//Program header sections
	std::vector<UVDElfProgramHeaderEntry *> m_programHeaderEntries;
	//Section headers
	std::vector<UVDElfSectionHeaderEntry *> m_sectionHeaderEntries;
	//If m_e_ehsize is larger than sizeof(Elf32_Ehdr), we have extra reserved junk
	UVDData *m_elfHeaderExtra;
	//Raw elf header structure
	Elf32_Ehdr m_elfHeader;
};

#endif
