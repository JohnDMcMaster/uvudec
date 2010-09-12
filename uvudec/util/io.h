/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UTIL_IO_H
#define UTIL_IO_H

#include "uvd_types.h"

/*
Print a string to all registered output callbacks
*/
void UVDPrint(const std::string &s);
void UVDPrintf(const char *format, ...);

typedef void (*UVDPrintCallback)(const std::string &s);
/*
No order of call is gauranteed
*/
uv_err_t UVDRegisterPrintCallback(UVDPrintCallback callback);
uv_err_t UVDUnregisterPrintCallback(UVDPrintCallback callback);

#endif

