/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_STRING_WRITTER_H
#define UVD_STRING_WRITTER_H

#include <string>

/*
printf() like formatting on a string buffer for std::string
*/
class UVDStringWritter
{
public:
	//Slightly diff than printf and avoids preprocessor issues
	void print(const char *format, ...);

public:
	std::string m_buffer;
};

#endif

