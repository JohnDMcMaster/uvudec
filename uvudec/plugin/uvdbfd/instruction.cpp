/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvdbfd/instruction.h"

/*
UVDBFDInstructionShared
*/

/*
UVDBFDInstructionShared::UVDBFDInstructionShared()
{
}

UVDBFDInstructionShared::~UVDBFDInstructionShared()
{
}
*/

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

uv_err_t UVDBFDInstruction::print_disasm(std::string &out)
{
	out = m_disassembly;
	return UV_ERR_OK;
}

uv_err_t UVDBFDInstruction::analyzeControlFlow(UVDInstructionAnalysis *out)
{
	//This will stay the this way in the near future
	//get assembly printed, then work on doing actual analysis
	return UV_DEBUG(UV_ERR_NOTSUPPORTED);
}

