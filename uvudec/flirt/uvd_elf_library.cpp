/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#ifdef UVD_FLIRT_PATTERN_UVD

#include "uvd_elf_library.h"

UVDElfLibrary::UVDElfLibrary()
{
}

UVDElfLibrary::~UVDElfLibrary()
{
	deinit();
}

uv_err_t UVDElfLibrary::init()
{
	return UV_ERR_OK;
}

uv_err_t UVDElfLibrary::deinit()
{
	return UV_ERR_OK;
}

uv_err_t UVDElfLibrary::canParse(UVDData *data, uint32_t *canParseOut)
{
	//Yes, always.  What other format would you want to use?
	*canParseOut = true;
	return UV_ERR_OK;
}

uv_err_t UVDElfLibrary::getLibrary(UVDData *data, UVDLibrary **libraryOut)
{
	uint32_t canParseResult = false;
	UVDLibrary *library = NULL;
	
	uv_assert_err_ret(canParse(data, &canParseResult));
	if( !canParseResult )
	{
		return UV_ERR_GENERAL;
	}

	library = new UVDElfLibrary();
	uv_assert_ret(library);
	
	uv_assert_ret(libraryOut);
	*libraryOut = library;
	
	return UV_ERR_OK;
}

#endif
