/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/architecture/architecture.h"
#include "uvd/assembly/cpu_vector.h"
#include "uvd/core/uvd.h"

UVDArchitecture::UVDArchitecture()
{
	m_uvd = NULL;
}

UVDArchitecture::~UVDArchitecture()
{
	UV_DEBUG(deinit());
}

uv_err_t UVDArchitecture::init()
{
	return UV_ERR_OK;
}

uv_err_t UVDArchitecture::deinit()
{
	for( std::vector<UVDCPUVector *>::iterator iter = m_vectors.begin();
			iter != m_vectors.end(); ++iter )
	{
		delete *iter;
	}

	return UV_ERR_OK;
}

uv_err_t UVDArchitecture::readByte(UVDAddress address, uint8_t *out)
{
	//uv_assert_ret(m_uvd);
	//uv_assert_ret(m_uvd->m_runtime->m_object);
	uv_assert_err_ret(address.m_space->m_data->readData(address.m_addr, out));
	
	return UV_ERR_OK;
}

uv_err_t UVDArchitecture::parseCurrentInstruction(UVDIteratorCommon &iterCommon)
{
	//Reduce errors from stale data
	if( !iterCommon.m_instruction )
	{
		uv_assert_err_ret(getInstruction(&iterCommon.m_instruction));
		uv_assert_ret(iterCommon.m_instruction);
	}
	uv_assert_err_ret(iterCommon.m_instruction->parseCurrentInstruction(iterCommon));

	return UV_ERR_OK;
}

uv_err_t UVDArchitecture::fixupDefaults()
{
	if( m_vectors.empty() )
	{
		UVDCPUVector *vector = NULL;
	
		vector = new UVDCPUVector();
		uv_assert_ret(vector);
		vector->m_name = "start";
		vector->m_description = "auto added";
		vector->m_offset = 0;

		m_vectors.push_back(vector);
	}
	
	return UV_ERR_OK;
}

