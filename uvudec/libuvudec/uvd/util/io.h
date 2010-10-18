/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UTIL_IO_H
#define UTIL_IO_H

#include "uvd/util/types.h"

/*
Print a string to all registered output callbacks
A newline should NOT be appended
It is assumed that a newline will be at the end
	Heck, its an error if the string contains a newline at all
	Undefined behavior if given
*/
uv_err_t UVDPrint(const std::string &s);
uv_err_t UVDPrintf(const char *format, ...);

typedef uv_err_t (*UVDPrintCallback)(const std::string &s, void *data);
/*
No order of call is gauranteed
A callback can only be registerd once
*/
uv_err_t UVDRegisterPrintCallback(UVDPrintCallback callback, void *data);
uv_err_t UVDUnregisterPrintCallback(UVDPrintCallback callback);

#endif

