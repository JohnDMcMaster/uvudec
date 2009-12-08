/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#ifndef UVD_ELF_H
#define UVD_ELF_H

/*
Original use was to store relocatable object files to aid in static analysis
FIXME: class is a mess, needs cleanup
*/

#include "uvd_data.h"
#include "uvd_elf_header.h"
#include "uvd_relocation.h"
#include "uvd_types.h"
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

class UVDElfSymbol;
class UVDElfRelocation
{
public:
	UVDElfRelocation();
	~UVDElfRelocation();
	
	/*
	Reloctions are done on an absolute basis of the data in the file
	If there was another chunk of data before this one, it must be accounted for
	Essentially this makes a relocation on a relocation
	For now, only a single symbol is stored in object files, making this factor 0 for now
	*/
	uv_err_t getFileOffset(uint32_t *elfSymbolFileOffset);
	uv_err_t updateRelocationTypeByBits(unsigned int nBits);
	//raw index into the symbol table
	//in practice might do this by a UVD relocation into m_relocation
	uv_err_t updateSymbolIndex(unsigned int symbolIndex);

	//Given a relocatable element, setup relocations on this object
	uv_err_t setupRelocations(UVDRelocationFixup *originalFixup);

	//What we preform the relocation on
	void setSymbol(UVDElfSymbol *symbol);
	
	//Raw access, r_info is non-trivial to access
	int getSymbolTableIndex();
	void setSymbolTableIndex(int index);
	int getRelocationType();
	void setRelocationType(int type);

public:
	//The symbol we will be relocating against
	UVDElfSymbol *m_symbol;
	//Switch to Elf32_Rela if needed
	Elf32_Rel m_relocation;
	//The symbol index for the relocation is dynamic and can only be figured out after symbols are placed
	UVDRelocationFixup m_symbolIndexRelocation;
};

/*
Some sort of globally visible symbol
All symbols occur in a single table and each symbol has a reference to the relavent section
There will be an assumption for now that since all relocations I want are in functions,
all relocations will be done in symbols' data
*/
class UVDElfSymbol
{
public:
	UVDElfSymbol();
	~UVDElfSymbol();

	//Set symbol name
	void setSymbolName(const std::string &sName);
	//Get symbol name
	std::string getSymbolName();

	uv_err_t getData(UVDData **data);
	void setData(UVDData *data);
	
	/*
	From TIS ELF specification

	STB_LOCAL Local symbols are not visible outside the object file containing their
	definition. Local symbols of the same name may exist in multiple files
	without interfering with each other.
	STB_GLOBAL Global symbols are visible to all object files being combined. One file's
	definition of a global symbol will satisfy another file's undefined reference
	to the same global symbol.
	STB_WEAK Weak symbols resemble global symbols, but their definitions have lower
	precedence.
	STB_LOPROC through Values in this inclusive range are reserved for processor-specific semantics.
	STB_HIPROC
	In each symbol table,
	*/
	//void setVisibility(int visibility);
	//uv_err_t getVisibility(int *visibility);
 
 public:
 	//The symbol's (function's/variable's) name


public:
	//The symbol's (function's/variable's) name
	std::string m_sName;
	//The actual data this symbol represents, if its resolved
	//If the symbol is not resolved, data will be NULL
	UVDData *m_data;
	//The ELF symbol structure
	Elf32_Sym m_symbol;
	//Offsets are taken from here and stored in file
	//UVDRelocatableElement *m_relocatable;
};

/*
Section that holds actual symbol data
*/
class UVDElfSymbolSectionHeaderEntry : public UVDElfSectionHeaderEntry
{
public:
	UVDElfSymbolSectionHeaderEntry();
	~UVDElfSymbolSectionHeaderEntry();

	uv_err_t addSymbol(UVDElfSymbol *symbol);
	uv_err_t findSymbol(const std::string &sName, UVDElfSymbol **symbol);
	uv_err_t getSymbol(const std::string &sName, UVDElfSymbol **symbol);
	
public:
	std::vector<UVDElfSymbol *> m_symbols;
};

/*
A global variable
*/
class UVDElfVariable : public UVDElfSymbol
{
public:
	UVDElfVariable();
	~UVDElfVariable();

public:
	//Size in bits
	int m_size;
};

/*
A peice of binary data identified as a function
For convienence only 
*/
class UVDElfFunction : public UVDElfSymbol
{
public:
	UVDElfFunction();
	~UVDElfFunction();

public:
	//Binary function data
	UVDData *m_data;
	//Position in the file written out, not in memory
	//Note this is not used to relocate data during ELF file writting, but rather to generate relocation table
	UVDRelocatableData *m_fileRelocatableData;
};

