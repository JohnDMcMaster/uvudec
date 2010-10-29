/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

/*
FIXME:
This is probably the file most needing cleanup in the project
Do something instead like create a list of optional and required members, with pointers if necessary for easy setup
*/

#include "uvd/core/uvd.h"
#include "uvd/flirt/flirt.h"
#include "uvd/core/runtime.h"
#include "uvd/event/engine.h"
#include "uvd/object/object.h"
#include "uvd/util/benchmark.h"

uv_err_t UVDInit()
{
	//Create the g_config object so we can start to take values from it
	uv_assert_err_ret(UVDInitConfigEarly());
	//Setup signal handling and basic logging
	//Also registers type prefixes and the associated debug args
	uv_assert_err_ret(UVDDebugInit());
	//Registers libuvudec args
	//Seems we should move this to arg parsing
	uv_assert_err_ret(UVDInitConfig());
	printf_debug_level(UVD_DEBUG_PASSES, "UVDInit(): done\n");
	return UV_ERR_OK;
}

uv_err_t UVDDeinit()
{
	/*
	Removed this to move away from the g_uvd instance
	We should work towards having multiple uvd objects
	if( g_uvd )
	{
		delete g_uvd;
	}
	*/
	g_uvd = NULL;

	//Hmm this one is more tricky
	//This won't get deleted by prev if it was global instance
	if( g_config )
	{
		delete g_config;
		g_config = NULL;
	}

	uv_assert_err_ret(UVDDebugDeinit());

	return UV_ERR_OK;
}

