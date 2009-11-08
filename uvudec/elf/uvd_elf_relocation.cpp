#include "uvd_elf.h"

UVDElfRelocation::UVDElfRelocation()
{
}

uv_err_t UVDElfRelocation::getFileOffset(uint32_t *elfSymbolFileOffset)
{
	uv_assert_ret(elfSymbolFileOffset);
	//See header note
	*elfSymbolFileOffset = 0;
	return UV_ERR_OK;
}

uv_err_t UVDElfRelocation::updateRelocationTypeByBits(unsigned int nBits)
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
	
	uv_assert_err_ret(getFileOffset(&elfSymbolFileOffset));
	
	//Absolute relocation is applied at the offset + local offset to make a full offset
	m_relocation.r_offset = elfSymbolFileOffset + originalFixup->m_offset;
	uv_assert_err_ret(updateRelocationTypeByBits(originalFixup->m_size));
	return UV_ERR_OK;
}

void UVDElfRelocation::setSymbol(UVDElfSymbol *symbol)
{
	m_symbol = symbol;
}
