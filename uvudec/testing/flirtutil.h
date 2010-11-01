/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/
/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_TESTING_FLIRTUTIL_H
#define UVD_TESTING_FLIRTUTIL_H

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "testing/flirt.h"

class UVDFlirtutilFixture : public UVDTestingFLIRTFixture
{
	CPPUNIT_TEST_SUITE(UVDFlirtutilFixture);
	CPPUNIT_TEST(testingMainDumpTest);
	CPPUNIT_TEST_SUITE_END();

protected:
	void testingMainDumpTest();

	/*
	Utility functions
	*/
	//
	void verifyDump(const std::string &filePrefix);
	void verifyDump(const std::string &sigFileName, const std::string &expectedDumpFileName);
};

#endif

