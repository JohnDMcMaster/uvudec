/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#include "uvd_elf.h"
#include "uvd_elf_relocation.h"
#include "uvd_util.h"

/*
UVDElfRinfoElement
*/

class UVDElfRinfoElement : public UVDRelocatableElement
{
public:
	UVDElfRinfoElement();
	UVDElfRinfoElement(UVDElfRelocation *relocation);
	~UVDElfRinfoElement();

	virtual uv_err_t updateDynamicValue();
	
public:
	UVDElfRelocation *m_relocation;
};

UVDElfRinfoElement::UVDElfRinfoElement()
{
	m_relocation = NULL;
}

UVDElfRinfoElement::UVDElfRinfoElement(UVDElfRelocation *relocation)
{
	m_relocation = relocation;
}

UVDElfRinfoElement::~UVDElfRinfoElement()
{
}

uv_err_t UVDElfRinfoElement::updateDynamicValue()
{
	UVDElfSymbol *symbol = NULL;
	UVDElfSymbolSectionHeaderEntry *sectionHeader = NULL;
	uint32_t symbolIndex = 0;

	uv_assert_ret(m_relocation);

	//symbol = dynamic_cast<UVDElfSymbol *>(m_relocation->m_symbol); 
	//uv_assert_ret(symbol);
	uv_assert_err_ret(m_relocation->getElfSymbol(&symbol));

	sectionHeader = symbol->m_symbolSectionHeader;
	uv_assert_ret(sectionHeader);

	uv_assert_err_ret(sectionHeader->getSymbolIndex(symbol, &symbolIndex));
	
	//Might help later to update the real object
	m_relocation->m_relocation.r_info = ELF32_R_INFO(symbolIndex, ELF32_R_TYPE(m_relocation->m_relocation.r_info));
	setDynamicValue(m_relocation->m_relocation.r_info);

	return UV_ERR_OK;
}

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

uv_err_t UVDElfRelocation::updateRelocationTypeByBits(uint32_t nBits)
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

uv_err_t UVDElfRelocation::setupRelocations(UVDRelocationFixup *originalFixup)
{
	uint32_t elfSymbolFileOffset = 0;
	
	//uv_assert_err_ret(getFileOffset(&elfSymbolFileOffset));
	
	//Absolute relocation is applied at the offset + local offset to make a full offset
	m_relocation.r_offset = elfSymbolFileOffset + originalFixup->m_offset;
	uv_assert_err_ret(updateRelocationTypeByBits(originalFixup->getSizeBits()));
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
	uv_assert_err_ret(m_headerEntryRelocatableData.setData(headerDataMemory));

	
	//Add relocations
	
	//r_offset should already be set or in any case is updated as needed
	
	//Symbol index
	{
		UVDRelocatableElement *relocatable = NULL;
		UVDRelocationFixup *offsetFixup = NULL;

		//The value
		relocatable = new UVDElfRinfoElement(this);
		uv_assert_ret(relocatable);
		//Where
		offsetFixup = new UVDRelocationFixup(relocatable,
				OFFSET_OF(Elf32_Rel, r_info), sizeof(m_relocation.r_info));
		uv_assert_ret(offsetFixup);
		
		m_headerEntryRelocatableData.addFixup(offsetFixup);
	}
	
	uv_assert_ret(symbolEntryRelocatableOut);
	*symbolEntryRelocatableOut = &m_headerEntryRelocatableData;
	
	return UV_ERR_OK;
}

uv_err_t UVDElfRelocation::updateForWrite()
{
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

uv_err_t UVDElfRelocationSectionHeaderEntry::updateDataCore()
{
	/*
	Form the relocation table
	Relocation order should not matter
	*/
	UVDMultiRelocatableData *relocatableData = NULL;
	
	printf_debug("relocation upate, entries: %d\n", m_relocations.size());

	relocatableData = dynamic_cast<UVDMultiRelocatableData *>(m_fileRelocatableData);
	uv_assert_ret(relocatableData);
	
	//We are rebuilding this table
	relocatableData->m_relocatableDatas.clear();

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

	return UV_ERR_OK;
}

uv_err_t UVDElfRelocationSectionHeaderEntry::syncDataAfterUpdate()
{
	//Data is auto syncd because of UVDMultiRelocatableData and cannot be set
	return UV_ERR_OK;
}

uv_err_t UVDElfRelocationSectionHeaderEntry::addRelocation(UVDElfRelocation *relocation)
{
	//Make sure its knows this is its home
	uv_assert_ret(relocation);
	relocation->m_relocationHeader = this;

	//And register it for output	
	m_relocations.push_back(relocation);
	return UV_ERR_OK;
}

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
	
	//We must set the index of the section we are relocating against (sh_info)
	uv_assert_ret(m_targetSectionHeader);
	{
		uint32_t index = 0;
		
		//We must have the correct link if applicable
		uv_assert_ret(m_elf);
		uv_assert_err_ret(m_elf->getSectionHeaderIndex(m_targetSectionHeader, &index));
		m_sectionHeader.sh_info = index;
	}

	
	return UV_ERR_OK;
}
