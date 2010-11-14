/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvdasciiart/ascii_art.h"
#include "uvdasciiart/plugin.h"
#include "uvd/core/uvd.h"

UVDAsciiartPlugin *g_asciiArtPlugin = NULL;

UVDAsciiartPlugin::UVDAsciiartPlugin()
{
	g_asciiArtPlugin = this;
}

UVDAsciiartPlugin::~UVDAsciiartPlugin()
{
}

uv_err_t UVDAsciiartPlugin::init(UVDConfig *config)
{
	uv_assert_err_ret(UVDPlugin::init(config));
	uv_assert_err_ret(m_config.init(config));
	return UV_ERR_OK;
}

uv_err_t UVDAsciiartPlugin::getName(std::string &out)
{
	out = UVD_PLUGIN_NAME;
	return UV_ERR_OK;
}

uv_err_t UVDAsciiartPlugin::getDescription(std::string &out)
{
	out = "ASCII art example simple plugin";
	return UV_ERR_OK;
}

uv_err_t UVDAsciiartPlugin::getVersion(UVDVersion &out)
{
	out.m_version = UVUDEC_VER_STRING;
	return UV_ERR_OK;
}

uv_err_t UVDAsciiartPlugin::getAuthor(std::string &out)
{
	out = "John McMaster <JohnDMcMaster@gmail.com>";
	return UV_ERR_OK;
}

uv_err_t UVDAsciiartPlugin::outputHeader(std::vector<std::string> &lines)
{
	std::string art;
	
	if( !m_config.m_outputASCIIArt )
	{
		return UV_ERR_OK;
	}

	uv_assert_err_ret(getRandomUVNetASCIIArt(art));
	art += "\n";
	lines.push_back(art);
	lines.push_back("");
	lines.push_back("");		

	return UV_ERR_OK;
}

