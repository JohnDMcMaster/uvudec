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

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>
#include <sys/stat.h>
#include <vector>
#include <algorithm>
#include "event/engine.h"
#include "uvd_debug.h"
#include "uvd_error.h"
#include "uvd_log.h"
#include "uvd_util.h"
#include "uvd.h"
#include "uvd_address.h"
#include "uvd_analysis.h"
#include "uvd_benchmark.h"
#include "uvd_config.h"
#include "uvd_config_symbol.h"
#include "uvd_data.h"
#include "uvd_format.h"
#include "uvd_instruction.h"
#include "uvd_register.h"
#include "uvd_types.h"
#include "uvd_cpu_vector.h"
#include "core/architecture.h"

uv_err_t UVDInit()
{
	//Initially we log to console until a "real" log is setup which may be an actual file
	//we don't know actual file because we haven't parsed args yet
	uv_assert_err_ret(uv_log_init("/dev/stdout"));
	uv_assert_err_ret(UVDInitConfigEarly());
	uv_assert_err_ret(UVDDebugInit());
	uv_assert_err_ret(UVDInitConfig());
	printf_debug_level(UVD_DEBUG_PASSES, "UVDInit(): done\n");
	return UV_ERR_OK;
}

uv_err_t UVDDeinit()
{
	if( g_uvd )
	{
		delete g_uvd;
		g_uvd = NULL;
	}

	//This won't get deleted by prev if it was global instance
	if( g_config )
	{
		delete g_config;
		g_config = NULL;
	}

	uv_assert_err_ret(UVDDebugDeinit());
	uv_assert_err_ret(uv_log_deinit());

	return UV_ERR_OK;
}

/*
file: data file to target
architecture: hint about what we are trying to disassemble
*/
uv_err_t UVD::init(const std::string &file, const std::string &architecture)
{
	uv_err_t rcTemp = UV_ERR_GENERAL;
	UVDData *data;
	
	UV_ENTER();
		
	rcTemp = UVDDataFile::getUVDDataFile(&data, file);
	if( UV_FAILED(rcTemp) || !data )
	{
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	return init(data, architecture);
}

//int init_count = 0;
uv_err_t UVD::init(UVDData *data, const std::string &architecture)
{
	uv_err_t rc = UV_ERR_GENERAL;
	m_data = data;
	
	uv_assert_ret(m_config);
	m_pluginEngine = &m_config->m_plugin.m_pluginEngine;
	m_pluginEngine->m_uvd = this;

	//printf("plugins to load: %d\n", m_config->m_plugin.m_toLoad.size());
	for( std::vector<std::string>::iterator iter = m_config->m_plugin.m_toLoad.begin();
			iter != m_config->m_plugin.m_toLoad.end(); ++iter )
	{
		std::string &pluginName = *iter;
		
		uv_assert_err_ret(m_pluginEngine->initPlugin(pluginName));
	}
	//printf("loaded plugins: %d\n", m_pluginEngine->m_loadedPlugins.size());

	uv_assert_err_ret(initArchitecture(architecture));
	
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
	uv_assert(m_analyzer);
	m_analyzer->m_uvd = this;
	uv_assert_err_ret(m_analyzer->init());
	//Default to our global config, which should have already been initialized since its program dependent
	m_config = g_config;
	uv_assert_ret(m_config);
	m_format = new UVDFormat();
	uv_assert(m_format);
	m_eventEngine = new UVDEventEngine();
	uv_assert(m_eventEngine);
	uv_assert_err_ret(m_eventEngine->init());

	/*
	Read file
	This is raw dat, NOT null terminated string
	*/

	printFormatting();
	printf_debug("UVD: init OK!\n\n\n");

	m_config->m_verbose = m_config->m_verbose_processing;
	
	engineInitBenchmark.stop();
	printf_debug_level(UVD_DEBUG_PASSES, "engine init time: %s\n", engineInitBenchmark.toString().c_str());

	rc = UV_ERR_OK;
error:
	return UV_DEBUG(rc);
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

uv_err_t UVD::initArchitecture(const std::string &architecture)
{
	//FIXME: replace this with a config based selection
	//Must be able to feed into plugins
	//This should be a switch instead
	//m_architecture->m_architecture = architecture;
	
	//Iterate over all plugins until one accepts our input
	for( std::map<std::string, UVDPlugin *>::iterator iter = m_pluginEngine->m_loadedPlugins.begin();
		iter != m_pluginEngine->m_loadedPlugins.end(); ++iter )
	{
		UVDPlugin *plugin = (*iter).second;
		uv_err_t rcTemp = UV_ERR_GENERAL;
		
		uv_assert_ret(plugin);
		rcTemp = plugin->getArchitecture(m_data, architecture, &m_architecture);
		if( rcTemp == UV_ERR_NOTSUPPORTED )
		{
			continue;
		}
		else if( UV_FAILED(rcTemp) )
		{
			printf_error("plugin %s failed to load architecture\n", (*iter).first.c_str());
			continue;
		}
		else if( !m_architecture )
		{
			printf_error("plugin %s claimed successed but didn't set architecture\n", (*iter).first.c_str());
			continue;
		}
		else
		{
			break;
		}
	}
	
	if( !m_architecture )
	{
		printf_error("could not find a suitable architecture module\n");
		return UV_ERR_GENERAL;
	}
	
	uv_assert_err_ret(m_architecture->init());

	return UV_ERR_OK;
}

