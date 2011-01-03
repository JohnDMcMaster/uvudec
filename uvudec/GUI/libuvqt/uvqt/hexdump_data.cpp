/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvqt/hexdump_data.h"
#include "uvd/util/debug.h"
#include <math.h>

/*
UVQtHexdumpData
*/

UVQtHexdumpData::UVQtHexdumpData()
{
	printf("UVQtHexdumpData::UVQtHexdumpData()\n");

	m_bytesPerRow = 16;
	m_bytesPerSubRow = 8;
	m_numberRows = 5;
	m_data = NULL;
}

uv_err_t UVQtHexdumpData::begin(unsigned int offset, unsigned int index, UVQtDynamicTextData::iterator *out)
{
	/*
	if( this == NULL )
	{
	printf("bad this\n");
	}
	printf("this: 0x%08X\n", (int)this);
	*/
	//printf("begin stuff\n");
	//fflush(stdout);
	UVQtHexdumpData::iterator_impl *iter_impl = new UVQtHexdumpData::iterator_impl(this, offset);
	//printf("data m_dataImpl val 0x%08X\n", (int)iter_impl->m_dataImpl);
	UVQtDynamicTextData::iterator iter = UVQtDynamicTextData::iterator(iter_impl);
	//printf("copy operation\n");
	//printf("iter m_dataImpl: 0x%08X\n", (int)iter.m_dataImpl);
	*out = iter;
	return UV_ERR_OK;
}

uv_err_t UVQtHexdumpData::end(iterator *out)
{
	UVQtHexdumpData::iterator_impl *iter_impl = new UVQtHexdumpData::iterator_impl(this, 0);
	//printf("data size: %d\n", m_data->size());
	iter_impl->m_offset = m_data->size();
	UVQtDynamicTextData::iterator iter = UVQtDynamicTextData::iterator(iter_impl);
	*out = iter;
	return UV_ERR_OK;
}
	
uv_err_t UVQtHexdumpData::getMinOffset(unsigned int *out)
{
	*out = 0;
	return UV_ERR_OK;
}

uv_err_t UVQtHexdumpData::getMaxOffset(unsigned int *out)
{
	//Number of lines
	if( m_data->size() == 0 )
	{
		*out = 0;
	}
	else
	{
		*out = ceil(1.0 * m_data->size() / m_bytesPerRow) - 1;
	}
	return UV_ERR_OK;
}

