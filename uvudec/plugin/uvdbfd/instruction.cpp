/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvdbfd/instruction.h"

/*
UVDBFDInstructionShared
*/

UVDBFDInstructionShared::UVDBFDInstructionShared()
{
}

UVDBFDInstructionShared::~UVDBFDInstructionShared()
{
}

/*
UVDBFDInstruction
*/
UVDBFDInstruction::UVDBFDInstruction()
{
	m_architecture = NULL;
}

UVDBFDInstruction::~UVDBFDInstruction()
{
}

uv_err_t UVDBFDInstruction::parseCurrentInstruction(UVDIteratorCommon &iterCommon)
{
	return UV_DEBUG(UV_ERR_NOTSUPPORTED);
}

uv_err_t UVDBFDInstruction::print_disasm(std::string &out)
{
	return UV_DEBUG(UV_ERR_NOTSUPPORTED);
}

uv_err_t UVDBFDInstruction::analyzeControlFlow()
{
	return UV_DEBUG(UV_ERR_NOTSUPPORTED);
}

