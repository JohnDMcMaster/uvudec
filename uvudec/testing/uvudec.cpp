/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "main.h"
#include "main_hook.h"
#include "uvd/core/uvd.h"
#include "uvd/core/init.h"
#include "testing/uvudec.h"
#include "uvd/util/util.h"
#include <vector>
#include <string>
#include <string.h>

CPPUNIT_TEST_SUITE_REGISTRATION(UVDUvudecUnitTest);

/*
Tests
*/

void UVDUvudecUnitTest::defaultDecompileFileTest(void)
{
	UVDData *data = NULL;
	std::string file = DEFAULT_DECOMPILE_FILE;

	libraryInit();

	/*
	Currently requires a file at engine init because its suppose to guess the type
	*/
	printf("Opening on %s\n", file.c_str());
	UVCPPUNIT_ASSERT(UVDDataFile::getUVDDataFile(&data, file));
	CPPUNIT_ASSERT(data != NULL);	

	delete data;

	deinit();
}

void UVDUvudecUnitTest::versionArgTest(void)
{
	m_args.push_back("--version");
	
	CPPUNIT_ASSERT_EQUAL(UV_ERR_DONE, configInit());
	deinit();
}

void UVDUvudecUnitTest::helpArgTest(void)
{
	m_args.push_back("--help");		

	CPPUNIT_ASSERT(configInit() == UV_ERR_DONE);
	deinit();
}

void UVDUvudecUnitTest::engineInitTest(void)
{
	generalInit();
	deinit();
}

void UVDUvudecUnitTest::disassembleTest(void)
{
	std::string output;
	
	generalDisassemble(output);
	//This just dumps unneeded junk to output
	//dumpAssembly("sample output", output);
}


void UVDUvudecUnitTest::disassembleRangeTestDeliminatorsTest(void)
{
	/*
	This asssumes using the candela image

	From 0x0000:0x0010
	
	LJMP #0x0026
	MOV R7, A
	MOV R7, A
	MOV R7, A
	MOV R7, A
	MOV R7, A
	MOV R7, A
	MOV R7, A
	MOV R7, A
	LJMP #0x0DA9
	MOV R7, A
	MOV R7, A
	MOV R7, A
	*/

	std::string smallRangeTarget =
		"LJMP #0x0026\n"
		"MOV R7, A\n"
		"MOV R7, A\n"
		"MOV R7, A\n"
		"MOV R7, A\n"
		"MOV R7, A\n"
		"MOV R7, A\n"
		"MOV R7, A\n"
		"MOV R7, A\n"
		"LJMP #0x0DA9\n"
		"MOV R7, A\n"
		"MOV R7, A\n"
		"MOV R7, A\n";

	std::string output;
	
	printf("Range deliminator tests\n");

	//Try all possible range delminators

	printf("- range deliminator\n");
	m_args.clear();
	m_args.push_back("--addr-include=0x0000-0x0010");
	generalDisassemble(output);
	try
	{
		CPPUNIT_ASSERT(output == smallRangeTarget);
	}
	catch(...)
	{
		dumpAssembly("smallRangeTarget", smallRangeTarget);
		dumpAssembly("output", output);
		throw;
	}
	
	printf(": range deliminator\n");
	m_args.clear();
	m_args.push_back("--addr-include=0x0000:0x0010");
	generalDisassemble(output);
	try
	{
		CPPUNIT_ASSERT(output == smallRangeTarget);
	}
	catch(...)
	{
		dumpAssembly("smallRangeTarget", smallRangeTarget);
		dumpAssembly("output", output);
		throw;
	}

	printf(", range deliminator\n");
	m_args.clear();
	m_args.push_back("--addr-include=0x0000,0x0010");
	generalDisassemble(output);
	try
	{
		CPPUNIT_ASSERT(output == smallRangeTarget);
	}
	catch(...)
	{
		dumpAssembly("smallRangeTarget", smallRangeTarget);
		dumpAssembly("output", output);
		throw;
	}
}

void UVDUvudecUnitTest::disassembleRangeTestDefaultEquivilenceTest(void)
{
	//Default and full range should be identical
	std::string defaultRange;
	std::string sameAsDefaultRange;
	std::string uselessExcludedrange;
	
	printf("Default range equivilence to specified\n");
	
	//Full analysis
	m_args.clear();
	generalDisassemble(defaultRange);

	//Range specified exactly to the full range should be same as default
	m_args.clear();
	m_args.push_back("--addr-include=0x0000,0xFFFF");
	generalDisassemble(sameAsDefaultRange);
	try
	{
		CPPUNIT_ASSERT(defaultRange == sameAsDefaultRange);
	}
	catch(...)
	{
		dumpAssembly("default range", defaultRange);
		dumpAssembly("sameAsDefaultRange", sameAsDefaultRange);
		throw;
	}

	//A range excluded outside of the analysis shouldn't effect output
	m_args.clear();
	m_args.push_back("--addr-exclude=0x10000,0x20000");
	generalDisassemble(uselessExcludedrange);
	try
	{
		CPPUNIT_ASSERT(defaultRange == uselessExcludedrange);
	}
	catch(...)
	{
		dumpAssembly("default range", defaultRange);
		dumpAssembly("useless excluded range", uselessExcludedrange);
		throw;
	}
}

void UVDUvudecUnitTest::disassembleRangeTestComplexTest(void)
{
	/*
	This asssumes using the candela image

	0x00-0x02
		LJMP #0x0026
	0x0B-0x0E:
		LJMP #0x0DA9
		MOV R7, A
	*/

	std::string target =
		"LJMP #0x0026\n"
		"LJMP #0x0DA9\n"
		"MOV R7, A\n";
	std::string output;
	
	printf("Range deliminator compound\n");
	
	//Try all possible range delminators

	printf("Multiple inclusion range\n");
	m_args.push_back("--addr-include=0x0000-0x0002");
	m_args.push_back("--addr-include=0x000B-0x000E");
	generalDisassemble(output);

	try
	{
		CPPUNIT_ASSERT(output == target);
	}
	catch(...)
	{
		dumpAssembly("target", target);
		dumpAssembly("output", output);
		throw;
	}
}

void UVDUvudecUnitTest::uvudecBasicRunTest(void)
{
	m_args.push_back("--output=/dev/null");
	argsToArgv();

	UVCPPUNIT_ASSERT(uvudec_uvmain(m_argc, m_argv));
}

