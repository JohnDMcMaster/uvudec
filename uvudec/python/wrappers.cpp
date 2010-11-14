/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "wrappers.h"

//static uv_err_t getUVDFromFileName(UVD **uvdOut, const std::string &file);
UVD *uvd::getUVDFromFileName(const char *fileName)
{
	UVD *ret = NULL;
	
	UVD_SWIG_ASSERT_ERR(UVD::getUVDFromFileName(&ret, fileName));
	return ret;
}

uv_err_t always_return_rc(int rc)
{
	return UV_DEBUG(rc);
}

uv_err_t returns_string(std::string &out)
{
	out = "pizza";
	return UV_ERR_OK;
}

