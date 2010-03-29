/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#ifdef UVD_FLIRT_PATTERN_UVD

#include "uvd_library.h"
#include "uvd_elf_library.h"

UVDLibrary::UVDLibrary()
{
	m_flirt = NULL;
	m_data = NULL;
}

UVDLibrary::~UVDLibrary()
{
	deinit();
}

uv_err_t UVDLibrary::init()
{
	return UV_ERR_OK;
}

uv_err_t UVDLibrary::deinit()
{
	delete m_data;
	m_data = NULL;
	
	return UV_ERR_OK;
}

uv_err_t UVDLibrary::prepareFLIRTAnalysis(UVDFLIRTPatternAnalysis *flirtEntries)
{
	uv_assert_ret(flirtEntries);
	return UV_ERR_OK;
}

uv_err_t UVDLibrary::getLibrary(UVDData *data, UVDLibrary **libraryOut)
{
	if( UV_SUCCEEDED(UVDElfLibrary::getLibrary(data, libraryOut)) )
	{
		return UV_ERR_OK;
	}
#ifdef USING_LIBBFD
	else if( UV_SUCCEEDED(UVDBFDLibrary::getLibrary(data, libraryOut)) )
	{
		return UV_ERR_OK;
	}
#endif
	else
	{
	}
	
	return UV_ERR_OK;
}

#endif
