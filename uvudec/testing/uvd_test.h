/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#ifndef UVD_TEST_H
#define UVD_TEST_H

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#define UVCPPUNIT_ASSERT(x)			CPPUNIT_ASSERT(UV_SUCCEEDED(x))

class UVDUnitTest : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(UVDUnitTest);
	CPPUNIT_TEST(versionTest);
	CPPUNIT_TEST(defaultDecompileFileTest);
	CPPUNIT_TEST(initDeinitTest);
	CPPUNIT_TEST(versionArgTest);
	CPPUNIT_TEST(helpArgTest);
	CPPUNIT_TEST(engineInitTest);
	CPPUNIT_TEST(analysisDirTest);
	CPPUNIT_TEST(disassembleTest);
	CPPUNIT_TEST_SUITE_END();

	public:
		void setUp(void);
		void tearDown(void);

	protected:
		/*
		Checks our exe compiled version aginst the library compiled version
		They should be equal
		*/
		void versionTest(void);
		/*
		Make sure that our default decompile file is accessible
		It is required for engine initialization
		*/
		void defaultDecompileFileTest(void);
		/*
		Early initialization
		Logging, argument parsing structures
		Should NOT initialize the actual decompiler engine
		*/
		void initDeinitTest(void);
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
		Generate analysis output such as object files
		*/
		void analysisDirTest(void);
		/*
		Disassemble the default binary
		*/
		void disassembleTest(void);

	private:
};

#endif
