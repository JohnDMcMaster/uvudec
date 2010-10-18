/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/language/c_decompiler.h"

UVDCDecompiler::UVDCDecompiler()
{
}

UVDCDecompiler::~UVDCDecompiler()
{
	deinit();
}

uv_err_t UVDCDecompiler::init()
{
	uv_assert_err_ret(UVDDecompiler::init());
	return UV_ERR_OK;
}

uv_err_t UVDCDecompiler::deinit()
{
	uv_assert_err_ret(UVDDecompiler::deinit());
	return UV_ERR_OK;
}

uv_err_t decompile(std::vector<UVDInstruction *> disassembledCode, std::string &highLevelCode, UVDDecompileNotes *notes)
{
	//HC08 example, but should be about the same
	//_asm cli _endasm;
	return UV_ERR_OK;
}
