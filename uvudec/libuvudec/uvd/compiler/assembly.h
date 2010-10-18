/*
UVNet Universal Decompiler (uvudec)
Copyright 2009 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_COMPILER_ASSEMBLY_H
#define UVD_COMPILER_ASSEMBLY_H

#include "uvd/compiler/compiler.h"

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
