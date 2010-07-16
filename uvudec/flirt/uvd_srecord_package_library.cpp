/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifdef USING_LIBBFD

UVDSRecordPackageLibrary::UVDSRecordPackageLibrary()
{
}

UVDSRecordPackageLibrary::~UVDSRecordPackageLibrary()
{
	deinit();
}

uv_err_t UVDSRecordPackageLibrary::init()
{
	return UV_ERR_OK;
}

uv_err_t UVDSRecordPackageLibrary::deinit()
{
	return UV_ERR_OK;
}

uv_err_t UVDSRecordPackageLibrary::prepareFLIRTAnalysis(UVDFLIRTPatternAnalysis *flirtEntries)
{
	return UV_DEBUG(UV_ERR_UNSUPPORTED);
}

uv_err_t UVDSRecordPackageLibrary::canParse(UVDData *data, uint32_t *canParse)
{
	*canParse = false;
	return UV_ERR_OK;
}

#endif
