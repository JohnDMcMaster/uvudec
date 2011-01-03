/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVDSTRINGS_STRINSG_H
#define UVDSTRINGS_STRINGS_H

#include "uvd/string/analyzer.h"

/*
Basic strings finder like the UNIX tool "strings"
Currently looks for C ASCII strings
In future will have to figure out how to treat different encodings

Other ways to find strings
-Strings table
	Resources, section, etc
-String arguments to functions
	ex: format string on printf
*/
class UVDStringsAnalyzerImpl : public UVDStringsAnalyzer
{
public:
	UVDStringsAnalyzerImpl(UVDStringsPlugin *plugin);
	~UVDStringsAnalyzerImpl();

	virtual uv_err_t appendAllStrings(std::vector<UVDStringsString> &out);

protected:
	uv_err_t doAppendAllStrings(UVDSection *section, std::vector<UVDAddressRange> &out);

public:
	UVDStringsPlugin *m_plugin;
	//Minimum printable length to be considered a string
	unsigned int m_minLength;
	//Encoding options?
};

#endif

