/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd_decompiler.h"
#include "uvd_c_decompiler.h"
#include "uvd_compiler.h"

UVDDecompileNotes::UVDDecompileNotes()
{
	language = UVD_LANGUAGE_UNKNOWN;
}

uv_err_t UVDDecompileNotes::getOptimalLanguage(int &language)
{
	language = m_optimalLanguage;
}

UVDDecompiler::UVDDecompiler()
{
}

UVDDecompiler::~UVDDecompiler()
{
	deinit();
}

//Class specific init function
uv_err_t UVDDecompiler::init()
{
	return UV_ERR_OK;
}

uv_err_t UVDDecompiler::deinit()
{
	return UV_ERR_OK;
}

uv_err_t getDecompiler(UVDCompiler *compiler, UVDDecompiler **decompilerIn)
{
	UVDDecompiler *decompiler = NULL;
	
	uv_assert_ret(compiler);
	uv_assert_ret(decompilerIn);
	
	//Only one choice right now
	decompiler = new UVDCDecompiler();
	uv_assert_ret(decompiler);
	*decompilerIn = decompiler;
	
	return UV_ERR_OK;
}
