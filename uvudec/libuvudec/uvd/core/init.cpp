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
	UVDConfig *config = NULL;
	//printf("UVDInit()\n");
	if( config == NULL )
	{
		config = new UVDConfig();
	//printf("created config: 0x%08X\n", (int)config);
		uv_assert_ret(config);
		if( g_config == NULL )
		{
			g_config = config;
		}
	}
	//Called before debugging initialized
	//Create the g_config object so we can start to take values from it
	uv_assert_err_ret(config->init());
	
	//Setup signal handling and basic logging
	//Also registers type prefixes and the associated debug args
	uv_assert_err_ret(UVDDebugInit());

	//Registers libuvudec args
	//Seems we should move this to arg parsing
	uv_assert_err_ret(config->initArgConfig());

	printf_debug_level(UVD_DEBUG_PASSES, "UVDInit(): done\n");
	return UV_ERR_OK;
}

uv_err_t UVDDeinit()
{
	//printf("UVDDeinit()\n"); fflush(stdout);
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
uv_err_t UVD::initFromFileName(const std::string &file, const UVDRuntimeHints &hints)
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
	uv_assert_err(initFromData(data, hints));
	return UV_ERR_OK;

error:
	delete data;
	return UV_DEBUG(UV_ERR_GENERAL);
}

uv_err_t UVD::initFromData(UVDData *data, const UVDRuntimeHints &hints)
{
	UVDObject *object = NULL;
	UVDArchitecture *architecture = NULL;
	
	uv_assert_err_ret(initEarly());

	//Object wraps data and architectures, so it seems reasonable to instantiate first
	uv_assert_err(initObject(data, hints, &object));
	uv_assert_err(initArchitecture(object, hints, &architecture));
	return UV_DEBUG(init(object, architecture));	

error:
	if( object )
	{
		object->m_data = NULL;
		delete object;
	}
	delete architecture;
	return UV_DEBUG(UV_ERR_GENERAL);
}

