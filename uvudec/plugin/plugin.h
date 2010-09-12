/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef PLUGIN_PLUGIN_H
#define PLUGIN_PLUGIN_H

#include "uvd_types.h"
#include <set>

/*
*/
class UVDPlugin
{
public:
	UVDPlugin();
	~UVDPlugin();
	
public:
	std::string m_name;
};

class UVDPluginEngine
{
public:
	//Just because a plugin is registered, does not mean it is active
	UVDPluginEngine();
	~UVDPluginEngine();
	
	//Load plugin at given path
	//needs an exported symbol called UVDPluginMain
	uv_err_t load(const std::string &path);
	
public:
	//We might want to map this from something like the plugin name
	std::set<UVDPlugin *> m_plugins;
};

#endif

