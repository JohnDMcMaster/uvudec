/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/object/object.h"
#include "uvd/plugin/plugin.h"
#include "uvd/core/uvd.h"

UVDObject::UVDObject()
{
	m_data = NULL;
}

UVDObject::~UVDObject()
{
	delete m_data;
	for( std::vector<UVDSection *>::iterator iter = m_sections.begin();
			iter != m_sections.end(); ++iter )
	{
		delete *iter;
	}
}
	
uv_err_t UVDObject::init(UVDData *data)
{
	m_data = data;
	return UV_ERR_OK;
}

uv_err_t UVDObject::addRelocation(UVDRelocationFixup *analysisRelocation)
{
	return UV_ERR_NOTSUPPORTED;
}

uv_err_t UVDObject::addFunction(UVDBinaryFunction *function)
{
	return UV_ERR_NOTSUPPORTED;
}

uv_err_t UVDObject::writeToFileName(const std::string &fileName)
{
	return UV_ERR_NOTSUPPORTED;
}

uv_err_t UVDObject::fromString(const std::string &type, UVDData *data, UVDObject **out)
{
	std::string pluginName;
	UVDPlugin *plugin = NULL;
	
	if( type.empty() )
	{
		return UV_DEBUG(g_uvd->initObject(data, UVDRuntimeHints(), out));
	}
	pluginName = UVDSplit(type, '.')[0];
	if( g_uvd->m_pluginEngine->m_loadedPlugins.find(pluginName) == g_uvd->m_pluginEngine->m_loadedPlugins.end() )
	{
		printf_error("cannot load object from type %s because plugin %s is not loaded\n", type.c_str(), pluginName.c_str());
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	plugin = (*g_uvd->m_pluginEngine->m_loadedPlugins.find(pluginName)).second;
	uv_assert_err_ret(plugin->loadObject(data, UVDRuntimeHints(), out));
	
	return UV_ERR_OK;
}

