/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

/*
For use with FLIRT
*/

#ifndef UVD_LIBRARY_H
#define UVD_LIBRARY_H

#include "flirt/flirt.h"
#include "uvd_data.h"

class UVDFLIRTPatternAnalysis;
class UVDLibrary
{
public:
	UVDLibrary();
	~UVDLibrary();
	uv_err_t init();
	uv_err_t deinit();

	/*
	Get a list of all of the functions this lib exposes
	This is the starting point for FLIRT analysis
	Should return all coherent entries and not do any filtering
	*/
	uv_err_t prepareFLIRTAnalysis(UVDFLIRTPatternAnalysis *flirtEntries);

	//Selects the best library wrapper based on the file data
	//Ownership of data will be given to the library object
	static uv_err_t getLibrary(UVDData *data, UVDLibrary **libraryOut);

public:
	//The FLIRT engine we are part of
	UVDFLIRT *m_flirt;
	//The library source data, we own this
	UVDData *m_data;
};

#endif
