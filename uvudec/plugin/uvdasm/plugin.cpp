/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvdasm/architecture.h"
#include "uvdasm/plugin.h"
#include "uvd/core/uvd.h"

UVDAsmPlugin *g_asmPlugin = NULL;

UVDAsmPlugin::UVDAsmPlugin()
{
	g_asmPlugin = this;
}

UVDAsmPlugin::~UVDAsmPlugin()
{
}

uv_err_t UVDAsmPlugin::init(UVDConfig *config)
{
	uv_assert_err_ret(UVDPlugin::init(config));
	uv_assert_err_ret(m_config.init(config));
	return UV_ERR_OK;
}

uv_err_t UVDAsmPlugin::onUVDInit(UVD *uvd)
{
#ifdef UVD_FLIRT_PATTERN_UVD
	UVDFLIRTPatternGeneratorUVD *generatorUVD = NULL;
	
	uv_assert_err_ret(UVDFLIRTPatternGeneratorUVD::getPatternGenerator(&generatorUVD));
	m_patternGenerators.push_back(generatorUVD);
#endif
	return UV_ERR_OK;
}

uv_err_t UVDAsmPlugin::getName(std::string &out)
{
	out = "uvdasm";
	return UV_ERR_OK;
}

uv_err_t UVDAsmPlugin::getDescription(std::string &out)
{
	out = "Configuration file based assembly engine";
	return UV_ERR_OK;
}

uv_err_t UVDAsmPlugin::getVersion(UVDVersion &out)
{
	out.m_version = UVUDEC_VER_STRING;
	return UV_ERR_OK;
}

uv_err_t UVDAsmPlugin::getAuthor(std::string &out)
{
	out = "John McMaster <JohnDMcMaster@gmail.com>";
	return UV_ERR_OK;
}

uv_err_t UVDAsmPlugin::canGetArchitecture(const UVDObject *object, const UVDRuntimeHints &hints, uvd_priority_t *confidence)
{
	/*
	If a file was specified, we should assume we are a good match for it
	Alternativly, we'd have to rummage through our config files and see if any disassemble sanely
	Likely this will require more specialized behavior, such as registering a handler for each architecture file we have
	*/
	std::string defaultArchitectureFile;
	
	uv_assert_ret(confidence);
	//If explictly specified, seems like a reasonable choice
	if( g_asmConfig->m_architectureFileName.empty() )
	{
		*confidence = UVD_MATCH_ACCEPTABLE;
	}
	//This is probably more of an archaec option from before I had config files
	//Likely will be removed in the future
	else if( UV_SUCCEEDED(g_asmConfig->getDefaultArchitectureFile(defaultArchitectureFile)) && !defaultArchitectureFile.empty() )
	{
		*confidence = UVD_MATCH_POOR;
	}
	else
	{
		*confidence = UVD_MATCH_NONE;
	}
	
	return UV_ERR_OK;
}

uv_err_t UVDAsmPlugin::getArchitecture(UVDObject *object, const UVDRuntimeHints &hints, UVDArchitecture **out)
{
	uv_assert_ret(out);
	*out = new UVDDisasmArchitecture();
	uv_assert_ret(*out);
	return UV_ERR_OK;
}

