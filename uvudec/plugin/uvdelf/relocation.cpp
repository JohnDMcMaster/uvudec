/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvdelf/object.h"
#include "uvdelf/relocation.h"
#include "uvd/util/util.h"
#include <stdio.h>
#include <string.h>

#if 1
#define printf_elf_relocation_debug(...)
#define ELF_RELOCATION_DEBUG(x)
#else
#define printf_elf_relocation_debug(format, ...)		do{ printf("ELF relocation: " format, ## __VA_ARGS__); fflush(stdout); } while(0)
#define ELF_RELOCATION_DEBUG(x)		x
#endif


/*
UVDElfRelocation
*/

UVDElfRelocation::UVDElfRelocation()
{
	memset(&m_relocation, 0, sizeof(m_relocation));
	m_relocationHeader = NULL;
}

UVDElfRelocation::~UVDElfRelocation()
{
}

uv_err_t UVDElfRelocation::getElfSymbol(UVDElfSymbol **symbolOut)
{
	UVDElfSymbol *symbol = NULL;
	
	uv_assert_ret(m_symbol);
	symbol = dynamic_cast<UVDElfSymbol *>(m_symbol);
	uv_assert_ret(symbol);
	
	uv_assert_ret(symbolOut);
	*symbolOut = symbol;
	
	return UV_ERR_OK;
}

/*
uv_err_t UVDElfRelocation::setSectionOffset(uint32_t sectionOffset)
{
	m_relocation.r_offset = sectionOffset;
	return UV_ERR_OK;
}

uv_err_t UVDElfRelocation::getSectionOffset(uint32_t *sectionOffset)
{
	uv_assert_ret(sectionOffset);
	*sectionOffset = m_relocation.r_offset;
	return UV_ERR_OK;
}

uv_err_t UVDElfRelocation::setOffset(uint32_t offset)
{
	return UV_DEBUG(setSectionOffset(offset));
}

uv_err_t UVDElfRelocation::getOffset(uint32_t *offset)
{
	return UV_DEBUG(getSectionOffset(offset));
}
*/

uv_err_t UVDElfRelocation::updateSymbolIndex(uint32_t symbolIndex)
{
	/*
	if( symbolIndex >= 0x1000000 )
	{
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	*/
	m_relocation.r_info = ELF32_R_INFO(symbolIndex, ELF32_R_TYPE(m_relocation.r_info));	
	return UV_ERR_OK;
}

void UVDElfRelocation::setSymbol(UVDElfSymbol *symbol)
{
	m_symbol = symbol;
}

uv_err_t UVDElfRelocation::getHeaderEntryRelocatable(UVDRelocatableData **symbolEntryRelocatableOut)
{
	//FIXME: make sure we only do this once
	/*
	typedef struct
	{
	  Elf32_Addr	r_offset;		// Address 
	  Elf32_Word	r_info;			// Relocation type and symbol index
	} Elf32_Rel;

	Assume symbols are not defined
	Thus, we don't know the value of:
		st_value (applied during final linking)
	*/
	UVDDataMemory *headerDataMemory = NULL;
	
	uv_assert_err_ret(UVDDataMemory::getUVDDataMemoryByTransfer(&headerDataMemory,
			(char *)&m_relocation, sizeof(Elf32_Rel), false));
	uv_assert_ret(headerDataMemory);
	//uv_assert_err_ret(m_headerEntryRelocatableData.setData(headerDataMemory));
	uv_assert_err_ret(m_headerEntryRelocatableData.transferData(headerDataMemory, true));
	
	uv_assert_ret(symbolEntryRelocatableOut);
	*symbolEntryRelocatableOut = &m_headerEntryRelocatableData;
	
	return UV_ERR_OK;
}

