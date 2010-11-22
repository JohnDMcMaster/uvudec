/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvdelf/object.h"
#include "uvdelf/plugin.h"
#include "uvd/core/uvd.h"

UVDELFPlugin *g_UVDELFPlugin = NULL;

UVDELFPlugin::UVDELFPlugin()
{
	g_UVDELFPlugin = this;
}

UVDELFPlugin::~UVDELFPlugin()
{
}

uv_err_t UVDELFPlugin::init(UVDConfig *config)
{
	uv_assert_err_ret(UVDPlugin::init(config));
	return UV_ERR_OK;
}

uv_err_t UVDELFPlugin::getName(std::string &out)
{
	out = UVD_PLUGIN_NAME;
	return UV_ERR_OK;
}

uv_err_t UVDELFPlugin::getDescription(std::string &out)
{
	out = "ELF object loader";
	return UV_ERR_OK;
}

uv_err_t UVDELFPlugin::getVersion(UVDVersion &out)
{
	out.m_version = UVUDEC_VER_STRING;
	return UV_ERR_OK;
}

uv_err_t UVDELFPlugin::getAuthor(std::string &out)
{
	out = "John McMaster <JohnDMcMaster@gmail.com>";
	return UV_ERR_OK;
}

uv_err_t UVDELFPlugin::canLoadObject(const UVDData *data, const UVDRuntimeHints &hints, uvd_priority_t *confidence)
{
	return UV_DEBUG(UVDElf::canLoad(data, hints, confidence));
}

uv_err_t UVDELFPlugin::loadObject(UVDData *data, const UVDRuntimeHints &hints, UVDObject **out)
{
	return UV_DEBUG(UVDElf::tryLoad(data, hints, out));
}

uv_err_t UVD_PLUGIN_MAIN_SYMBOL(UVDConfig *config, UVDPlugin **out)
{
	*out = new UVDELFPlugin();
	
	return UV_ERR_OK;
}

