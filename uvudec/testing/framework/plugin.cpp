/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "testing/framework/plugin.h"
#include "uvd/core/uvd.h"

void UVDTestingPluginFixture::setUp(void)
{
	UVDTestingCommonFixture::setUp();
	m_plugin = NULL;
	m_pluginEngine = NULL;
}

void UVDTestingPluginFixture::init(void)
{
	//Make sure we try to load this plugin regardless of config
	CPPUNIT_ASSERT(!m_pluginName.empty());
	appendArgument(std::string("--plugin=") + m_pluginName);
	CPPUNIT_ASSERT(generalInit() == UV_ERR_OK);
	m_pluginEngine = &m_uvd->m_config->m_plugin.m_pluginEngine;
	//Assert plugin loaded
	CPPUNIT_ASSERT(m_pluginEngine->m_loadedPlugins.find(m_pluginName) != m_pluginEngine->m_loadedPlugins.end());
	m_plugin = m_pluginEngine->m_loadedPlugins[m_pluginName];
}

void UVDTestingPluginFixture::deinit(void)
{
	//We don't own m_plugin, don't delete it
	UVDTestingCommonFixture::deinit();
}

