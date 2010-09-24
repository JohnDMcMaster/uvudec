/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_PLUGIN_ENGINE_H
#define UVD_PLUGIN_ENGINE_H

#include "uvd_types.h"
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

	//Load plugin at given path
	//needs an exported symbol called UVDPluginMain
	//Obviously this will error under a static build
	uv_err_t loadByPath(const std::string &path, bool reportErrors = true);
	
	//This actually activates a plugin for use
	//Error if the plugin was not previously loaded
	uv_err_t initPlugin(const std::string &name);
	uv_err_t deinitPlugin(const std::string &name);
	
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