/*
An ELF (Executable and Linker Format) reader/writter for working with relocatable object files
Focus will be on section header since original idea was only to do object file storing/loading for reloc
*/
class UVDElf
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
	static uv_err_t getUVDElf(UVDElf **out);
	//Setup with default stuffs
	uv_err_t init();
	uv_err_t initHeader();
	
	static uv_err_t getFromFile(const std::string &file, UVDElf **elf);
	uv_err_t loadFromFile(const std::string &file);
	//Get the raw binary image of this object
	uv_err_t constructBinary(UVDData **data);
	uv_err_t saveToFile(const std::string &file);
	//Construct from a UVD relocatable
	//Assumes in the .text section
	static uv_err_t getFromRelocatableData(UVDRelocatableData *relocatableData,
			const std::string &rawDataSymbolName, UVDElf **out);
	//Doest not make section assumptions
	static uv_err_t getFromRelocatableDataCore(UVDRelocatableData *relocatableData,
			const std::string &rawDataSymbolName,
			const std::string &sDataSection, const std::string &sRelocationSection,
			UVDElf **out);
	
	//Add another symbol along with supporting data
	uv_err_t addRelocatableData(UVDRelocatableData *relocatableData,
			const std::string &rawDataSymbolName);
	uv_err_t addRelocatableDataCore(UVDRelocatableData *relocatableData,
			const std::string &rawDataSymbolName,
			const std::string &sDataSection, const std::string &sRelocationSection);

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
	
	//WARNING: this is currently find, not get (ie won't create on not found)
	uv_err_t getSectionHeaderByName(const std::string &sName, UVDElfSectionHeaderEntry **section);
	
	
	/*
	Generic string table functions
	Will hopefully return an error if you feed it a table that isn't a string table
	*/
	//Gaurantee string is in the table
	//If index is specified, it will be returned
	uv_err_t addStringCore(const std::string &sSection, const std::string &s, unsigned int *index = NULL);
	uv_err_t getStringIndexCore(const std::string &sSection, const std::string &s, unsigned int *index);
	//For a given string, get an object to find its final address
	uv_err_t getStringRelocatableElementCore(const std::string &sSection, const std::string &s, UVDRelocatableElement **relocatable, UVDRelocationManager *relocationManager);
	

	/*
	.shstrtab utilities
	Should be used for section header names
	*/
	uv_err_t addSectionHeaderString(const std::string &s, unsigned int *index = NULL);
	uv_err_t getSectionHeaderStringIndex(const std::string &s, unsigned int *index);
	uv_err_t getSectionHeaderStringRelocatableElement(const std::string &s, UVDRelocatableElement **relocatable, UVDRelocationManager *relocationManager);
	//Get the section header for .shstrtab
	uv_err_t getSectionHeaderStringTableEntry(UVDElfStringTableSectionHeaderEntry **sectionOut);


	/*
	.strtab
	Used for symbol names
	*/
	uv_err_t addSymbolString(const std::string &s, unsigned int *index = NULL);
	uv_err_t getSymbolStringIndex(const std::string &s, unsigned int *index);
	uv_err_t getSymbolStringRelocatableElement(const std::string &s, UVDRelocatableElement **relocatable, UVDRelocationManager *relocationManager);
	//Get the section header for .strtab
	uv_err_t getSymbolStringTableSectionHeaderEntry(UVDElfStringTableSectionHeaderEntry **sectionOut);

	/*
	.symtab
	Used for symbols
	*/
	//Get the table holding the actual symbols
	uv_err_t getSymbolTableSectionHeaderEntry(UVDElfSymbolSectionHeaderEntry **sectionOut);
	//Higher level functions
	//Some arbitrary symbol, it is not know what it represents
	uv_err_t addSymbol(UVDElfSymbol *symbol);
	uv_err_t findSymbol(const std::string &sName, UVDElfSymbol **symbol);
	uv_err_t getSymbol(const std::string &sName, UVDElfSymbol **symbol);
	
	/*
	.text
	Holds executable information
	Stores the analyzed functions
	*/
	
	//Raw data adds
	uv_err_t addSectionHeaderData(const std::string &sSection, UVDRelocatableData *relocatableData);
	uv_err_t addSectionHeaderData(int sectionIndex, UVDRelocatableData *relocatableData);
	
		
	//Adds constant data to .rodata
	//Strings and what not
	//Considering adding to .text instead to make processing easier...but this is more proper
	//uv_err_t addROMData(UVDRomData *romData)

	//Program data
	//Add arbitrary data to the text section
	//uv_err_t addTextData(UVDElfVariable *variable);
	uv_err_t addFunction(UVDElfFunction *function);
	//Register a variable to current ELF file
	uv_err_t addVariable(UVDElfVariable *variable);

	void printDebug();

private:
	//Elf header
	uv_err_t constructElfHeaderBinary(UVDRelocationManager *elfRelocationManager);
	uv_err_t constructProgramHeaderBinaryPhoff(UVDRelocationManager *elfRelocationManager,
			UVDRelocatableData *elfHeaderRelocatable);
	uv_err_t constructSectionHeaderBinaryShoff(UVDRelocationManager *elfRelocationManager,
			UVDRelocatableData *elfHeaderRelocatable);
	//Section header
	uv_err_t constructProgramHeaderBinary(UVDRelocationManager &elfRelocationManager,
			std::vector<UVDRelocatableData *> &headerSupportingData);
	uv_err_t constructProgramHeaderSectionBinary(UVDRelocationManager &elfRelocationManager,
			std::vector<UVDRelocatableData *> &headerSupportingData,
			UVDElfProgramHeaderEntry *entry);
	//Program header
	uv_err_t constructSectionHeaderBinary(UVDRelocationManager &elfRelocationManager,
			std::vector<UVDRelocatableData *> &headerSupportingData);
	uv_err_t constructSectionHeaderSectionBinary(UVDRelocationManager &elfRelocationManager,
			std::vector<UVDRelocatableData *> &headerSupportingData,
			UVDElfSectionHeaderEntry *entry);

private:
	//If we loaded this, pointer to the raw data source
	UVDData *m_data;
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
