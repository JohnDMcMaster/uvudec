/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "testing/flirtutil.h"
#include "uvd/core/uvd.h"
#include "uvd/flirt/flirt.h"
#include <string.h>

CPPUNIT_TEST_SUITE_REGISTRATION(UVDFlirtutilFixture);

/*
void UVDFlirtutilFixture::cppTest()
{
	verifyDump("cpp");
}

void UVDFlirtutilFixture::inlineTest()
{
	verifyDump("inline");
}

void UVDFlirtutilFixture::noNameTest()
{
	verifyDump("no_name");
}

void UVDFlirtutilFixture::noNamesTest()
{
	verifyDump("no_names");
}

void UVDFlirtutilFixture::recursiveTest()
{
	verifyDump("recursive");
}

void UVDFlirtutilFixture::shortNamesTest()
{
	verifyDump("short_names");
}

void UVDFlirtutilFixture::shortNamesRefTest()
{
	verifyDump("short_names_ref");
}

void UVDFlirtutilFixture::shortTest()
{
	verifyDump("short");
}
*/

void UVDFlirtutilFixture::testingMainDumpTest()
{
	verifyDump("uvtest_main");
}

/*
void UVDFlirtutilFixture::libmTest()
{
	verifyDump("libm");
}
*/

/*
Utility
*/

void UVDFlirtutilFixture::verifyDump(const std::string &filePrefix)
{
	verifyDump(filePrefix + ".sig", filePrefix + ".dump");
}

void UVDFlirtutilFixture::verifyDump(const std::string &sigFileNameIn, const std::string &expectedDumpFileNameIn)
{
	std::string tempFile;

	std::string unitTestDir;
	//std::string outputPatFileName = getTempFileName();
	std::string output;
	std::string expectedOutput;
	std::string inputFileName;
	std::string expectedOutputFileName;
	
	unitTestDir = getUnitTestDir();
	inputFileName = unitTestDir + "/flirt/ELF/" + sigFileNameIn;
	expectedOutputFileName = unitTestDir + "/flirt/ELF/" + expectedDumpFileNameIn;

	m_args.clear();
	init();
	UVCPPUNIT_ASSERT(m_flirt->dumpSigFile(inputFileName));
	deinit();
	
	//FIXME: we aren't verifying right now...
	//we need a way to capture the output
	//probably by changing the dump API		
	printf_warn("FIXME: no dump verify support\n");
}