/*
file: data file to target
architecture: hint about what we are trying to disassemble
*/
uv_err_t UVD::init(const std::string &file, const UVDRuntimeHints &hints)
{
	uv_err_t rcTemp = UV_ERR_GENERAL;
	UVDData *data = NULL;
	
	uv_assert_err_ret(initEarly());
		
	rcTemp = UVDDataFile::getUVDDataFile(&data, file);
	if( UV_FAILED(rcTemp) || !data )
	{
		printf_error("could not open file: %s\n", file.c_str());
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	return init(data, hints);
}

uv_err_t UVD::init(UVDData *data, const UVDRuntimeHints &hints)
{
	UVDObject *object = NULL;
	UVDArchitecture *architecture = NULL;
	
	uv_assert_err_ret(initEarly());

	//Object wraps data and architectures, so it seems reasonable to instantiate first
	uv_assert_err_ret(initObject(data, hints, &object));
	uv_assert_err_ret(initArchitecture(object, hints, &architecture));
	return UV_DEBUG(init(object, architecture));	
}

uv_err_t UVD::initObject(UVDData *data, const UVDRuntimeHints &hints, UVDObject **out)
{
	//FIXME: replace this with a config based selection
	//Must be able to feed into plugins
	//This should be a switch instead
	//m_architecture->m_architecture = architecture;
	UVDObject *object = NULL;
	
	//Iterate over all plugins until one accepts our input
	uv_assert_ret(m_pluginEngine);
	for( std::map<std::string, UVDPlugin *>::iterator iter = m_pluginEngine->m_loadedPlugins.begin();
		iter != m_pluginEngine->m_loadedPlugins.end(); ++iter )
	{
		UVDPlugin *plugin = (*iter).second;
		uv_err_t rcTemp = UV_ERR_GENERAL;
		
		uv_assert_ret(plugin);
		rcTemp = plugin->getObject(data, hints, &object);
		if( rcTemp == UV_ERR_NOTSUPPORTED )
		{
			continue;
		}
		else if( UV_FAILED(rcTemp) )
		{
			printf_error("plugin %s failed to load object\n", (*iter).first.c_str());
			continue;
		}
		else if( !object )
		{
			printf_error("plugin %s claimed successed but didn't set object\n", (*iter).first.c_str());
			continue;
		}
		else
		{
			break;
		}
	}
	
	if( !object )
	{
		printf_error("could not find a suitable object module\n");
		return UV_ERR_GENERAL;
	}
	
	uv_assert_err_ret(object->init(data));
	
	uv_assert_ret(out);
	*out = object;

	return UV_ERR_OK;
}

uv_err_t UVD::initArchitecture(UVDObject *object, const UVDRuntimeHints &hints, UVDArchitecture **out)
{
	//FIXME: replace this with a config based selection
	//Must be able to feed into plugins
	//This should be a switch instead
	//m_architecture->m_architecture = architecture;
	UVDArchitecture *architecture = NULL;
	
	//Iterate over all plugins until one accepts our input
	for( std::map<std::string, UVDPlugin *>::iterator iter = m_pluginEngine->m_loadedPlugins.begin();
		iter != m_pluginEngine->m_loadedPlugins.end(); ++iter )
	{
		UVDPlugin *plugin = (*iter).second;
		uv_err_t rcTemp = UV_ERR_GENERAL;
		
		uv_assert_ret(plugin);
		rcTemp = plugin->getArchitecture(object, hints, &architecture);
		if( rcTemp == UV_ERR_NOTSUPPORTED )
		{
			continue;
		}
		else if( UV_FAILED(rcTemp) )
		{
			printf_error("plugin %s failed to load architecture\n", (*iter).first.c_str());
			continue;
		}
		else if( !architecture )
		{
			printf_error("plugin %s claimed successed but didn't set architecture\n", (*iter).first.c_str());
			continue;
		}
		else
		{
			break;
		}
	}
	
	if( !architecture )
	{
		printf_error("could not find a suitable architecture module\n");
		return UV_ERR_GENERAL;
	}
	
	uv_assert_err_ret(architecture->init());

	uv_assert_ret(out);
	*out = architecture;

	return UV_ERR_OK;
}

uv_err_t UVD::initEarly()
{
	uv_assert_ret(m_config);
	m_pluginEngine = &m_config->m_plugin.m_pluginEngine;
	m_pluginEngine->m_uvd = this;
	
	return UV_ERR_OK;
}

//int init_count = 0;
uv_err_t UVD::init(UVDObject *object, UVDArchitecture *architecture)
{
	if( !m_config->m_argv )
	{
		printf_warn("initializing UVD without parsing main\n");
	}
	uv_assert_err_ret(initEarly());

	//We might want to make this more dynamic just in case
	m_runtime = new UVDRuntime();
	uv_assert_ret(m_runtime);
	uv_assert_err_ret(m_runtime->init(object, architecture));

	//printf("plugins to load: %d\n", m_config->m_plugin.m_toLoad.size());
	for( std::vector<std::string>::iterator iter = m_config->m_plugin.m_toLoad.begin();
			iter != m_config->m_plugin.m_toLoad.end(); ++iter )
	{
		std::string &pluginName = *iter;
		
		uv_assert_err_ret(m_pluginEngine->initPlugin(pluginName));
	}
	//printf("loaded plugins: %d\n", m_pluginEngine->m_loadedPlugins.size());

	printf_debug_level(UVD_DEBUG_PASSES, "UVD::init(): initializing engine...\n");
	UVDBenchmark engineInitBenchmark;
	engineInitBenchmark.start();

	uv_assert_ret(m_config);
		
	m_config->m_verbose = m_config->m_verbose_init;

	/*
	m_CPU = new UVDCPU();
	uv_assert_ret(m_CPU);
	uv_assert_err_ret(m_CPU->init());
	*/

	m_analyzer = new UVDAnalyzer();
	uv_assert_ret(m_analyzer);
	m_analyzer->m_uvd = this;
	uv_assert_err_ret(m_analyzer->init());
	//Default to our global config, which should have already been initialized since its program dependent
	m_config = g_config;
	uv_assert_ret(m_config);
	m_format = new UVDFormat();
	uv_assert_ret(m_format);
	m_eventEngine = new UVDEventEngine();
	uv_assert_ret(m_eventEngine);
	uv_assert_err_ret(m_eventEngine->init());
	m_flirt = new UVDFLIRT();
	uv_assert_ret(m_flirt);
	m_flirt->m_uvd = this;
	uv_assert_err_ret(m_flirt->init());
	/*
	Read file
	This is raw dat, NOT null terminated string
	*/

	uv_assert_err_ret(m_config->m_plugin.m_pluginEngine.onUVDInit());

	printFormatting();
	printf_debug("UVD: init OK!\n\n\n");

	m_config->m_verbose = m_config->m_verbose_processing;
	
	engineInitBenchmark.stop();
	printf_debug_level(UVD_DEBUG_PASSES, "engine init time: %s\n", engineInitBenchmark.toString().c_str());

	return UV_ERR_OK;
}

uv_err_t UVD::initPlugins()
{
	//Load plugins as specified in config
	for( std::vector<std::string>::iterator iter = m_config->m_plugin.m_toLoad.begin();
			iter != m_config->m_plugin.m_toLoad.end(); ++iter )
	{
		const std::string &curPluginName = *iter;
		
		uv_assert_err_ret(m_config->m_plugin.m_pluginEngine.initPlugin(curPluginName));
	}
	
	return UV_ERR_OK;
}


