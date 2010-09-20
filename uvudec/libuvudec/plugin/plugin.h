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
#define UVD_PLUGIN_MAIN_MANGLED_SYMBOL			UVDPluginMain
#define UVD_PLUGIN_MAIN_MANGLED_SYMBOL_STRING	"UVDPluginMain"

/*
Unless otherwise specified, no functions need to be called on the parent UVDPlugin class if overriden
Don't throw exceptions, I won't catch them
If you need to do plugin specific initialization even before the plugin is activated
	Should be rare
	Do it in PluginMain()
*/
class UVDVersion;
class UVD;
class UVDConfig;
class UVDArchitecture;
class UVDData;
class UVDPlugin
{
public:
	//<name, version range>
	typedef std::pair<std::string, UVDVersionRange> PluginDependency;
	typedef std::set<PluginDependency> PluginDependencies;
	/*
	Since we want plugins to be able to process args, they are loaded very early
	They will have a uvd engine object set later as needed
	In theory, we could initialize multiple config/uvd engines this way
	but wouldn't count on it for some time
	most likely we'd just unload and reload the plugin
	*/
	typedef uv_err_t (*PluginMain)(UVDConfig *config, UVDPlugin **out);

public:
	UVDPlugin();
	virtual ~UVDPlugin();
	
	//Make the plugin active
	virtual uv_err_t init(UVD *uvd);
	//Deactivate w/e the plugin does
	virtual uv_err_t deinit();
	//Top level wrappers so subclasses don't have to do ugly super stuff

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
	/*
	If we deem appropriete, load architecture
	Returns UV_ERR_NOTSUPPORTED if we can't support the data format
		such as a plugin that doesn't support architectures
	Since many of the formats will be binary, we should specify the correct plugin instead of spamming
	since it will be difficult/impossible to tell what architecture it is simply from the binary
	Note that we could support multiple archs in the plugin, we just return the best one
	*/
	virtual uv_err_t getArchitecture(UVDData *data, const std::string &architecture, UVDArchitecture **out);

public:
	//returned by dlopen()
	void *m_hLibrary;
	//This will be set upon UVD engine initialization
	//Since plugins are loaded before even arguments are parsed, no UVD exists at that time
	UVD *m_uvd;
};

#endif

