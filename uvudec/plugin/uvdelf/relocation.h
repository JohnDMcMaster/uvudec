/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_ELF_RELOCATION_H
#define UVD_ELF_RELOCATION_H

#include "uvd/util/error.h"
#include "uvd/relocation/relocation.h"
#include "uvdelf/symbol.h"
#include <string>

/*
A specialized fixup version for ELF files
Ignores a lot of functionality of UVDRelocationFixup, so beware
*/
class UVDElfRelocation : public UVDRelocationFixup
{
public:
	UVDElfRelocation();
	~UVDElfRelocation();
	
	//Set the value of r_offset
	//These two sets are equivilent, eliminate setSectionOffset later
	//uv_err_t setSectionOffset(uint32_t sectionOffset);
	//uv_err_t getSectionOffset(uint32_t *sectionOffset);	
	//virtual uv_err_t setOffset(uint32_t offset);
	//virtual uv_err_t getOffset(uint32_t *offset);

	//Every relocation has a number of bits it should support it seems
	virtual uv_err_t updateRelocationTypeByBits(uint32_t nBits) = 0;
	//raw index into the symbol table
	//in practice might do this by a UVD relocation into m_relocation
	uv_err_t updateSymbolIndex(uint32_t symbolIndex);

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

	//virtual uv_err_t constructForWrite();
	virtual uv_err_t applyRelocationsForWrite();

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
An absolute relocation that should be satisfied based on a symbol
*/
class UVDElfSymbolRelocation : public UVDElfRelocation
{
public:
	UVDElfSymbolRelocation();
	~UVDElfSymbolRelocation();

	virtual uv_err_t updateRelocationTypeByBits(uint32_t nBits);
	
public:
};


/*
A relocation that should be satisfied on a fixed offset from the PC
Typically used for relocating absolute jump labels
*/
class UVDElfPCRelocation : public UVDElfRelocation
{
public:
	UVDElfPCRelocation();
	~UVDElfPCRelocation();

	virtual uv_err_t updateRelocationTypeByBits(uint32_t nBits);
	
public:
	//Signed offset of where we should jump to
	//Location is relative to the instruction after current? from relocatable data start/end?
	//can't be instruction I would think since linker doesn't know much about instructions
	//need to find out more info on how exactly TIS or GNU defines this
	int32_t m_offset;
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
	//virtual uv_err_t updateDataCore();
	//virtual uv_err_t syncDataAfterUpdate();

	uv_err_t addRelocation(UVDElfRelocation *relocation);

	//virtual uv_err_t updateForWrite();
	virtual uv_err_t constructForWrite();
	virtual uv_err_t applyRelocationsForWrite();

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

#if 0
/*
A relocation value based on finding the index of the element in a table
*/
class UVDStringTableRelocatableElement : public UVDRelocatableElement
{
public:
	UVDStringTableRelocatableElement();
	//Find the index of s in the given string table as the value
	UVDStringTableRelocatableElement(UVDElfStringTableSectionHeaderEntry *stringTable, std::string &out);
	virtual ~UVDStringTableRelocatableElement();
	
	virtual uv_err_t updateDynamicValue();
	
public:
	//The string table we will be searching
	UVDElfStringTableSectionHeaderEntry *m_stringTable;
	//The string we are looking for
	std::string m_sTarget;
};
#endif

#endif
