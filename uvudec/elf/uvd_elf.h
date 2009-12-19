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
#include "uvd_elf_symbol.h"
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
	uv_err_t getSectionHeaderIndex(const UVDElfSectionHeaderEntry *section, uint32_t *index);
	
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
	uv_err_t getSymbolStringRelocatableElement(const std::string &s, UVDRelocatableElement **relocatable);
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
	uv_err_t getFunctionSymbol(const std::string &sName, UVDElfSymbol **symbol);
	
	/*
	.text
	Holds executable information
	Stores the analyzed functions
	*/
	uv_err_t getTextSectionHeaderEntry(UVDElfTextSectionHeaderEntry **sectionHeaderOut);
	
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
	uv_err_t addFunction(UVDElfFunctionSymbol *function);
	//Register a variable to current ELF file
	uv_err_t addVariable(UVDElfVariableSymbol *variable);

	void printDebug();

	//If the section does not exist, it will be created
	uv_err_t getUVDElfSectionHeaderEntry(const std::string &sSection, UVDElfSectionHeaderEntry **sectionHeaderOut);

protected:
	//Update all of the members so that it can be written to disk
	uv_err_t updateForWrite();
	uv_err_t updateHeaderForWrite();
	uv_err_t updateProgramHeadersForWrite();
	uv_err_t updateSectionHeadersForWrite();

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
