/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

/*
Uses libbfd (part of GNU binutils) for object file manipulation
"The GNU Binutils are a collection of binary tools."
http://sourceware.org/binutils/

THIS CODE IS NOT CURRENTLY BEING USED
*/

#if 0

#ifndef UVD_BFD_LIBRARY_H
#define UVD_BFD_LIBRARY_H

#if USING_LIBBFD

#include "uvd_library.h"
#include "uvd/util/types.h"

class UVDBFDLibrary : public UVDLibrary
{
public:
	UVDBFDLibrary();
	~UVDBFDLibrary();
	uv_err_t init();
	uv_err_t deinit();

	uv_err_t prepareFLIRTAnalysis(UVDFLIRTPatternAnalysis *flirtEntries);

	//Does the data passed in seem parsable by this library?
	static uv_err_t canParse(UVDData *data, uint32_t *canParse);
};

#endif

#endif

#endif
