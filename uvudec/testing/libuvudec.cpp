/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "testing/libuvudec.h"
#include "uvd/core/uvd.h"
#include <string.h>

CPPUNIT_TEST_SUITE_REGISTRATION(UVDLibuvudecUnitTest);

void UVDLibuvudecUnitTest::versionTest(void)
{
	CPPUNIT_ASSERT(strcmp(UVUDEC_VER_STRING, UVDGetVersion()) == 0);
}

void UVDLibuvudecUnitTest::initDeinitTest(void)
{
	m_args.clear();	
	CPPUNIT_ASSERT_EQUAL(UV_ERR_OK, configInit());
	deinit();
}


