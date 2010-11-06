/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_TESTING_OBJECT_H
#define UVD_TESTING_OBJECT_H

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "testing/plugin.h"
#include "uvd/flirt/flirt.h"

class UVDTestingObjectFixture : public UVDTestingPluginFixture
{
protected:
	void init();

protected:
};

#endif

