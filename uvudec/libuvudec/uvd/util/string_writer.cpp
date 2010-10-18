/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include <stdio.h>
#include <stdarg.h>
#include "uvd/util/string_writer.h"

static std::string g_outputBuff;
void UVDStringWriter::print(const char *format, ...)
{
	va_list ap;
	char buff[512];
	
	va_start(ap, format);
	vsnprintf(buff, sizeof(buff), format, ap);
	m_buffer += buff;

	va_end(ap);
}

