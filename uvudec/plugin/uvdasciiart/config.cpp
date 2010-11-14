/*
UVNet Universal Decompiler (uvudec)
Copyright 2009 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvdasciiart/ascii_art.h"
#include "uvdasciiart/config.h"
#include "uvdasciiart/plugin.h"
#include "uvd/config/arg_property.h"
#include "uvd/config/arg_util.h"

UVDASCIIArtConfig *g_asciiArtConfig = NULL;

static uv_err_t argParser(const UVDArgConfig *argConfig, std::vector<std::string> argumentArguments)
{
	//If present
	std::string firstArg;
	bool firstArgBool = true;
	
	uv_assert_ret(argConfig);

	if( !argumentArguments.empty() )
	{
		firstArg = argumentArguments[0];
		firstArgBool = UVDArgToBool(firstArg);
	}

	if( argConfig->m_propertyForm == UVD_PROP_ACTION_USELESS_ASCII_ART )
	{
		std::string art;
		uv_assert_err_ret(getRandomUVNetASCIIArt(art));
		printf("Have too much time on our hands do we?\n%s\n\n", art.c_str());
		return UV_ERR_DONE;
	}
	else if( argConfig->m_propertyForm == UVD_PROP_OUTPUT_USELESS_ASCII_ART )
	{
		g_asciiArtConfig->m_outputASCIIArt = firstArgBool;
	}
	else
	{
		printf_error("Property not recognized in callback: %s\n", argConfig->m_propertyForm.c_str());
		return UV_DEBUG(UV_ERR_GENERAL);
	}

	return UV_ERR_OK;
}


UVDASCIIArtConfig::UVDASCIIArtConfig()
{
	m_outputASCIIArt = UVD_PROP_OUTPUT_USELESS_ASCII_ART_DEFAULT;
	g_asciiArtConfig = this;
}

UVDASCIIArtConfig::~UVDASCIIArtConfig()
{
	UV_DEBUG(deinit());
}

uv_err_t UVDASCIIArtConfig::init(UVDConfig *config)
{
	uv_assert_err_ret(g_asciiArtPlugin->registerArgument(UVD_PROP_ACTION_USELESS_ASCII_ART, 0, "print-useless-ascii-art", "print nifty ASCII art", 1, argParser, true));	
	uv_assert_err_ret(g_asciiArtPlugin->registerArgument(UVD_PROP_OUTPUT_USELESS_ASCII_ART, 0, "useless-ascii-art", "append nifty ascii art headers to output files", 1, argParser, true));
	
	return UV_ERR_OK;
}

uv_err_t UVDASCIIArtConfig::deinit()
{
	return UV_ERR_OK;
}

