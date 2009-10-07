#pragma once

#include "uvd_compiler.h"

/*
Assembler common options
*/
class UVDCompilerAssembly : public UVDCompiler
{
public:
	UVDCompilerAssembly();
	
	virtual uv_err_t comment(const std::string &in, std::string &out);
	virtual uv_err_t commentAggressive(const std::string &in, std::string &out);
};
