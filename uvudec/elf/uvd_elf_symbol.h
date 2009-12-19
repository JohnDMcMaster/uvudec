/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#ifndef UVD_ELF_SYMBOL_H
#define UVD_ELF_SYMBOL_H

#include "uvd_data.h"
#include "uvd_elf_header.h"
#include "uvd_relocation.h"
#include "uvd_types.h"
#include <elf.h>

/*
Some sort of globally visible symbol
All symbols occur in a single table and each symbol has a reference to the relavent section
There will be an assumption for now that since all relocations I want are in functions,
all relocations will be done in symbols' data

This is a hybrid between UVDRelocatableElement and UVDRelocatableData, 
but don't like multiple inheritance
UVDRelocatableData is instead put inside
*/
class UVDElfRelocation;
class UVDElfSymbolSectionHeaderEntry;
class UVDElfSymbol : public UVDRelocatableElement
{
public:
	UVDElfSymbol();
	~UVDElfSymbol();
	
	virtual uv_err_t init();

	//Superseeded by UVDRelocatableElement::getName(), setName()
	//Set symbol name
	//void setSymbolName(const std::string &sName);
	//Get symbol name
	//std::string getSymbolName();

	//Symbol payload
	uv_err_t getData(UVDData **data);
	uv_err_t setData(UVDData *data);
	
	uv_err_t addRelocation(UVDElfRelocation *relocation);
	
	//Get a template relocation value for this symbol
	uv_err_t getRelocation(UVDElfRelocation **relocationOut);
	
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
	//Deprecated for setBinding(), more tech correct in ELF speak
	//void setVisibility(uint32_t visibility);
	//uv_err_t getVisibility(uint32_t *visibility);
 	
 	uv_err_t setBinding(uint32_t binding);
 	uv_err_t getBinding(uint32_t *binding);
 	uv_err_t setType(uint32_t type);
 	uv_err_t getType(uint32_t *type);
 	
 	//Get the header entry with the fixups necessary to push it to the file
 	uv_err_t getHeaderEntryRelocatable(UVDRelocatableData **symbolEntryRelocatable);
	
	virtual uv_err_t updateForWrite();

  	//The symbol's (function's/variable's) name

protected:
	virtual uv_err_t addSymbolNameRelocation();
	//The symbol is about to be saved and all its relocatables should be updated
	virtual uv_err_t updateRelocations();
	//Every symbol type has a specific type that it should have since construction
	virtual uv_err_t updateType() = 0;

public:
	//The symbol's (function's/variable's) name
	//std::string m_sName;
	//The actual data this symbol represents, if its resolved
	//If the symbol is not resolved, data will be NULL
	//Relocatable form so we can do some util stuff like get zerod version
	UVDRelocatableData m_relocatableData;
	//For the header entry
	UVDRelocatableData m_headerEntryRelocatableData;
	//The ELF symbol structure
	Elf32_Sym m_symbol;
	//Relocations
	//std::vector<UVDElfRelocation *> m_relocations;

	//The section header this belongs to
	UVDElfSymbolSectionHeaderEntry *m_symbolSectionHeader;

	//The section header this relocation applies to, if its not special
	//ie if STT_SECTION is used, used to compute st_shndx
	UVDElfSectionHeaderEntry *m_relevantSectionHeader;
};

/*
A global variable
*/
class UVDElfVariableSymbol : public UVDElfSymbol
{
public:
	UVDElfVariableSymbol();
	~UVDElfVariableSymbol();

protected:
	uv_err_t updateType();

public:
};

/*
A peice of binary data identified as a function
For convienence only 
*/
class UVDElfFunctionSymbol : public UVDElfSymbol
{
public:
	UVDElfFunctionSymbol();
	~UVDElfFunctionSymbol();

protected:
	uv_err_t updateType();

public:
	//Binary function data
	//UVDData *m_data;
	//Position in the file written out, not in memory
	//Note this is not used to relocate data during ELF file writting, but rather to generate relocation table
	//UVDRelocatableData *m_fileRelocatableData;
};

/*
Section link symbol
*/
class UVDElfSectionSymbol : public UVDElfSymbol
{
public:
	UVDElfSectionSymbol();
	~UVDElfSectionSymbol();

	virtual uv_err_t updateForWrite();

protected:
	uv_err_t updateType();

public:
};

/*
Null symbol
*/
class UVDElfNullSymbol : public UVDElfSymbol
{
public:
	UVDElfNullSymbol();
	~UVDElfNullSymbol();
	static uv_err_t getUVDElfNullSymbol(UVDElfNullSymbol **symbolOut);

protected:
	uv_err_t updateType();
	virtual uv_err_t addSymbolNameRelocation();

public:
};

/*
Section that holds actual symbol data
*/
class UVDElfSymbolSectionHeaderEntry : public UVDElfSectionHeaderEntry
{
public:
	UVDElfSymbolSectionHeaderEntry();
	~UVDElfSymbolSectionHeaderEntry();

	virtual uv_err_t init();
	virtual uv_err_t initRelocatableData();

