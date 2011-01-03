/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/string/engine.h"
#include "uvd/plugin/engine.h"
#include "uvd/core/uvd.h"
#include "uvd/util/benchmark.h"

/*
UVDStringEngine
*/

static uv_err_t onPluginActivated(UVDPlugin *plugin, void *user)
{
	UVDStringEngine *stringAnalyzers = (UVDStringEngine *)user;
	
	return UV_DEBUG(stringAnalyzers->pluginActivatedCallback(plugin));
}

UVDStringEngine::UVDStringEngine()
{
	m_uvd = NULL;
}

uv_err_t UVDStringEngine::init(UVD *uvd)
{
	UVDPluginEngine *pluginEngine = NULL;
	
	m_uvd = uvd;
	pluginEngine = &m_uvd->m_config->m_plugin.m_pluginEngine;

	uv_assert_err_ret(pluginEngine->registerPluginActivatedCallback(onPluginActivated, this));
	
	return UV_ERR_OK;
}

uv_err_t UVDStringEngine::pluginActivatedCallback(UVDPlugin *plugin)
{
	return UV_DEBUG(tryPlugin(plugin));
}

/*
uv_err_t UVDStringEngine::analyze()
{
	for( std::set<UVDStringsAnalyzer *>::iterator iter = m_analyzers.begin(); iter != m_analyzers.end(); ++iter )
	{
		UVDStringsAnalyzer *analyzer = *iter;
		
		uv_assert_ret(analyzer);
		uv_assert_err_ret(analyzer->analyze());
	}
	
	return UV_ERR_OK;
}
*/

uv_err_t UVDStringEngine::findAnalyzers()
{
	UVDPluginEngine *pluginEngine = &m_uvd->m_config->m_plugin.m_pluginEngine;

	m_analyzers.clear();	
	for( std::map<std::string, UVDPlugin *>::iterator iter = pluginEngine->m_loadedPlugins.begin();
			iter != pluginEngine->m_loadedPlugins.end(); ++iter )
	{
		uv_assert_err_ret(tryPlugin((*iter).second));
	}
	
	return UV_ERR_OK;
}

uv_err_t UVDStringEngine::tryPlugin(UVDPlugin *plugin)
{
	UVDStringsAnalyzer *analyzer = NULL;
	uv_err_t rc = UV_ERR_GENERAL;
	
	uv_assert_ret(plugin);
	rc = plugin->getStringsAnalyzer(&analyzer);
	if( rc == UV_ERR_NOTSUPPORTED )
	{
		return UV_ERR_OK;
	}
	uv_assert_err_ret(rc);
	uv_assert_ret(analyzer);
	m_analyzers.insert(analyzer);

	return UV_ERR_OK;
}

uv_err_t UVDStringEngine::getAllStrings(std::vector<UVDString> &out)
{
	printf_debug_level(UVD_DEBUG_PASSES, "UVDStringsAnalyzerImpl: getting/analyzering strings...\n");
	UVDBenchmark stringAnalysisBenchmark;
	stringAnalysisBenchmark.start();
	
	out.clear();
	for( std::set<UVDStringsAnalyzer *>::iterator iter = m_analyzers.begin();
			iter != m_analyzers.end(); ++iter )
	{
		UVDStringsAnalyzer *analyzer = *iter;
		
		uv_assert_err_ret(analyzer->appendAllStrings(out));
	}

	stringAnalysisBenchmark.stop();
	printf_debug_level(UVD_DEBUG_PASSES, "string analysis time: %s\n", stringAnalysisBenchmark.toString().c_str());
	
	return UV_ERR_OK;
}

