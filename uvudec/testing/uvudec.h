/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_TESTING_H
#define UVD_TESTING_H

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "testing/common_fixture.h"

class UVDUvudecUnitTest : public UVDTestingCommonFixture
{
	CPPUNIT_TEST_SUITE(UVDUvudecUnitTest);
	CPPUNIT_TEST(defaultDecompileFileTest);
	CPPUNIT_TEST(versionArgTest);
	CPPUNIT_TEST(helpArgTest);
	CPPUNIT_TEST(engineInitTest);
	CPPUNIT_TEST(disassembleTest);
	CPPUNIT_TEST(disassembleRangeTestDeliminatorsTest);
	CPPUNIT_TEST(disassembleRangeTestDefaultEquivilenceTest);
	CPPUNIT_TEST(disassembleRangeTestComplexTest);
	CPPUNIT_TEST(uvudecBasicRunTest);
	CPPUNIT_TEST_SUITE_END();

protected:
	/*
	Make sure that our default decompile file is accessible
	It is required for engine initialization
	*/
	void defaultDecompileFileTest(void);
	/*
	Test the "--version" option
	*/
	void versionArgTest(void);
	/*
	Test the "--help" option
	*/
	void helpArgTest(void);
	/*
	Perform a full engine init, but don't do any analysis
	*/
	void engineInitTest(void);
	/*
	Disassemble the default binary
	*/
	void disassembleTest(void);
	/*
	Disassemble the default binary with inclusion/exclusion ranges
	*/
	void disassembleRangeTestDeliminatorsTest(void);
	void disassembleRangeTestDefaultEquivilenceTest(void);
	void disassembleRangeTestComplexTest(void);
	/*
	Actually calls uvudec's uvmain using the hooks
	Does a basic test where as most of hte thorough test test libuvudec rather than what the uvudec exe can do
	*/
	void uvudecBasicRunTest(void);
};

#endif