uv_err_t UVD::initObject(UVDData *data, const UVDRuntimeHints &hints, UVDObject **out)
{
	//FIXME: replace this with a config based selection
	//Must be able to feed into plugins
	//This should be a switch instead
	//m_architecture->m_architecture = architecture;
	UVDObject *object = NULL;
	std::vector<UVDPlugin *> best;
	uvd_priority_t bestPriority = UVD_MATCH_NONE;
	
	UVD_POKE(data);
	//Iterate over all plugins until one accepts our input
	uv_assert_ret(m_pluginEngine);
	for( std::map<std::string, UVDPlugin *>::iterator iter = m_pluginEngine->m_loadedPlugins.begin();
		iter != m_pluginEngine->m_loadedPlugins.end(); ++iter )
	{
		UVDPlugin *plugin = (*iter).second;
		uv_err_t rcTemp = UV_ERR_GENERAL;
		uvd_priority_t loadPriority = 0;
		
		uv_assert_ret(plugin);
		printf_plugin_debug("plugin %s trying canLoad object\n", (*iter).first.c_str());
		rcTemp = plugin->canLoadObject(data, hints,  &loadPriority);
		if( UV_FAILED(rcTemp) )
		{
			printf_plugin_debug("plugin %s failed to canLoad object\n", (*iter).first.c_str());
			continue;
		}

		if( loadPriority <= bestPriority )
		{
			printf_plugin_debug("plugin %s candidate at priority 0x%08X\n", (*iter).first.c_str(), loadPriority);
			if( loadPriority < bestPriority )
			{
				if( !best.empty() )
				{
					printf_plugin_debug("clearing %d plugins due to better priorty\n", best.size());
				}
				best.clear();
				bestPriority = loadPriority;
			}
			best.push_back(plugin);
		}
		else
		{
			printf_plugin_debug("plugin %s skipped due to worse priority (cur: %d, plugin: %d)\n",
					(*iter).first.c_str(), bestPriority, loadPriority);
		}
	}
	
	UVD_POKE(data);
	printf_plugin_debug("best priorty: 0x%08X, plugins: %d\n", bestPriority, best.size());
	if( bestPriority == UVD_MATCH_NONE )
	{
		printf_warn("could not find a suitable object loader\n");
		return UV_ERR_NOTFOUND;
	}
	uv_assert_ret(!best.empty());

	//Iterate over all plugins until one accepts our input
	uv_assert_ret(m_pluginEngine);
	for( std::vector<UVDPlugin *>::iterator iter = best.begin();
		iter != best.end(); ++iter )
	{
		UVDPlugin *plugin = *iter;
		uv_err_t rcTemp = UV_ERR_GENERAL;
		
		uv_assert_ret(plugin);
		rcTemp = plugin->loadObject(data, hints, &object);
		if( UV_FAILED(rcTemp) )
		{
			printf_error("plugin %s failed to load object\n", plugin->getName().c_str());
			continue;
		}
		else if( !object )
		{
			printf_error("plugin %s claimed successed but didn't set object\n", plugin->getName().c_str());
			continue;
		}
		else
		{
			printf_debug_level(UVD_DEBUG_PASSES, "loaded object from plugin %s\n", plugin->getName().c_str());
			break;
		}
	}
	
	if( !object )
	{
		printf_error("could not find a suitable object module\n");
		return UV_ERR_GENERAL;
	}
	
	//Its probably better to let the object take care of this
	//For example, if init fails, we can try another candidate
	//uv_assert_err_ret(object->init(data));
	
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
	std::vector<UVDPlugin *> best;
	uvd_priority_t bestPriority = UVD_MATCH_NONE;
	
	for( std::map<std::string, UVDPlugin *>::iterator iter = m_pluginEngine->m_loadedPlugins.begin();
		iter != m_pluginEngine->m_loadedPlugins.end(); ++iter )
	{
		UVDPlugin *plugin = (*iter).second;
		uv_err_t rcTemp = UV_ERR_GENERAL;
		uvd_priority_t loadPriority = 0;
		
		uv_assert_ret(plugin);
		rcTemp = plugin->canGetArchitecture(object, hints,  &loadPriority);
		if( UV_FAILED(rcTemp) )
		{
			printf_plugin_debug("plugin %s failed to canLoad object\n", (*iter).first.c_str());
			continue;
		}

		if( loadPriority <= bestPriority )
		{
			if( loadPriority < bestPriority )
			{
				best.clear();
				bestPriority = loadPriority;
			}
			best.push_back(plugin);
		}
	}
	
	printf_plugin_debug("best priorty: 0x%08X, plugins: %d\n", bestPriority, best.size());
	if( bestPriority == UVD_MATCH_NONE )
	{
		printf_warn("could not find a suitable architecture loader\n");
		return UV_ERR_NOTFOUND;
	}
	uv_assert_ret(!best.empty());

	//Iterate over all plugins until one accepts our input
	for( std::vector<UVDPlugin *>::iterator iter = best.begin();
		iter != best.end(); ++iter )
	{
		UVDPlugin *plugin = *iter;
		uv_err_t rcTemp = UV_ERR_GENERAL;
		
		uv_assert_ret(plugin);		
		rcTemp = plugin->getArchitecture(object, hints, &architecture);
		if( UV_FAILED(rcTemp) )
		{
			printf_error("plugin %s failed to load architecture\n", plugin->getName().c_str());
			continue;
		}
		else if( !architecture )
		{
			printf_error("plugin %s claimed successed but didn't set architecture\n", plugin->getName().c_str());
			continue;
		}
		else
		{
			printf_debug_level(UVD_DEBUG_PASSES, "loaded architecture from plugin %s\n", plugin->getName().c_str());
			break;
		}
	}
	
	if( !architecture )
	{
		printf_error("could not find a suitable architecture loader\n");
		return UV_ERR_NOTFOUND;
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

	//Since plugin initialization may depend on our configuration, this needs to be established even before onUVDInit() is called
	//Tried to pass in UVDConfig, but it gets too messy
	for( std::map<std::string, UVDPlugin *>::iterator iter = m_pluginEngine->m_loadedPlugins.begin();
		iter != m_pluginEngine->m_loadedPlugins.end(); ++iter )
	{
		UVDPlugin *plugin = (*iter).second;
		
		uv_assert_ret(plugin);
		plugin->m_uvd = this;
	}
	
	return UV_ERR_OK;
}

//int init_count = 0;
uv_err_t UVD::init(UVDObject *object, UVDArchitecture *architecture)
{
	UVDBenchmark engineInitBenchmark;

	if( !m_config->m_argv )
	{
		printf_warn("initializing UVD without parsing main\n");
	}
	uv_assert_err(initEarly());

	//We might want to make this more dynamic just in case
	m_runtime = new UVDRuntime();
	uv_assert(m_runtime);
	uv_assert_err(m_runtime->init(object, architecture));

	printf_debug_level(UVD_DEBUG_PASSES, "UVD::init(): initializing engine...\n");
	engineInitBenchmark.start();

	uv_assert(m_config);
		
	m_config->m_verbose = m_config->m_verbose_init;

	/*
	m_CPU = new UVDCPU();
	uv_assert(m_CPU);
	uv_assert_err(m_CPU->init());
	*/

	m_analyzer = new UVDAnalyzer();
	uv_assert(m_analyzer);
	m_analyzer->m_uvd = this;
	uv_assert_err(m_analyzer->init());
	m_format = new UVDFormat();
	uv_assert(m_format);
	m_eventEngine = new UVDEventEngine();
	uv_assert(m_eventEngine);
	uv_assert_err(m_eventEngine->init());
	m_flirt = new UVDFLIRT();
	uv_assert(m_flirt);
	m_flirt->m_uvd = this;
	uv_assert_err(m_flirt->init());
	/*
	Read file
	This is raw dat, NOT null terminated string
	*/

	uv_assert_err(m_config->m_plugin.m_pluginEngine.onUVDInit());

	printFormatting();
	printf_debug("UVD: init OK!\n\n\n");

	m_config->m_verbose = m_config->m_verbose_processing;
	
	engineInitBenchmark.stop();
	printf_debug_level(UVD_DEBUG_PASSES, "engine init time: %s\n", engineInitBenchmark.toString().c_str());

	return UV_ERR_OK;
	
error:
	//If we fail, we do not own its subobjects
	if( m_runtime )
	{
		m_runtime->m_object = NULL;
		m_runtime->m_architecture = NULL;
		delete m_runtime;
		m_runtime = NULL;
	}
	return UV_DEBUG(UV_ERR_GENERAL);
}

