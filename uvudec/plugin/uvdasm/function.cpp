/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvdasm/function.h"
#include "uvdasm/instruction.h"

UVDDisasmFunctionShared::UVDDisasmFunctionShared()
{
}

UVDDisasmFunctionShared::~UVDDisasmFunctionShared()
{
	deinit();
}

uv_err_t UVDDisasmFunctionShared::deinit()
{
	for( std::vector<UVDDisasmOperandShared *>::iterator iter = m_args.begin(); iter != m_args.end(); ++iter )
	{
		delete *iter;
	}
	m_args.clear();

	return UV_ERR_OK;
}

UVDDisasmFunction::UVDDisasmFunction()
{
}

UVDDisasmFunction::~UVDDisasmFunction()
{
	deinit();
}

uv_err_t UVDDisasmFunction::deinit()
{
	/*
	for( std::vector<UVDOperand *>::iterator iter = m_args.begin(); iter != m_args.end(); ++iter )
	{
		delete *iter;
	}
	*/
	m_args.clear();

	return UV_ERR_OK;
}

