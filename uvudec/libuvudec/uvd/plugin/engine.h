/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_PLUGIN_ENGINE_H
#define UVD_PLUGIN_ENGINE_H

#include "uvd/util/types.h"
#include <map>
#include <string>

/*
NOTE NOTE NOTE
This is still active code even if a static build
The plugins are just staticaly linked in is all
*/
class UVD;
class UVDPlugin;
class UVDConfig;
class UVDArgConfig;
class UVDPluginEngine
{
public:
	//Just because a plugin is registered, does not mean it is active
	UVDPluginEngine();
	~UVDPluginEngine();
	
	//Including finding all of the plugins in our config search path
	//they will have their main's called to initialize argument structures
	uv_err_t init(UVDConfig *config);
	uv_err_t deinit();
	
	//Loading: makes it ready for use (calls UVDPluginMain), but will not activate it (no plugin->init())
	
	//Try to find a plugin library matching given name and OS specific junk
	//eg: uvdasm might try to find libuvdasm.so on Linux
	uv_err_t loadByName(const std::string &name);

	/*
	Try to batch load an entire directory
	failOnBad: if any file in the dir fails to load, return error
	failOnBadPlugin: only return error if it demonstrated reasonable ability to be a plugin
	*/
	uv_err_t loadByDir(const std::string &pluginDir, UVDConfig *config,
			bool recursive = false,
			bool failOnBad = false, bool failOnBadPlugin = false);

	/*
	Load plugin at given path
	needs an exported symbol called UVDPluginMain
	Obviously this will error under a static build
	reportErrors: set to false to ignore printing errors on something that might not be a plugin
	Returns UV_ERR_NOTSUPPORTED if we don't think its a plugin, UV_ERR_GENERAL if we do and it errors
	*/
	uv_err_t loadByPath(const std::string &path, bool reportErrors = true);
	
	//This actually activates a plugin for use
	//Error if the plugin was not previously loaded
	uv_err_t activatePluginByName(const std::string &name);
	uv_err_t ensurePluginActiveByName(const std::string &name);
	uv_err_t deactivatePluginByName(const std::string &name);
	
	uv_err_t onUVDInit();
	uv_err_t onUVDDeinit();

	//In an acceptable dependency order	
	uv_err_t getAllPluginDependencies(const std::string &name, std::vector<UVDPlugin *> &out);
	uv_err_t getPluginDependencyOrder(std::vector<UVDPlugin *> &out);

protected:
	//Initialize statically linked plugins
	uv_err_t staticInit();

public:
	//We might want to map this from something like the plugin name
	std::map<std::string, UVDPlugin *> m_plugins;
	std::map<std::string, UVDPlugin *> m_loadedPlugins;
	std::map<UVDArgConfig *, std::string> m_pluginArgMap;
	//WARNING: this won't get set until later
	UVD *m_uvd;
};

#endif

