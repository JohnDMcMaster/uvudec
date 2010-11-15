/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvdgb/architecture.h"
#include "uvdgb/config.h"
#include "uvd/assembly/instruction.h"
#include "uvd/core/iterator.h"

UVDGBArchitecture::UVDGBArchitecture()
{
}

UVDGBArchitecture::~UVDGBArchitecture()
{
	UV_DEBUG(deinit());
}

uv_err_t UVDGBArchitecture::init()
{
	//m_opcodeTable = new UVDDisasmOpcodeLookupTable();
	//uv_assert_ret(m_opcodeTable);
	
	return UV_ERR_OK;
}

uv_err_t UVDGBArchitecture::deinit()
{
	//delete m_opcodeTable;
	//m_opcodeTable = NULL;

	return UV_ERR_OK;
}

uv_err_t UVDGBArchitecture::getInstruction(UVDInstruction **out)
{
	uv_assert_ret(out);
	*out = new UVDGBInstruction();
	uv_assert_ret(*out);
	return UV_ERR_OK;
}

uv_err_t UVDGBArchitecture::parseCurrentInstruction(UVDIteratorCommon &iterCommon)
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

