/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_FORMAT_H
#define UVD_FORMAT_H

#include "uvd/compiler/compiler.h"

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
	virtual ~UVDFormat();
	
	virtual uv_err_t init();
	virtual uv_err_t deinit();
	
	virtual uv_err_t formatAddress(uint32_t address, std::string &out);
	virtual uv_err_t formatRegister(const std::string &reg, std::string &out);
	
	//Set as new and delete old if necessary
	//Ownership is transferred to this object
	uv_err_t setCompiler(UVDCompiler *compiler);
	
	void printFormatting();

public:
	//FIXME: add an object for a high level language (compiler) and a low level language (assembler)
	//Output data has to be formatted to a given compiler
	//Note this has nothing to do with with the compiler used to compile the code
	//This is owned by this object
	UVDCompiler *m_compiler;
};

#endif

