/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "plugin/engine.h"
#include "plugin/plugin.h"
#include <dlfcn.h>

UVDPluginEngine::UVDPluginEngine()
{
	m_uvd = NULL;
}

UVDPluginEngine::~UVDPluginEngine()
{
}
	
uv_err_t UVDPluginEngine::init(UVD *uvd)
{
	m_uvd = uvd;
	return UV_ERR_OK;
}

uv_err_t UVDPluginEngine::staticInit()
{
	/*
	FIXME
	Due to python issues, the static build has been dropped for now
	If we want a static build, use a python build script to generate this function for static builds
	Maybe we can use KConfig ;)
	*/
	
	return UV_ERR_OK;
}

uv_err_t UVDPluginEngine::deinit()
{
	return UV_ERR_OK;
}

uv_err_t UVDPluginEngine::loadByName(const std::string &name)
{
	/*
	We could also scan over plugin dirs and see if any match
	But, current strategy seems to preload them all, so point is mute
	*/
	
	//We could prob link some Boost in and make this easier
	std::string path;
	
	path = std::string("lib") + name + ".so";
	
	uv_assert_err_ret(loadByPath(path));
	return UV_ERR_OK;
}

uv_err_t UVDPluginEngine::loadByPath(const std::string &path)
{
	UVDPlugin *plugin = NULL;
	void *library = NULL;
	UVDPlugin::PluginMain pluginMain = NULL;
	const char *lastError = NULL;
	uv_err_t rcTemp = UV_ERR_GENERAL;
	std::string name;
		
	//Clear errors
	dlerror();
	library = dlopen(path.c_str(), RTLD_LAZY);
	lastError = dlerror();
	if( !library || lastError )
	{
		//Maybe should be a warning?
		//is there any reason why we'd lazily try to load a plugin only if it exists?
		if( !lastError )
		{
			lastError = "<UNKNOWN>";
		}
		printf_error("%s: load library failed: %s\n", path.c_str(), lastError);
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	

	pluginMain = (UVDPlugin::PluginMain)dlsym(library, UVD_PLUGIN_MAIN_SYMBOL_STRING);
	lastError = dlerror();
	if( !pluginMain || lastError )
	{
		if( !lastError )
		{
			lastError = "<UNKNOWN>";
		}
		printf_error("plugin %s: failed to load main: %s\n", path.c_str(), lastError);
		dlclose(library);
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	
	//	typedef PluginMain uv_err_t (*)(UVD *uvd, UVDPlugin **out);
	rcTemp = UV_DEBUG(pluginMain(m_uvd, &plugin));
	if( UV_FAILED(rcTemp) )
	{
		printf_error("plugin %s: main failed\n", path.c_str());
		dlclose(library);
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	if( !plugin )
	{
		printf_error("plugin %s: didn't return a plugin object\n", path.c_str());
		dlclose(library);
		return UV_DEBUG(UV_ERR_GENERAL);
	}

	rcTemp = UV_DEBUG(plugin->getName(name));
	if( UV_FAILED(rcTemp) )
	{
		dlclose(library);
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	
	if( name.empty() )
	{
		/*
		we could do
		name = path;
		but it might cause issues later
		refusing to load makes people stop being lazy
		*/
		printf_error("plugin %s: didn't provide a name\n", path.c_str());
		dlclose(library);
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	
	plugin->m_hLibrary = library;

	return UV_ERR_OK;
}

