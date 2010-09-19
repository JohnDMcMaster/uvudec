/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef PLUGIN_PLUGIN_H
#define PLUGIN_PLUGIN_H

#include "uvd_types.h"
#include "uvd_version.h"
#include <set>
#include <map>

//Entry point for a loaded plugin to register itself
#define UVD_PLUGIN_MAIN_SYMBOL					UVDPluginMain
#define UVD_PLUGIN_MAIN_SYMBOL_STRING			"UVDPluginMain"

/*
Unless otherwise specified, no functions need to be called on the parent UVDPlugin class if overriden
Don't throw exceptions, I won't catch them
If you need to do plugin specific initialization even before the plugin is activated
	Should be rare
	Do it in PluginMain()
*/
class UVDVersion;
class UVD;
class UVDPlugin
{
public:
	//<name, version range>
	typedef std::pair<std::string, UVDVersionRange> PluginDependency;
	typedef std::set<PluginDependency> PluginDependencies;
	typedef uv_err_t (*PluginMain)(UVD *uvd, UVDPlugin **out);

public:
	UVDPlugin();
	virtual ~UVDPlugin();
	
	//Make the plugin active
	virtual uv_err_t init();
	//Deactivate w/e the plugin does
	virtual uv_err_t deinit();
	//Top level wrappers so subclasses don't have to do ugly super stuff
	uv_err_t initEntry();
	uv_err_t deinitEntry();

	/*
	Note the following may be called before init()
	*/
	//Terse name to identify the plugin
	//No spaces, use [a-z][A-Z][0-9][-]
	virtual uv_err_t getName(std::string &out) = 0;
	//Human readable description
	virtual uv_err_t getDescription(std::string &out) = 0;	
	//Should be obvious enough
	virtual uv_err_t getVersion(UVDVersion &out) = 0;
	//If we require another plugin loaded, get it
	//Default is not dependencies
	virtual uv_err_t getDependencies(PluginDependencies &out);

public:
	//returned by dlopen()
	void *m_hLibrary;
};

#endif

