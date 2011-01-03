/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvdstrings/config.h"
#include "uvdstrings/plugin.h"
#include "uvd/util/debug.h"

static uv_err_t argParser(const UVDArgConfig *argConfig, std::vector<std::string> argumentArguments, void *user)
{
	UVDConfig *config = NULL;
	//If present
	std::string firstArg;
	uint32_t firstArgNum = 0;
	
	config = g_config;
	uv_assert_ret(config);
	uv_assert_ret(config->m_argv);

	uv_assert_ret(argConfig);

	if( !argumentArguments.empty() )
	{
		firstArg = argumentArguments[0];
		firstArgNum = strtol(firstArg.c_str(), NULL, 0);
	}

	if( argConfig->m_propertyForm == UVDBFD_ARCHITECTURE_HINT )
	{
		uv_assert_ret(!argumentArguments.empty());
		g_bfdConfig->m_architecture = firstArg;
	}
	else
	{
		printf_error("Property not recognized in callback: %s\n", argConfig->m_propertyForm.c_str());
		return UV_DEBUG(UV_ERR_GENERAL);
	}

	return UV_ERR_OK;
}

UVDStringsConfig::UVDBFDConfig()
{
	g_bfdConfig = this;

	m_architecture = UVDBFD_ARCHITECTURE_HINT_DEFAULT;
}

UVDStringsConfig::~UVDBFDConfig()
{
}

uv_err_t UVDStringsConfig::init(UVDConfig *config)
{
	uv_assert_err_ret(g_bfdPlugin->registerArgument(UVDBFD_ARCHITECTURE_HINT, 0, "encoding", "string encodings to look for", 1, argParser, true));

	return UV_ERR_OK;
}

