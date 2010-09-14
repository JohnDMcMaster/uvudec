/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "util/io.h"
#include <map>
#include <stdarg.h>

//Default set sort is pointer order, which is what we need
typedef std::map<UVDPrintCallback, void *> UVDPrintCallbacks;
static UVDPrintCallbacks g_printCallbacks;

uv_err_t UVDPrint(const std::string &s)
{
	for( UVDPrintCallbacks::iterator iter = g_printCallbacks.begin(); iter != g_printCallbacks.end(); ++iter )
	{
		UVDPrintCallback callback = (*iter).first;

		if( callback )
		{
			UV_DEBUG(callback(s, (*iter).second));
		}
	}
	return UV_ERR_OK;
}

uv_err_t UVDPrintf(const char *format, ...)
{
	va_list ap;
	char buff[512];

	va_start(ap, format);
	vsnprintf(buff, sizeof(buff), format, ap);
	uv_assert_err_ret(UVDPrint(buff));
	va_end(ap);
	
	return UV_ERR_OK;
}

uv_err_t UVDRegisterPrintCallback(UVDPrintCallback callback, void *data)
{
	g_printCallbacks[callback] = data;
	return UV_ERR_OK;
}

uv_err_t UVDUnregisterPrintCallback(UVDPrintCallback callback)
{
	if( g_printCallbacks.find(callback) != g_printCallbacks.end() )
	{
		g_printCallbacks.erase(callback);
	}
	return UV_ERR_OK;
}

