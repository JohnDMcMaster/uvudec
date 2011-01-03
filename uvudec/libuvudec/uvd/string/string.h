/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_STRING_STRING_H
#define UVD_STRING_STRING_H

#include "uvd/assembly/address.h"

/*
from man strings:
-e encoding
--encoding=encoding
   Select the character encoding of the strings that are to be found.  Possible values for encoding are: s = single-7-bit-byte
   characters (ASCII, ISO 8859, etc., default), S = single-8-bit-byte characters, b = 16-bit bigendian, l = 16-bit
   littleendian, B = 32-bit bigendian, L = 32-bit littleendian.  Useful for finding wide character strings. (l and b apply to,
   for example, Unicode UTF-16/UCS-2 encodings).

C
	ASCII null terminated
COBOL
	First byte is length?
EBCDIC
	Some old encoding, not familar with it
	dd also lists "ibm" option as EBCDIC alternative
wchar_t *
	Think this varies a lot between the powers
	BSD
	Linux
	Mac
	Windows
Windows specific
	COM
		BSTR
	LPCSTR
		long pointer to C string, standard C string

Assorted other archeaic encodings
*/
#define UVD_STRING_ENCODING_UNKNOWN				0
//Standard null terminated C/ASCII
#define UVD_STRING_ENCODING_ASCII				1
#define UVD_STRING_ENCODING_BIG_ENDIAN16		2
#define UVD_STRING_ENCODING_LITTLE_ENDIAN16		3
#define UVD_STRING_ENCODING_BIG_ENDIAN32		4
#define UVD_STRING_ENCODING_LITTLE_ENDIAN32		5
#define UVD_STRING_ENCODING_EBCDIC				6
typedef int UVDStringEncoding;

class UVDString
{
public:
	UVDString();
	UVDString(UVDAddressRange addressRange, UVDStringEncoding encoding = UVD_STRING_ENCODING_ASCII);
	~UVDString();

public:
	UVDStringEncoding m_encoding;
	UVDAddressRange m_addressRange;
};

#endif

