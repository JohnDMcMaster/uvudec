/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVDGB_INSTRUCTION_H
#define UVDGB_INSTRUCTION_H

#include "uvd/util/types.h"

class UVDGBArchitecture;
class UVDGBInstruction : public UVDInstruction
{
public:
	UVDGBInstruction();
	~UVDGBInstruction();
	
	virtual uv_err_t init();
	virtual uv_err_t deinit();

	virtual uv_err_t parseCurrentInstruction(UVDIteratorCommon &iterCommon);
	virtual uv_err_t print_disasm(std::string &out);
	virtual uv_err_t analyzeControlFlow();

public:	
	UVDGBArchitecture *m_architecture;
};

#endif

