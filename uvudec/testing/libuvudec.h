/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_TESTING_LIBUVUDEC_H
#define UVD_TESTING_LIBUVUDEC_H

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "testing/common_fixture.h"

class UVDLibuvudecUnitTest : public UVDTestingCommonFixture
{
	CPPUNIT_TEST_SUITE(UVDLibuvudecUnitTest);
	CPPUNIT_TEST(versionTest);
	CPPUNIT_TEST(initDeinitTest);
	CPPUNIT_TEST_SUITE_END();

protected:
	/*
	Checks our exe compiled version aginst the library compiled version
	They should be equal
	*/
	void versionTest(void);
	/*
	Early initialization
	Logging, argument parsing structures
	Should NOT initialize the actual decompiler engine
	*/
	void initDeinitTest(void);
};

#endif

