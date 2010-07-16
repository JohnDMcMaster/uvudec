/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd_config_flirt.h"
#include "uvd_flirt_args_property.h"

UVDConfigFLIRT::UVDConfigFLIRT()
{
	m_signatureLengthMin = UVD_PROP_FLIRT_MIN_SIGNATURE_LENGTH_DEFAULT;
	m_signatureLengthMax = UVD_PROP_FLIRT_MAX_SIGNATURE_LENGTH_DEFAULT;
	m_patternFileNewline = UVD_PROP_FLIRT_PAT_NEWLINE_DEFAULT;
	m_relocateNearFar = UVD_PROP_FLIRT_RELOCATION_NEARFAR_DEFAULT;
	
	m_outputFile = "/dev/stdout";
}

UVDConfigFLIRT::~UVDConfigFLIRT()
{
	deinit();
}

uv_err_t UVDConfigFLIRT::deinit()
{
	return UV_ERR_OK;
}
