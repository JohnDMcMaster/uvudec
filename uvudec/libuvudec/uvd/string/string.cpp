/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/string/string.h"

/*
UVDString
*/

UVDString::UVDString()
{
	m_encoding  = UVD_STRING_ENCODING_UNKNOWN;
}

UVDString::UVDString(UVDAddressRange addressRange, UVDStringEncoding encoding)
{
	m_addressRange = addressRange;
	m_encoding = encoding;
}

UVDString::~UVDString()
{
}

uv_err_t UVDString::readString(std::string &out, bool safe) const
{
	return UV_DEBUG(m_addressRange.memoryToString(out, safe));
}

