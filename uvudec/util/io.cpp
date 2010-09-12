/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "util/io.h"

//Default set sort is pointer order, which is what we need
static std::set<UVDPrintCallback> g_printCallbacks;

void UVDPrint(const std::string &s)
{
	for( std::set<UVDPrintCallback>::iteartor iter = g_printCallbacks.begin(); iter != g_printCallbacks.end(); ++iter )
	{
		UVDPrintCallback callback = *iter;

		uv_assert_ret(callback);
		callback();
	}
}

void UVDPrintf(const char *format, ...)
{
	va_list ap;
	char buff[512];

	va_start(ap, format);
	vsprintf(buff, format, ap);
	UVDPrint(buff);
	va_end(ap);
	
	return UV_ERR_OK;
}

uv_err_t UVDRegisterPrintCallback(UVDPrintCallback callback)
{
	g_printCallbacks.insert(callback);
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

