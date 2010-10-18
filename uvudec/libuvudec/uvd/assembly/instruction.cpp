/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/assembly/instruction.h"

/*
UVDInstructionShared
*/

UVDInstructionShared::UVDInstructionShared()
{
}

UVDInstructionShared::~UVDInstructionShared()
{
}

uv_err_t UVDInstructionShared::init()
{
	return UV_ERR_OK;
}

uv_err_t UVDInstructionShared::deinit()
{
	return UV_ERR_OK;
}

std::string UVDInstructionShared::getHumanReadableUsage()
{
	return "";
}

/*
UVDInstruction
*/

UVDInstruction::UVDInstruction()
{
}

UVDInstruction::~UVDInstruction()
{
}

uv_err_t UVDInstruction::init()
{
	return UV_ERR_OK;
}

uv_err_t UVDInstruction::deinit()
{
	return UV_ERR_OK;
}

