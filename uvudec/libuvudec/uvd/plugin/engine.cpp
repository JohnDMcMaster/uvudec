/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/plugin/engine.h"
#include "uvd/plugin/plugin.h"
#include "uvd/core/uvd.h"
#include "uvd/init/config.h"
#include <boost/filesystem.hpp>
#include <dlfcn.h>

UVDPluginEngine::UVDPluginEngine()
{
	m_uvd = NULL;
}

UVDPluginEngine::~UVDPluginEngine()
{
}
	
uv_err_t UVDPluginEngine::init(UVDConfig *config)
{
	uv_assert_ret(config);
	uv_assert_ret(config == g_config);
	/*
	Call all plugin mains
	*/
	/*
	for dir in plugin dirs
		find all .so files with plugin main symbol
	*/
	for( std::vector<std::string>::iterator iter = config->m_plugin.m_dirs.begin();
			iter != config->m_plugin.m_dirs.end(); ++iter )
	{
		const std::string &pluginDir = *iter;
		
		for( boost::filesystem::directory_iterator iter(pluginDir);
			iter != boost::filesystem::directory_iterator(); ++iter )
		{
			//Not necessarily canonical
			std::string path;
			
			if( is_directory(iter->status()) )
			{
				continue;				
			}
			path = pluginDir + "/" + iter->path().filename();
			
			//Try loading it, ignoring errors since it might just be a text file or something
			if( UV_FAILED(loadByPath(path, false)) )
			{
				//printf_warn("failed to load possible plugin: %s\n", path.c_str());
			}
		}		
	}
	
	/*
	And initialize those plugins that should be initially loaded
	*/
	for( std::vector<std::string>::iterator iter = config->m_plugin.m_toLoad.begin();
			iter != config->m_plugin.m_toLoad.end(); ++iter )
	{
		const std::string &toInit = *iter;
		
		uv_assert_err_ret(initPlugin(toInit));		
	}

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

uv_err_t UVDPluginEngine::loadByPath(const std::string &path, bool reportErrors)
{
	UVDPlugin *plugin = NULL;
	void *library = NULL;
	UVDPlugin::PluginMain pluginMain = NULL;
	const char *lastError = NULL;
	uv_err_t rcTemp = UV_ERR_GENERAL;
	std::string name;

reportErrors = true;

	//Clear errors
	dlerror();
	library = dlopen(path.c_str(), RTLD_LAZY);
	lastError = dlerror();
	if( !library || lastError )
	{
		if( reportErrors )
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
		else
		{
			return UV_ERR_GENERAL;
		}
	}

	/*
	Prefer mangled symbol since its more type safe
	*/
	pluginMain = (UVDPlugin::PluginMain)dlsym(library, UVD_PLUGIN_MAIN_MANGLED_SYMBOL_STRING);
	lastError = dlerror();
	if( !pluginMain || lastError )
	{
		//But settle for the extern'd symbol if they do that for w/e reason
		pluginMain = (UVDPlugin::PluginMain)dlsym(library, UVD_PLUGIN_MAIN_SYMBOL_STRING);
		lastError = dlerror();
		if( !pluginMain || lastError )
		{
			if( reportErrors )
			{
				if( !lastError )
				{
					lastError = "<UNKNOWN>";
				}
				printf_error("plugin %s: failed to load main: %s\n", path.c_str(), lastError);
				dlclose(library);
				return UV_DEBUG(UV_ERR_GENERAL);
			}
			else
			{
				dlclose(library);
				return UV_ERR_GENERAL;
			}
		}
	}
	
	//	typedef PluginMain uv_err_t (*)(UVD *uvd, UVDPlugin **out);
	UVDConfig *config = NULL;
	if( m_uvd )
	{
		config = m_uvd->m_config;
	}
	else
	{
		uv_assert_ret(g_config);
		config = g_config;
	}
	rcTemp = UV_DEBUG(pluginMain(config, &plugin));
	if( UV_FAILED(rcTemp) )
	{
		if( reportErrors )
		{
			printf_error("plugin %s: main failed\n", path.c_str());
			dlclose(library);
			return UV_DEBUG(UV_ERR_GENERAL);
		}
		else
		{
			dlclose(library);
			return UV_ERR_GENERAL;
		}
	}
	if( !plugin )
	{
		if( reportErrors )
		{
			printf_error("plugin %s: didn't return a plugin object\n", path.c_str());
			dlclose(library);
			return UV_DEBUG(UV_ERR_GENERAL);
		}
		else
		{
			dlclose(library);
			return UV_ERR_GENERAL;
		}
	}

	rcTemp = UV_DEBUG(plugin->getName(name));
	if( UV_FAILED(rcTemp) )
	{
		if( reportErrors )
		{
			dlclose(library);
			return UV_DEBUG(UV_ERR_GENERAL);
		}
		else
		{
			dlclose(library);
			return UV_ERR_GENERAL;
		}
	}
	
	if( name.empty() )
	{
		/*
		we could do
		name = path;
		but it might cause issues later
		refusing to load makes people stop being lazy
		*/
		if( reportErrors )
		{
			printf_error("plugin %s: didn't provide a name\n", path.c_str());
			dlclose(library);
			return UV_DEBUG(UV_ERR_GENERAL);
		}
		else
		{
			dlclose(library);
			return UV_ERR_GENERAL;
		}
	}
	
	plugin->m_hLibrary = library;
	m_plugins[name] = plugin;
	

	return UV_ERR_OK;
}

uv_err_t UVDPluginEngine::initPlugin(const std::string &name)
{
	std::map<std::string, UVDPlugin *>::iterator iter = m_plugins.find(name);
	UVDPlugin *plugin = NULL;

	if( iter == m_plugins.end() )
	{
		printf_error("no plugin loaded named %s\n", name.c_str());
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	plugin = (*iter).second;
	uv_assert_ret(plugin);
	uv_assert_err_ret(plugin->init(g_config));
	m_loadedPlugins[name] = plugin;

	return UV_ERR_OK;
}

uv_err_t UVDPluginEngine::deinitPlugin(const std::string &name)
{
	std::map<std::string, UVDPlugin *>::iterator iter = m_loadedPlugins.find(name);
	UVDPlugin *plugin = NULL;

	uv_assert_ret(iter != m_loadedPlugins.end());
	plugin = (*iter).second;
	uv_assert_ret(plugin);
	if( UV_FAILED(plugin->deinit(g_config)) )
	{
		printf_error("failed to cleanly unload plugin %s\n", name.c_str());
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	
	m_loadedPlugins.erase(iter);
	
	return UV_ERR_OK;
}

uv_err_t UVDPluginEngine::onUVDInit()
{
	uv_assert_ret(m_uvd);

	for( std::map<std::string, UVDPlugin *>::iterator iter = m_loadedPlugins.begin();
			iter != m_loadedPlugins.end(); ++iter )
	{
		UVDPlugin *plugin = (*iter).second;
		
		uv_assert_ret(plugin);
		//FIXME: add config for potentially nonfatal errors
		uv_assert_err_ret(plugin->onUVDInit(m_uvd));
	}
	return UV_ERR_OK;
}

uv_err_t UVDPluginEngine::onUVDDeinit()
{
	uv_assert_ret(m_uvd);

	for( std::map<std::string, UVDPlugin *>::iterator iter = m_loadedPlugins.begin();
			iter != m_loadedPlugins.end(); ++iter )
	{
		UVDPlugin *plugin = (*iter).second;
		
		uv_assert_ret(plugin);
		//FIXME: add config for potentially nonfatal errors
		uv_assert_err_ret(plugin->onUVDDeinit(m_uvd));
	}
	return UV_ERR_OK;
}

