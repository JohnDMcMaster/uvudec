/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/config/plugin.h"
#include "uvd/config/arg_property.h"

/*
UVDPluginConfig
*/

UVDPluginConfig::UVDPluginConfig()
{
	m_activateAll = UVD_PROP_PLUGIN_ACTIVATE_ALL_DEFAULT;
}

UVDPluginConfig::~UVDPluginConfig()
{
	UV_DEBUG(deinit());
}

uv_err_t UVDPluginConfig::init(UVDConfig *config)
{
	uv_assert_err_ret(earlyArgParse(config));

	return UV_ERR_OK;
}

uv_err_t UVDPluginConfig::deinit()
{
	for( UVDArgConfigs::iterator iter = m_earlyConfigArgs.begin(); iter != m_earlyConfigArgs.end(); ++iter )
	{
		delete (*iter).second;
	}
	m_earlyConfigArgs.clear();

	return UV_ERR_OK;
}

uv_err_t UVDPluginConfig::addToLoad(const std::string &pluginLibraryName)
{
	m_pluginFiles.push_back(pluginLibraryName);
	return UV_ERR_OK;
}

uv_err_t UVDPluginConfig::addToActivate(const std::string &pluginName)
{
	m_toActivate.push_back(pluginName);
	return UV_ERR_OK;
}

uv_err_t UVDPluginConfig::appendPluginPath(const std::string &path)
{
	m_dirs.insert(m_dirs.end(), path);
	return UV_ERR_OK;
}

uv_err_t UVDPluginConfig::prependPluginPath(const std::string &path)
{
	m_dirs.insert(m_dirs.begin(), path);
	return UV_ERR_OK;
}

