/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/assembly/instruction.h"

/*
UVDOperandShared
*/

UVDOperandShared::UVDOperandShared()
{
}

UVDOperandShared::~UVDOperandShared()
{
}

uv_err_t UVDOperandShared::init()
{
	return UV_ERR_OK;
}

uv_err_t UVDOperandShared::deinit()
{
	return UV_ERR_OK;
}

/*
UVDOperand
*/

UVDOperand::UVDOperand()
{
	m_shared = NULL;
	m_instruction = NULL;
}

UVDOperand::~UVDOperand()
{
}

uv_err_t UVDOperand::init()
{
	return UV_ERR_OK;
}

uv_err_t UVDOperand::deinit()
{
	return UV_ERR_OK;
}


