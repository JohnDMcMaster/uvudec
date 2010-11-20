/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvdflirt/config.h"
#include "uvdflirt/args_property.h"

UVDFLIRTConfig *g_UVDFLIRTConfig = NULL;

UVDFLIRTConfig::UVDFLIRTConfig()
{
	g_UVDFLIRTConfig = this;
	
	m_FLAIRCompatibility = UVD_PROP_FLIRT_FLAIR_COMPATIBILITY_DEFAULT;
	m_patSignatureLengthMin = UVD_PROP_FLIRT_MIN_SIGNATURE_LENGTH_DEFAULT;
	m_patSignatureLengthMax = UVD_PROP_FLIRT_MAX_SIGNATURE_LENGTH_DEFAULT;
	m_patternFileNewline = UVD_PROP_FLIRT_PAT_NEWLINE_DEFAULT;
	m_relocateNearFar = UVD_PROP_FLIRT_PAT_RELOC_NEARFAR_DEFAULT;
	m_patLeadingLength = UVD_PROP_FLIRT_PAT_LEADING_LENGTH_DEFAULT;
	m_patModuleLengthMax = UVD_PROP_FLIRT_PAT_MODULE_LENGTH_MAX_DEFAULT;
	m_functionsAsModules = UVD_PROP_FLIRT_PAT_FUNCTIONS_AS_MODULES_DEFAULT;
	m_patternReferenceTrailingSpace = UVD_PROP_FLIRT_PAT_REFERENCE_TRAILING_SPACE_DEFAULT;
	m_patternPublicNameLengthMin = UVD_PROP_FLIRT_PAT_PUBLIC_NAME_LENGTH_MIN_DEFAULT;

	m_libName = UVD_PROP_FLIRT_SIG_LIB_NAME_DEFAULT;
	m_sigVersion = UVD_PROP_FLIRT_SIG_VERSION_DEFAULT;
	m_sigFeatures = UVD_PROP_FLIRT_SIG_FEATURES_DEFAULT;
	m_sigPad = UVD_PROP_FLIRT_SIG_PAD_DEFAULT;
	m_sigProcessorID = UVD_PROP_FLIRT_SIG_PROCESSOR_ID_DEFAULT;
	m_sigOSTypes = UVD_PROP_FLIRT_SIG_OS_TYPES_DEFAULT;
	m_sigAppTypes = UVD_PROP_FLIRT_SIG_APP_TYPES_DEFAULT;
	m_sigFileTypes = UVD_PROP_FLIRT_SIG_FILE_TYPES_DEFAULT;

	m_outputFile = "/dev/stdout";

	m_prefixUnderscores = UVD_PROP_FLIRT_PAT_PREFIX_UNDERSCORES_DEFAULT;
	m_debugDumpTab = "  ";
	
	makeFLAIRCompatible();
}

uv_err_t UVDFLIRTConfig::init()
{
	return UV_ERR_OK;
}

UVDFLIRTConfig::~UVDFLIRTConfig()
{
	deinit();
}

uv_err_t UVDFLIRTConfig::deinit()
{
	return UV_ERR_OK;
}

uv_err_t UVDFLIRTConfig::makeFLAIRCompatible(bool makeCompatible)
{
	m_FLAIRCompatibility = makeCompatible;
	m_prefixUnderscores = makeCompatible;
	//Don't think FLAIR does this, but will need some evidence
	m_trimZeros = !makeCompatible;
	m_functionsAsModules = !makeCompatible;
	//Changing this has a higher risk of breaking pat parsing programs where as the other options shouldn't
	m_patternFileNewline = "\r\n";
	m_patternReferenceTrailingSpace = makeCompatible;

	//m_relocateNearFar is settable in FLAIR I think

	return UV_ERR_OK;
}