uv_err_t UVDElfRelocation::applyRelocationsForWrite()
{
	printf_elf_relocation_debug("relocation: applying relocation for write\n");
	//The two r_info parts
	{
		UVDElfSymbol *symbol = NULL;
		UVDElfSymbolSectionHeaderEntry *sectionHeader = NULL;
		uint32_t symbolIndex = 0;

		uv_assert_err_ret(getElfSymbol(&symbol));
	
		sectionHeader = symbol->m_symbolSectionHeader;
		uv_assert_ret(sectionHeader);
		uv_assert_err_ret(sectionHeader->getSymbolIndex(symbol, &symbolIndex));
		printf_elf_relocation_debug("symbol: %s, index: %d\n", symbol->m_sName.c_str(), symbolIndex);
	
		//What does this comment mean?  What is the real object?
		//Might help later to update the real object
		m_relocation.r_info = ELF32_R_INFO(symbolIndex, ELF32_R_TYPE(m_relocation.r_info));
	}
	uv_assert_err_ret(updateRelocationTypeByBits(getSizeBits()));

	//r_offset
	{
		uint32_t elfSymbolFileOffset = 0;
	
		//uv_assert_err_ret(getFileOffset(&elfSymbolFileOffset));
	
		//Absolute relocation is applied at the offset + local offset to make a full offset
		//FIXME: ...or not since it seems this is being set to 0
		m_relocation.r_offset = elfSymbolFileOffset + m_offset;
	}

	ELF_RELOCATION_DEBUG(m_headerEntryRelocatableData.hexdump());
	return UV_ERR_OK;
}

/*
UVDElfSymbolRelocation
*/

UVDElfSymbolRelocation::UVDElfSymbolRelocation()
{
	m_offset = 0;
}

UVDElfSymbolRelocation::~UVDElfSymbolRelocation()
{
}

