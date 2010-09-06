/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "flirt/config.h"
#include "flirt/args_property.h"

UVDConfigFLIRT::UVDConfigFLIRT()
{
	m_patSignatureLengthMin = UVD_PROP_FLIRT_MIN_SIGNATURE_LENGTH_DEFAULT;
	m_patSignatureLengthMax = UVD_PROP_FLIRT_MAX_SIGNATURE_LENGTH_DEFAULT;
	m_patternFileNewline = UVD_PROP_FLIRT_PAT_NEWLINE_DEFAULT;
	m_relocateNearFar = UVD_PROP_FLIRT_PAT_RELOC_NEARFAR_DEFAULT;
	m_patLeadingLength = UVD_PROP_FLIRT_PAT_LEADING_LENGTH_DEFAULT;
	m_patModuleLengthMax = UVD_PROP_FLIRT_PAT_MODULE_LENGTH_MAX_DEFAULT;

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
