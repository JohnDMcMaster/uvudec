/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
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
	
	//Set as new and delete old if necessary
	//Ownership is transferred to this object
	uv_err_t setCompiler(UVDCompiler *compiler);
	
	void printFormatting();

public:
	//Output data has to be formatted to a given compiler
	//Note this has nothing to do with with the compiler used to compile the code
	//This is owned by this object
	UVDCompiler *m_compiler;
};
