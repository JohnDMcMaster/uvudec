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
}

UVDPlugin::~UVDPlugin()
{
	if( m_hLibrary )
	{
		dlclose(m_hLibrary);
	}
}

uv_err_t UVDPlugin::init()
{
	return UV_ERR_OK;
}

uv_err_t UVDPlugin::deinit()
{
	return UV_ERR_OK;
}

uv_err_t UVDPlugin::initEntry()
{
	return UV_DEBUG(init());
}

uv_err_t UVDPlugin::deinitEntry()
{
	return UV_DEBUG(deinit());
}

uv_err_t UVDPlugin::getDependencies(PluginDependencies &out)
{
	out.clear();
	return UV_ERR_OK;
}