uv_err_t UVDElfSymbolRelocation::updateRelocationTypeByBits(uint32_t nBits)
{
	uint32_t relocationType = 0;
	switch( nBits )
	{
	case 8:
		relocationType = R_386_8;
		break;
	case 16:
		relocationType = R_386_16;
		break;
	case 32:
		relocationType = R_386_32;
		break;
	default:
		printf_error("Invalid number of bits: %d\n", nBits);
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	m_relocation.r_info = ELF32_R_INFO(ELF32_R_SYM(m_relocation.r_info), relocationType);
	return UV_ERR_OK;
}

/*
UVDElfPCRelocation
*/

UVDElfPCRelocation::UVDElfPCRelocation()
{
}

UVDElfPCRelocation::~UVDElfPCRelocation()
{
}

uv_err_t UVDElfPCRelocation::updateRelocationTypeByBits(uint32_t nBits)
{
	uint32_t relocationType = 0;
	switch( nBits )
	{
	case 8:
		relocationType = R_386_PC8;
		break;
	case 16:
		relocationType = R_386_PC16;
		break;
	case 32:
		relocationType = R_386_PC32;
		break;
	default:
		printf_error("Invalid number of bits: %d\n", nBits);
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	m_relocation.r_info = ELF32_R_INFO(ELF32_R_SYM(m_relocation.r_info), relocationType);
	return UV_ERR_OK;
}

/*
UVDElfRelocationSectionHeaderEntry
*/

UVDElfRelocationSectionHeaderEntry::UVDElfRelocationSectionHeaderEntry()
{
	m_targetSectionHeader = NULL;
}

UVDElfRelocationSectionHeaderEntry::~UVDElfRelocationSectionHeaderEntry()
{
}

uv_err_t UVDElfRelocationSectionHeaderEntry::init()
{
	uv_assert_err_ret(UVDElfSectionHeaderEntry::init());
	
	m_sectionHeader.sh_entsize = sizeof(Elf32_Rel);
	
	return UV_ERR_OK;
}

uv_err_t UVDElfRelocationSectionHeaderEntry::initRelocatableData()
{
	//So that we can construct the relocatable table in relocatable units
	m_fileRelocatableData = new UVDMultiRelocatableData();
	uv_assert_ret(m_fileRelocatableData);

	printf_debug("relocatable section file relocatable: 0x%.8X)\n", (unsigned int)m_fileRelocatableData);
	return UV_ERR_OK;
}

uv_err_t UVDElfRelocationSectionHeaderEntry::setSymbolSection(UVDElfSymbolSectionHeaderEntry *section)
{
	uv_assert_ret(section);
	//This is described by sh_link
	m_relevantSectionHeader = section;
	return UV_ERR_OK;
}

uv_err_t UVDElfRelocationSectionHeaderEntry::getSymbolSection(UVDElfSymbolSectionHeaderEntry **section)
{
	uv_assert_ret(section);
	//This is described by sh_link
	*section = dynamic_cast<UVDElfSymbolSectionHeaderEntry *>(m_relevantSectionHeader);
	uv_assert_ret(*section);
	return UV_ERR_OK;
}

uv_err_t UVDElfRelocationSectionHeaderEntry::setRelocationSection(UVDElfSectionHeaderEntry *section)
{
	uv_assert_ret(section);
	//This is described by sh_info
	m_targetSectionHeader = section;
	return UV_ERR_OK;
}

uv_err_t UVDElfRelocationSectionHeaderEntry::getRelocationSection(UVDElfSectionHeaderEntry **section)
{
	uv_assert_ret(section);
	//This is described by sh_info
	*section = m_targetSectionHeader;
	return UV_ERR_OK;
}

/*
uv_err_t UVDElfRelocationSectionHeaderEntry::updateForWrite()
{
	//note that sh_link (the symbol table section) should be upated here
	uv_assert_ret(m_relevantSectionHeader);
	uv_assert_err_ret(UVDElfSectionHeaderEntry::updateForWrite());

	//Update all of the relocations
	for( std::vector<UVDElfRelocation *>::iterator iter = m_relocations.begin();
			iter != m_relocations.end(); ++iter )
	{
		UVDElfRelocation *relocation = *iter;
		
		uv_assert_ret(relocation);
		uv_assert_err_ret(relocation->updateForWrite());
	}
	
	return UV_ERR_OK;
}
*/

uv_err_t UVDElfRelocationSectionHeaderEntry::constructForWrite()
{
	/*
	Form the relocation table
	Relocation order should not matter
	*/

	UVDMultiRelocatableData *relocatableData = NULL;

	uv_assert_err_ret(UVDElfSectionHeaderEntry::constructForWrite());
	
	printf_debug("relocation upate, entries: %d\n", m_relocations.size());

	relocatableData = dynamic_cast<UVDMultiRelocatableData *>(m_fileRelocatableData);
	uv_assert_ret(relocatableData);	
	uv_assert_ret(relocatableData->m_relocatableDatas.empty());

	for( std::vector<UVDElfRelocation *>::iterator iter = m_relocations.begin();
			iter != m_relocations.end(); ++iter )
	{
		//Note they are of type UVDRelocationFixup
		UVDElfRelocation *relocation = *iter;
		UVDRelocatableData *relocatableEntryRelocatable = NULL;
		
		uv_assert_ret(relocation);
 		uv_assert_err_ret(relocation->getHeaderEntryRelocatable(&relocatableEntryRelocatable));
		relocatableData->m_relocatableDatas.push_back(relocatableEntryRelocatable);
	}
	
	uv_assert_err_ret(relocatableData->getRelocatableData(&m_fileData));

	printf_elf_relocation_debug("relocation table after constructForWrite\n");
	ELF_RELOCATION_DEBUG(m_fileRelocatableData->hexdump());

	return UV_ERR_OK;
}

uv_err_t UVDElfRelocationSectionHeaderEntry::applyRelocationsForWrite()
{
	uv_assert_err_ret(UVDElfSectionHeaderEntry::applyRelocationsForWrite());

	for( std::vector<UVDElfRelocation *>::iterator iter = m_relocations.begin();
			iter != m_relocations.end(); ++iter )
	{
		//Note they are of type UVDRelocationFixup
		UVDElfRelocation *relocation = *iter;
		
		uv_assert_ret(relocation);
 		uv_assert_err_ret(relocation->applyRelocationsForWrite());
	}

	{
		//We must set the index of the section we are relocating against (sh_info)
		uv_assert_ret(m_targetSectionHeader);
		uint32_t index = 0;
	
		//We must have the correct link if applicable
		uv_assert_ret(m_elf);
		uv_assert_err_ret(m_elf->getSectionHeaderIndex(m_targetSectionHeader, &index));
		m_sectionHeader.sh_info = index;
	}
	
	printf_elf_relocation_debug("relocation table after relocations\n");
	ELF_RELOCATION_DEBUG(m_fileRelocatableData->hexdump());
	printf_elf_relocation_debug("end table\n");

	return UV_ERR_OK;
}

/*
uv_err_t UVDElfRelocationSectionHeaderEntry::syncDataAfterUpdate()
{
	//Data is auto syncd because of UVDMultiRelocatableData and cannot be set
	return UV_ERR_OK;
}
*/

uv_err_t UVDElfRelocationSectionHeaderEntry::addRelocation(UVDElfRelocation *relocation)
{
	//Make sure its knows this is its home
	uv_assert_ret(relocation);
	relocation->m_relocationHeader = this;

	//And register it for output	
	m_relocations.push_back(relocation);
	return UV_ERR_OK;
}


