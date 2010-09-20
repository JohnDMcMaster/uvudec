/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "plugin/plugin.h"
#include <dlfcn.h>

UVDPlugin::UVDPlugin()
{
	m_hLibrary = NULL;
	m_uvd = NULL;
}

UVDPlugin::~UVDPlugin()
{
	if( m_hLibrary )
	{
		dlclose(m_hLibrary);
	}
}

uv_err_t UVDPlugin::init(UVD *uvd)
{
	m_uvd = uvd;
	return UV_ERR_OK;
}

uv_err_t UVDPlugin::deinit()
{
	return UV_ERR_OK;
}

uv_err_t UVDPlugin::getDependencies(PluginDependencies &out)
{
	out.clear();
	return UV_ERR_OK;
}

uv_err_t UVDPlugin::getArchitecture(UVDData *data, const std::string &architecture, UVDArchitecture **out)
{
	return UV_ERR_NOTSUPPORTED;
}

