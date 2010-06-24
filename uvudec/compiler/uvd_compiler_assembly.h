/*
UVNet Universal Decompiler (uvudec)
Copyright 2009 John McMaster <JohnDMcMaster@gmail.com>
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#ifndef UVD_COMPILER_ASSEMBLY_H
#define UVD_COMPILER_ASSEMBLY_H

#include "uvd_compiler.h"

/*
Assembler common options
*/
class UVDCompilerAssembly : public UVDCompiler
{
public:
	UVDCompilerAssembly();
	
	virtual uv_err_t comment(const std::string &in, std::string &out);
};

#endif
