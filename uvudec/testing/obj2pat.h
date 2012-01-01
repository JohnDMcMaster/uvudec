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

#ifndef UVD_TESTING_OBJ2PAT_H
#define UVD_TESTING_OBJ2PAT_H

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "testing/framework/common_fixture.h"

class UVDObj2patUnitTest : public UVDTestingCommonFixture
{
	CPPUNIT_TEST_SUITE(UVDObj2patUnitTest);
	CPPUNIT_TEST(cppTest);
	CPPUNIT_TEST(inlineTest);
	CPPUNIT_TEST(noNameTest);
	CPPUNIT_TEST(noNamesTest);
	CPPUNIT_TEST(recursiveTest);
	CPPUNIT_TEST(shortNamesTest);
	CPPUNIT_TEST(shortNamesRefTest);
	CPPUNIT_TEST(shortTest);
	CPPUNIT_TEST(testingMainTest);
	CPPUNIT_TEST(libmTest);
	CPPUNIT_TEST_SUITE_END();

protected:
	/*
	Simple C++ file
	*/
	void cppTest();
	/*
	Inline function
	*/
	void inlineTest();
	/*
	Single short name (which results in a "no name" condition)
	*/
	void noNameTest();
	/*
	More complex version of above where multiple are present
	Multiple present has different treatment
	*/
	void noNamesTest();
	/*
	Recursive function
	*/
	void recursiveTest();
	/*
	Varying length function names
	*/
	void shortNamesTest();
	/*
	Varying length function names referenced in a standard function
	*/
	void shortNamesRefTest();
	/*
	A very short function
	It should be excluded from the pat file
	*/
	void shortTest();
	/*
	A real object file
	*/
	void testingMainTest();
	/*
	A real library
	*/
	void libmTest();

	/*
	Utility functions
	*/
	//
	void verifyObj2Pat(const std::string &filePrefix);
	void verifyObj2Pat(const std::string &objectFileName, const std::string &expectedPatFileName);
	//void verifyObj2Pat(const std::string &file, const std::string &expectedPatFileContents, bool fixupPaths = false);
};

#endif

