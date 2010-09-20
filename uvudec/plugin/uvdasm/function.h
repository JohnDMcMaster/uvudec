/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVDASM_FUNCTION_H
#define UVDASM_FUNCTION_H

#include "uvd_types.h"

//These are part of operands, which already have a name
//Thus certain object such as that are uncessary
class UVDDisasmOperandShared;
class UVDDisasmFunctionShared
//struct uv_disasm_func_shared_t
{
public:
	UVDDisasmFunctionShared();
	~UVDDisasmFunctionShared();
	uv_err_t deinit();

public:
	std::vector<UVDDisasmOperandShared *> m_args;
};

class UVDDisasmOperand;
class UVDOperand;
class UVDDisasmFunction
//struct uv_disasm_func_t
{
public:
	UVDDisasmFunction();
	~UVDDisasmFunction();
	uv_err_t deinit();

public:
	//This needs to match type of UVDInstruction for recursive funcs to work correctly
	std::vector<UVDOperand *> m_args;
};

#endif

