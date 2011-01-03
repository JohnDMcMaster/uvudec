/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/plugin/plugin.h"
#include "uvd/util/io.h"
#include "uvd/core/uvd.h"
#include "uvdstrings/plugin.h"
#include "uvd/core/uvd.h"

UVDStringsPlugin::UVDStringsPlugin()
{
}

UVDStringsPlugin::~UVDStringsPlugin()
{
}

uv_err_t UVDStringsPlugin::init(UVDConfig *config)
{
	uv_assert_err_ret(UVDPlugin::init(config));
	return UV_ERR_OK;
}

uv_err_t UVDStringsPlugin::getName(std::string &out)
{
	out = UVD_PLUGIN_NAME;
	return UV_ERR_OK;
}

uv_err_t UVDStringsPlugin::getDescription(std::string &out)
{
	out = "Basic strings analyzer";
	return UV_ERR_OK;
}

uv_err_t UVDStringsPlugin::getVersion(UVDVersion &out)
{
	out.m_version = UVUDEC_VER_STRING;
	return UV_ERR_OK;
}

uv_err_t UVDStringsPlugin::getAuthor(std::string &out)
{
	out = "John McMaster <JohnDMcMaster@gmail.com>";
	return UV_ERR_OK;
}

uv_err_t UVDPluginMain(UVDConfig *config, UVDPlugin **out)
{
	UVDPrint("Plugin " UVD_PLUGIN_NAME " loaded!\n");
	*out = new UVDStringsPlugin();
	return UV_ERR_OK;
}

