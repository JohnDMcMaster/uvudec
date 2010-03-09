/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#pragma once

#include "uvd_compiler.h"

/*
TODO: clean this up.  Its left over from C stuff
Make a formatter class (and use it...)
*/


#define UV_DISASM_RET_BUFF_SIZE			1024
#define PRINT_OPERAND_SEPERATOR			", "

void printFormatting();

class UVDFormat
{
public:
	UVDFormat();
	~UVDFormat();
	
	uv_err_t init();
	uv_err_t deinit();
	
	std::string formatAddress(uint32_t address);
	std::string formatRegister(const std::string &reg);
	
	void printFormatting();

public:
	//Output data has to be formatted to a given compiler
	//This is owned by this object
	UVDCompiler *m_compiler;
};

