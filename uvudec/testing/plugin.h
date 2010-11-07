/*
UVNet Universal Decompiler (uvudec)
For test fixtures that need FLIRT, but not UVD
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_TESTING_PLUGIN_H
#define UVD_TESTING_PLUGIN_H

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "testing/common_fixture.h"
#include "uvd/flirt/flirt.h"

class UVDTestingPluginFixture : public UVDTestingCommonFixture
{
public:
	void setUp(void);

protected:
	void init(void);
	void deinit(void);

protected:
	UVDPlugin *m_plugin;
	std::string m_pluginName;
	//Takes a lot of deref otherwise
	UVDPluginEngine *m_pluginEngine;
};

#endif

