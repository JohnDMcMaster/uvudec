/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#include "uvd_relocation.h"

/*
UVDRelocationFixup
*/

UVDRelocationFixup::UVDRelocationFixup()
{
	m_symbol = NULL;
	m_offset = 0;
	m_size = 1;
}

UVDRelocationFixup::UVDRelocationFixup(UVDRelocatableElement *symbol, unsigned int offset, unsigned int size)
{
	m_symbol = symbol;
	m_offset = offset;
	m_size = size;
}

UVDRelocationFixup::~UVDRelocationFixup()
{
}

uv_err_t UVDRelocationFixup::applyPatch(UVDData *data)
{
	return UV_DEBUG(applyPatchCore(data, false));
}

uv_err_t UVDRelocationFixup::applyPatchCore(UVDData *data, bool useDefaultValue)
{
	const char *dynamicValue = NULL;
	int defaultValue = RELOCATION_DEFAULT_VALUE;

	//XXX: endianness, size issues
	if( useDefaultValue )
	{
		dynamicValue = (const char *)&defaultValue;
	}
	else
	{
		uv_assert_ret(m_symbol);
		uv_assert_err_ret(m_symbol->getDynamicValue(&dynamicValue));
	}
	
	uv_assert_err_ret(data->writeData(m_offset, dynamicValue, m_size));
	
	return UV_ERR_OK;
}

