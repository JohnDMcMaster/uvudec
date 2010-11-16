/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvdobjbin/object.h"
#include "uvdobjbin/plugin.h"
#include "uvd/core/uvd.h"

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

uv_err_t UVDObjbinPlugin::canLoadObject(const UVDData *data, const UVDRuntimeHints &hints, uvd_priority_t *confidence)
{
	return UV_DEBUG(UVDBinaryObject::canLoad(data, hints, confidence, this));
}

uv_err_t UVDObjbinPlugin::loadObject(UVDData *data, const UVDRuntimeHints &hints, UVDObject **out)
{
	return UV_DEBUG(UVDBinaryObject::tryLoad(data, hints, out, this));
}

