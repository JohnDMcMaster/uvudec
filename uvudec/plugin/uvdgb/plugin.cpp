/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvdgb/object.h"
#include "uvdgb/plugin.h"
#include "uvd/core/uvd.h"

UVDObjgbPlugin *g_objbinPlugin = NULL;

UVDObjgbPlugin::UVDObjgbPlugin()
{
	g_objbinPlugin = this;
}

UVDObjgbPlugin::~UVDObjgbPlugin()
{
}

uv_err_t UVDObjgbPlugin::init(UVDConfig *config)
{
	uv_assert_err_ret(UVDPlugin::init(config));
	return UV_ERR_OK;
}

uv_err_t UVDObjgbPlugin::getName(std::string &out)
{
	out = UVD_PLUGIN_NAME;
	return UV_ERR_OK;
}

uv_err_t UVDObjgbPlugin::getDescription(std::string &out)
{
	out = "Game Boy ROM object loader";
	return UV_ERR_OK;
}

uv_err_t UVDObjgbPlugin::getVersion(UVDVersion &out)
{
	out.m_version = UVUDEC_VER_STRING;
	return UV_ERR_OK;
}

uv_err_t UVDObjgbPlugin::getAuthor(std::string &out)
{
	out = "John McMaster <JohnDMcMaster@gmail.com>";
	return UV_ERR_OK;
}

uv_err_t UVDObjgbPlugin::canGetObject(const UVDData *data, const UVDRuntimeHints &hints, uvd_priority_t *confidence)
{
	return UV_DEBUG(UVDGBObject::canLoad(data, hints, confidence, this));
}

uv_err_t UVDObjgbPlugin::getObject(UVDData *data, const UVDRuntimeHints &hints, UVDObject **out)
{
	return UV_DEBUG(UVDGBObject::tryLoad(data, hints, out, this));
}

