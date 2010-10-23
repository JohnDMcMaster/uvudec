/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvdbfd/architecture.h"
#include "uvdbfd/instruction.h"
#include "uvd/assembly/instruction.h"
#include "uvd/core/iterator.h"

UVDBFDArchitecture::UVDBFDArchitecture()
{
}

UVDBFDArchitecture::~UVDBFDArchitecture()
{
}

uv_err_t UVDBFDArchitecture::init()
{
	return UV_ERR_OK;
}

uv_err_t UVDBFDArchitecture::deinit()
{
	return UV_ERR_OK;
}

uv_err_t UVDBFDArchitecture::getInstruction(UVDInstruction **out)
{
	uv_assert_ret(out);
	*out = new UVDBFDInstruction();
	uv_assert_ret(*out);
	return UV_ERR_OK;
}

uv_err_t UVDBFDArchitecture::getAddresssSpaceNames(std::vector<std::string> &names)
{
	return UV_ERR_OK;
}

uv_err_t UVDBFDArchitecture::parseCurrentInstruction(UVDIteratorCommon &iterCommon)
{
	//Reduce errors from stale data
	if( !iterCommon.m_instruction )
	{
		//iterCommon.m_instruction = new UVDDisasmInstruction();
		uv_assert_err_ret(getInstruction(&iterCommon.m_instruction));
		uv_assert_ret(iterCommon.m_instruction);
	}
	uv_assert_err_ret(iterCommon.m_instruction->parseCurrentInstruction(iterCommon));

	return UV_ERR_OK;
}