	uv_err_t addSymbol(UVDElfSymbol *symbol);
	uv_err_t findSymbol(const std::string &sName, UVDElfSymbol **symbol);
	uv_err_t prepareSymbol(UVDElfSymbol *symbol);
	//A typical symbol having a name and such
	//It will need to be further refined as a function etc
	//Whether or not the symbol is defined will be determined by set data 
	uv_err_t getFunctionSymbol(const std::string &section, UVDElfSymbol **symbol);
	uv_err_t getVariableSymbol(const std::string &section, UVDElfSymbol **symbol);
	//Add a STT_SECTION entry
	uv_err_t addSectionSymbol(const std::string &section);
	
	//Used during writting relocations to ELF file
	uv_err_t getSymbolIndex(const UVDElfSymbol *symbool, uint32_t *index);
	
	//The symbol table has string relocations, non-trivial to return
	virtual uv_err_t updateDataCore();
	virtual uv_err_t syncDataAfterUpdate();

	uv_err_t getSymbolStringRelocatableElement(const std::string &s, UVDRelocatableElement **relocatable);
		
	virtual uv_err_t updateForWrite();

protected:
	uv_err_t getSectionSymbol(const std::string &section, UVDElfSymbol **symbol);
	uv_err_t addNullSymbol();
		
public:
	std::vector<UVDElfSymbol *> m_symbols;
	//Special symbols
	UVDElfSymbol *m_fileNameSymbol;
	//Do we need a symbol specifying the section symbols are in?
};

/*
A specialized fixup version for ELF files
Ignores a lot of functionality of UVDRelocationFixup, so beware
*/
class UVDElfRelocation : public UVDRelocationFixup
{
public:
	UVDElfRelocation();
	~UVDElfRelocation();
	
	/*
	FIXME: below seems wrong.  From TIS ELF specification (1-22):
	r_offset
	This member gives the location at which to apply the relocation action. For
	a relocatable file, the value is the byte offset from the beginning of the
	section to the storage unit affected by the relocation.

	Reloctions are done on an absolute basis of the data in the file
	If there was another chunk of data before this one, it must be accounted for
	Essentially this makes a relocation on a relocation
	For now, only a single symbol is stored in object files, making this factor 0 for now
	*/
	//uv_err_t getFileOffset(uint32_t *elfSymbolFileOffset);

	//Set the value of r_offset
	//These two sets are equivilent, eliminate setSectionOffset later
	uv_err_t setSectionOffset(uint32_t sectionOffset);
	uv_err_t getSectionOffset(uint32_t *sectionOffset);	
	virtual uv_err_t setOffset(uint32_t offset);
	virtual uv_err_t getOffset(uint32_t *offset);

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

 	//Get the header entry with the fixups necessary to push it to the file
 	uv_err_t getHeaderEntryRelocatable(UVDRelocatableData **relocatableEntryRelocatable);

	//replaced by UVDRelocationFixup::m_symbol which must be of type UVDElfSymbol
	uv_err_t getElfSymbol(UVDElfSymbol **symbolOut);

	virtual uv_err_t updateForWrite();

public:
	//replaced by UVDRelocationFixup::m_symbol which must be of type UVDElfSymbol
	//The symbol we will be relocating against
	//UVDElfSymbol *m_symbol;
	
	//Switch to Elf32_Rela if needed
	Elf32_Rel m_relocation;
	//The symbol index for the relocation is dynamic and can only be figured out after symbols are placed
	//UVDRelocationFixup m_symbolIndexRelocation;
	//For the table entry
	UVDRelocatableData m_headerEntryRelocatableData;

	//the relocation table header
	//m_relevantSectionHeader now must be of type UVDElfRelocationSectionHeaderEntry
	UVDElfRelocationSectionHeaderEntry *m_relocationHeader;
};

/*
A generic relocation section, not necessary for .text
*/
class UVDElfRelocationSectionHeaderEntry : public UVDElfSectionHeaderEntry
{
public:
	UVDElfRelocationSectionHeaderEntry();
	~UVDElfRelocationSectionHeaderEntry();
	virtual uv_err_t init();

	virtual uv_err_t initRelocatableData();

	//To compute the necessary Elf32_Rel table
	virtual uv_err_t updateDataCore();
	virtual uv_err_t syncDataAfterUpdate();

	uv_err_t addRelocation(UVDElfRelocation *relocation);

	virtual uv_err_t updateForWrite();

	//stored in sh_link
	uv_err_t setSymbolSection(UVDElfSymbolSectionHeaderEntry *section);
	uv_err_t getSymbolSection(UVDElfSymbolSectionHeaderEntry **section);
	//stored in sh_link
	uv_err_t setRelocationSection(UVDElfSectionHeaderEntry *section);
	uv_err_t getRelocationSection(UVDElfSectionHeaderEntry **section);

public:
	//The section we will apply ELF relocations to
	//We will need to put its index in the link info in the header
	UVDElfSectionHeaderEntry *m_targetSectionHeader;

	//All relocations are defined relative to a symbol table
	//This is what should be in our relative table sh_link entry
	//UVDElfSymbolSectionHeaderEntry *m_symbolSectionHeader;

protected:
	std::vector<UVDElfRelocation *> m_relocations;
};

#endif
