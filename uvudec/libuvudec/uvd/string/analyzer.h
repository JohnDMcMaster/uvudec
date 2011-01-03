/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_STRINGS_H
#define UVD_STRINGS_H

#include "uvd/string/string.h"
#include "uvd/util/types.h"

class UVDStringsAnalyzer
{
public:
	UVDStringsAnalyzer();
	virtual ~UVDStringsAnalyzer();
	
	//virtual uv_err_t analyze() = 0;
	//Do not clear the vector
	virtual uv_err_t appendAllStrings(std::vector<UVDString> &out) = 0;
};

#endif

