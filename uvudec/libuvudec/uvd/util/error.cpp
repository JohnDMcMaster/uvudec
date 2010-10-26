/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/util/error.h"
#include "uvd/config/config.h"
#include <stdio.h>
#include <stdarg.h>

uv_err_t uv_err_ret_handler(uv_err_t rc, const char *file, uint32_t line, const char *func)
{
	if( !UVDAnyDebugActive() )
	{
		return UV_ERR_OK;
	}

	if( UV_FAILED(rc) )
	{
		UVDPrintfError("%s (%s:%d): rc=%s\n", func, file, line, uv_err_str(rc));
	}
	else if( UV_WARNING(rc) )
	{
		UVDPrintfWarning("%s (%s:%d): rc=%s\n", func, file, line, uv_err_str(rc));
	}
	return rc;
}

#define uv_err_str_case(x) case x: return #x;
const char *uv_err_str(uv_err_t err)
{
	switch(err)
	{
	uv_err_str_case(UV_ERR_GENERAL);
	uv_err_str_case(UV_ERR_ACCESS);
	uv_err_str_case(UV_ERR_OUTMEM);
	uv_err_str_case(UV_ERR_NOTFOUND);
	uv_err_str_case(UV_ERR_ABORTED);
	uv_err_str_case(UV_ERR_ARGS);
	uv_err_str_case(UV_ERR_NOTSUPPORTED);
	uv_err_str_case(UV_ERR_BUFFERSIZE);
	uv_err_str_case(UV_ERR_ARBITRARYLIMIT);
	uv_err_str_case(UV_ERR_COMPATIBILITY);
	uv_err_str_case(UV_ERR_NOTIMPLEMENTED);

	uv_err_str_case(UV_ERR_DISASM_COMBO);
	uv_err_str_case(UV_ERR_DISASM_NODAT);
	uv_err_str_case(UV_ERR_DISASM_PREFIX);

	default:
		return "UNKNOWN";
	}
}

void printf_debug_error(const char *format, ...)
{
	//If we are in any sort of debug state, report errors
	if( g_config && g_config->anyVerboseActive() )
	{
		va_list ap;

		va_start(ap, format);
		UVDPrintfErrorV(format, ap);
		va_end(ap);
	}
}

