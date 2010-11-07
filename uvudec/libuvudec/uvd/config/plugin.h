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

	//Add a plugin to be loaded by file name
	uv_err_t addToLoad(const std::string &fileName);
	uv_err_t addToActivate(const std::string &logicalName);
	uv_err_t appendPluginPath(const std::string &path);
	uv_err_t prependPluginPath(const std::string &path);

public:
	//Load find the libraries and add them to the availible plugin list
	std::vector<std::string> m_pluginFiles;
	//To activate at startup
	std::vector<std::string> m_toActivate;

	//All of these will be added to plugin selection lists and have their main called
	//std::vector<std::string> m_pluginDirs;
	std::vector<std::string> m_dirs;

	//Since plugins are required for arg parsing, the plugin engine has to be part of the config
	UVDPluginEngine m_pluginEngine;

	//Plugin related early parsing so that main argument parse goes correctly
	UVDArgConfigs m_earlyConfigArgs;
	
	//Instead of explictly saying which plugins to activate, just load every plugin we find?
	//Activating a plugin could imply altering analysis, so use with caution
	//UVD_PROP_PLUGIN_LOAD_ALL
	uvd_bool_t m_activateAll;
};

#endif

