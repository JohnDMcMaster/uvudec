/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "testing/plugin.h"

void UVDTestingPluginFixture::tearDown(void)
{
	//We don't own m_plugin, don't delete it
}

void UVDTestingPluginFixture::init(void)
{
	CPPUNIT_ASSERT(configInit() == UV_ERR_OK);
	m_plugin = NULL;
	//Assert plugin loaded
}

void UVDTestingPluginFixture::deinit(void)
{
	UVDTestingCommonFixture::deinit();
}

