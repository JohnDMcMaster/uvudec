/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#if USING_VECTORS

#include "uvd_cpu.h"

UVDCPU::UVDCPU()
{
	m_opcodeTable = NULL;
}

uv_err_t UVDCPU::init()
{
	return UV_ERR_OK;
}

#endif
