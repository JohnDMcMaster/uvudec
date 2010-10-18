/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/object/section.h"
#include "uvd/assembly/address.h"

UVDSection::UVDSection()
{
	m_data = NULL;
	m_alignment = 0;
	m_flags = 0;
	m_VMA = 0;
	m_VMASize = 0;
	
	m_R = UVD_TRI_UNKNOWN;
	m_W = UVD_TRI_UNKNOWN;
	m_X = UVD_TRI_UNKNOWN;
}

UVDSection::~UVDSection()
{
	delete m_data;
}

/*
uv_err_t UVDSection::getName(std::string &out)
{
}

uv_err_t UVDSection::getData(UVDData **out)
{
}
*/

uv_err_t UVDSection::getAllRelocations(std::vector<UVDRelocationFixup> &out)
{
	//No arch specific processing here
	out = m_relocations;
	return UV_ERR_OK;
}

uv_err_t UVDSection::toAddressSpace(UVDAddressSpace **out)
{
	UVDAddressSpace *space = NULL;
	UVDDataChunk *data = NULL;
	
	space = new UVDAddressSpace();
	uv_assert_ret(space);

	space->m_name = m_name;
	space->m_desc = "generated from object file section";
	if( m_data )
	{
		//Do a direct remapping
		data = new UVDDataChunk();
		data->init(m_data);
		space->m_data = data;
	}
	else
	{
		space->m_data = NULL;
	}
	//If specified, map it
	//Otherwise, do a simple mapping
	if( m_VMASize )
	{
		space->m_min_addr = m_VMA;
		space->m_max_addr = m_VMA + m_VMASize - 1;
	}
	else
	{
		space->m_min_addr = 0;
		if( m_data )
		{
			space->m_max_addr = m_data->size() - 1;
		}
		else
		{
			space->m_max_addr = 0;
		}
	}
	space->m_R = m_R;
	space->m_W = m_W;
	space->m_X = m_X;
	space->m_NV = UVD_TRI_UNKNOWN;
	space->m_word_size = 8;
	space->m_word_alignment = m_alignment;
		
	uv_assert_ret(out);
	*out = space;
	
	return UV_ERR_OK;
}

