/*
UVNet Universal Decompiler (uvudec)
Copyright 2009 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd_compiler_assembly.h"

UVDCompilerAssembly::UVDCompilerAssembly()
{
	m_compiler = UVD_COMPILER_UNKNOWN;
}

uv_err_t UVDCompilerAssembly::comment(const std::string &in, std::string &out)
{
	out = "# " + in;
	return UV_ERR_OK;
}
