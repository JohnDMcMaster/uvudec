/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

/*
Uses SRecord package for object file manipulation
"The SRecord package is a collection of powerful tools for manipulating EPROM load files."
http://srecord.sourceforge.net/

Since SRecord is part of GPL licensed code, it can only be supported if
code is GPL licensed.  This is really only an issue if you are trying
to distribute a binary
*/

#ifndef UVD_SRECORD_PACKAGE_LIBRARY_H
#define UVD_SRECORD_PACKAGE_LIBRARY_H

#if USING_SRECORD_PACKAGE

#include "uvd_library.h"
#include "uvd_types.h"

class UVDSRecordPackageLibrary : public UVDLibrary
{
public:
	UVDSRecordPackageLibrary();
	~UVDSRecordPackageLibrary();
	uv_err_t init();
	uv_err_t deinit();

	uv_err_t prepareFLIRTAnalysis(UVDFLIRTPatternAnalysis *flirtEntries);

	//Does the data passed in seem parsable by this library?
	static uv_err_t canParse(UVDData *data, uint32_t *canParse);
};

#endif

#endif
