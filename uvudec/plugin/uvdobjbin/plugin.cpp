/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvdobjbin/object.h"
#include "uvdobjbin/plugin.h"
#include "core/uvd.h"

UVDObjbinPlugin *g_objbinPlugin = NULL;

UVDObjbinPlugin::UVDObjbinPlugin()
{
	g_objbinPlugin = this;
}

UVDObjbinPlugin::~UVDObjbinPlugin()
{
}

uv_err_t UVDObjbinPlugin::init(UVDConfig *config)
{
	uv_assert_err_ret(UVDPlugin::init(config));
	return UV_ERR_OK;
}

uv_err_t UVDObjbinPlugin::getName(std::string &out)
{
	out = "uvdobjbin";
	return UV_ERR_OK;
}

uv_err_t UVDObjbinPlugin::getDescription(std::string &out)
{
	out = "Raw binary object loader";
	return UV_ERR_OK;
}

uv_err_t UVDObjbinPlugin::getVersion(UVDVersion &out)
{
	out.m_version = UVUDEC_VER_STRING;
	return UV_ERR_OK;
}

uv_err_t UVDObjbinPlugin::getAuthor(std::string &out)
{
	out = "John McMaster <JohnDMcMaster@gmail.com>";
	return UV_ERR_OK;
}

uv_err_t UVDObjbinPlugin::getObject(UVDData *data, const std::string &object, const std::string &architecture, UVDObject **out)
{
	return UV_DEBUG(UVDBinaryObject::tryLoad(data, object, architecture, out));
}

