/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_TESTING_BIN2OBJ_H
#define UVD_TESTING_BIN2OBJ_H

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "testing/common_fixture.h"

class UVDBin2objTestFixture : public UVDTestingCommonFixture
{
	CPPUNIT_TEST_SUITE(UVDBin2objTestFixture);
	CPPUNIT_TEST(defaultDecompileFileTest);
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
	void disassembleRangeTestDeliminators(void);
	void disassembleRangeTestDefaultEquivilence(void);
	void disassembleRangeTestComplex(void);
	/*
	Actually calls uvudec's uvmain using the hooks
	Does a basic test where as most of hte thorough test test libuvudec rather than what the uvudec exe can do
	*/
	void uvudecBasicRun(void);
};

#endif

