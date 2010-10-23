/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVDBFD_INSTRUCTION_H
#define UVDBFD_INSTRUCTION_H

#include "uvdasm/function.h"
#include "uvdasm/operand.h"
#include "uvdasm/util.h"

class UVDBFDInstructionShared : public UVDInstructionShared
{
public:
	UVDBFDInstructionShared();
	~UVDBFDInstructionShared();

public:
};

class UVDBFDArchitecture;
class UVDBFDInstruction : public UVDInstruction
{
public:
	UVDBFDInstruction();
	~UVDBFDInstruction();
	
	virtual uv_err_t parseCurrentInstruction(UVDIteratorCommon &iterCommon);
	virtual uv_err_t print_disasm(std::string &out);
	virtual uv_err_t analyzeControlFlow();

public:	
	UVDBFDArchitecture *m_architecture;
};

#endif

