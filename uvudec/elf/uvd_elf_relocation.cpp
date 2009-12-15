/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#include "uvd_elf.h"
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

	sectionHeader = symbol->m_sectionHeader;
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
uv_err_t UVDElfRelocation::getFileOffset(uint32_t *elfSymbolFileOffset)
{
	uv_assert_ret(elfSymbolFileOffset);
	//See header note
	*elfSymbolFileOffset = 0;
	return UV_ERR_OK;
}
*/

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
	unsigned int relocationType = 0;
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

uv_err_t UVDElfRelocation::updateSymbolIndex(unsigned int symbolIndex)
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
