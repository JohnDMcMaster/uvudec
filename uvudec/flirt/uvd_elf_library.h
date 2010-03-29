/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#ifndef UVD_ELF_LIBRARY_H
#define UVD_ELF_LIBRARY_H

#ifdef UVD_FLIRT_PATTERN_UVD

#include "uvd_library.h"
#include "uvd_types.h"

class UVDElfLibrary : public UVDLibrary
{
public:
	UVDElfLibrary();
	~UVDElfLibrary();
	uv_err_t init();
	uv_err_t deinit();

	uv_err_t prepareFLIRTAnalysis(UVDFLIRTPatternAnalysis *flirtEntries);

	//Does the data passed in seem parsable by this library?
	static uv_err_t canParse(UVDData *data, uint32_t *canParse);
	
	static uv_err_t getLibrary(UVDData *data, UVDLibrary **libraryOut);
};

#endif

#endif
