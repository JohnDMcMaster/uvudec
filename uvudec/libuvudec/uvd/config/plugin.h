/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_PLUGIN_CONFIG_H
#define UVD_PLUGIN_CONFIG_H

#include "uvd/plugin/plugin.h"
#include "uvd/plugin/engine.h"

class UVDPluginConfig
{
public:
	UVDPluginConfig();
	~UVDPluginConfig();

	uv_err_t init(UVDConfig *config);
	uv_err_t earlyArgParse(UVDConfig *config);
	uv_err_t deinit();

	//Add a plugin to be loaded
	uv_err_t addPlugin(const std::string &pluginLibraryName);
	uv_err_t appendPluginPath(const std::string &path);
	uv_err_t prependPluginPath(const std::string &path);

public:
	//To activate at startup
	//std::vector<std::string> m_plugins;
	//renamed
	std::vector<std::string> m_toLoad;

	//All of these will be added to plugin selection lists and have their main called
	//std::vector<std::string> m_pluginDirs;
	std::vector<std::string> m_dirs;

	//Since plugins are required for arg parsing, the plugin engine has to be part of the config
	UVDPluginEngine m_pluginEngine;

	//Plugin related early parsing so that main argument parse goes correctly
	UVDArgConfigs m_earlyConfigArgs;
};

#endif

