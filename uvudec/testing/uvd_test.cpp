/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "main_hook.h"
#include "uvd/core/uvd.h"
#include "uvd/core/init.h"
#include "uvd_test.h"
#include "uvd/util/util.h"
#include <vector>
#include <string>
#include <string.h>

CPPUNIT_TEST_SUITE_REGISTRATION (UVDUnitTest);

void UVDUnitTest::setUp(void)
{
	m_argc = 0;
	m_argv = NULL;
	m_config = NULL;
	m_uvd = NULL;
}

void UVDUnitTest::tearDown(void) 
{
}

void UVDUnitTest::initDeinitTest(void)
{
	try
	{
		std::vector<std::string> args;
		
		CPPUNIT_ASSERT_EQUAL(UV_ERR_OK, configInit(args));
		generalDeinit();
	}
	catch(...)
	{
		configDeinitSafe();
		throw;
	}
}

void UVDUnitTest::versionTest(void)
{
	CPPUNIT_ASSERT(strcmp(UVUDEC_VER_STRING, UVDGetVersion()) == 0);
}

void UVDUnitTest::defaultDecompileFileTest(void)
{
	try
	{
		UVDData *data = NULL;
		std::string file = DEFAULT_DECOMPILE_FILE;
	
		uvdInit();

		/*
		Currently requires a file at engine init because its suppose to guess the type
		*/
		printf("Opening on %s\n", file.c_str());
		UVCPPUNIT_ASSERT(UVDDataFile::getUVDDataFile(&data, file));
		CPPUNIT_ASSERT(data != NULL);	
	
		delete data;

		configDeinit();
	}
	catch(...)
	{
		configDeinitSafe();
		throw;
	}
}

void UVDUnitTest::versionArgTest(void)
{
	try
	{
		std::vector<std::string> args;
		
		args.push_back("--version");
		
		CPPUNIT_ASSERT_EQUAL(UV_ERR_DONE, configInit(args));
		generalDeinit();
	}
	catch(...)
	{
		configDeinitSafe();
		throw;
	}
}

void UVDUnitTest::helpArgTest(void)
{
	try
	{
		std::vector<std::string> args;
		args.push_back("--help");
		
		CPPUNIT_ASSERT(configInit(args) == UV_ERR_DONE);
		generalDeinit();
	}
	catch(...)
	{
		configDeinitSafe();
		throw;
	}
}

void UVDUnitTest::engineInitTest(void)
{
	try
	{
		std::vector<std::string> args;

		generalInit(args);
		generalDeinit();
	}
	catch(...)
	{
		configDeinitSafe();
		throw;
	}
}

#define UNITTEST_ANALYSIS_DIR	"analysis.unittest"
void UVDUnitTest::analysisDirTest(void)
{
	try
	{
		std::vector<std::string> args;
	
		args.push_back("--analysis-dir=" UNITTEST_ANALYSIS_DIR);

		generalInit(args);
		//TODO: can we use objdump to do a sanity check on output files?
		//ie give it an expected symbol list and verify they exist
		UVCPPUNIT_ASSERT(m_uvd->createAnalysisDir());
		generalDeinit();
		//Lame...what is a good C func to do this recursivly?
		CPPUNIT_ASSERT(system("rm -rf " UNITTEST_ANALYSIS_DIR) == 0);
	}
	catch(...)
	{
		configDeinitSafe();
		system("rm -rf " UNITTEST_ANALYSIS_DIR);
		throw;
	}
}

void UVDUnitTest::disassembleTest(void)
{
	std::vector<std::string> args;
	std::string output;
	
	generalDisassemble(args, output);
	printf("sample output:\n%s", limitString(output, 200).c_str());
}


void UVDUnitTest::disassembleRangeTestDeliminators(void)
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

	std::vector<std::string> args;
	std::string output;
	
	printf("Range deliminator tests\n");

	//Try all possible range delminators

	printf("- range deliminator\n");
	args.clear();
	args.push_back("--addr-include=0x0000-0x0010");
	generalDisassemble(args, output);
	CPPUNIT_ASSERT(output == smallRangeTarget);

	printf(": range deliminator\n");
	args.clear();
	args.push_back("--addr-include=0x0000:0x0010");
	generalDisassemble(args, output);
	CPPUNIT_ASSERT(output == smallRangeTarget);

	printf(", range deliminator\n");
	args.clear();
	args.push_back("--addr-include=0x0000,0x0010");
	generalDisassemble(args, output);
	CPPUNIT_ASSERT(output == smallRangeTarget);
}

void UVDUnitTest::disassembleRangeTestDefaultEquivilence(void)
{
	std::vector<std::string> args;
	//Default and full range should be identical
	std::string defaultRange;
	std::string sameAsDefaultRange;
	std::string uselessExcludedrange;
	
	printf("Default range equivilence to specified\n");
	
	//Full analysis
	args.clear();
	generalDisassemble(args, defaultRange);

	//Range specified exactly to the full range should be same as default
	args.clear();
	args.push_back("--addr-include=0x0000,0xFFFF");
	generalDisassemble(args, sameAsDefaultRange);
	CPPUNIT_ASSERT(defaultRange == sameAsDefaultRange);

	//A range excluded outside of the analysis shouldn't effect output
	args.clear();
	args.push_back("--addr-exclude=0x10000,0x20000");
	generalDisassemble(args, sameAsDefaultRange);
	printf("\n\n\ndefaultRange\n<%s>\n\n\n", limitString(defaultRange, 200).c_str());
	printf("\n\n\nuselessExcludedrange\n<%s>\n\n\n", limitString(uselessExcludedrange, 200).c_str());
	CPPUNIT_ASSERT(defaultRange == uselessExcludedrange);
}

void UVDUnitTest::disassembleRangeTestComplex(void)
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
	std::vector<std::string> args;
	std::string output;
	
	printf("Range deliminator compound\n");
	
	//Try all possible range delminators

	printf("Multiple inclusion range\n");
	args.clear();
	args.push_back("--addr-include=0x0000-0x0002");
	args.push_back("--addr-include=0x000B-0x000E");
	generalDisassemble(args, output);
	printf("\n\n\noutput\n<%s>\n\n\n", limitString(output, 200).c_str());
	printf("\n\n\ntarget\n<%s>\n\n\n", limitString(target, 200).c_str());
	CPPUNIT_ASSERT(output == target);
}

void UVDUnitTest::generalDisassemble(const std::vector<std::string> &args)
{
	std::string discard;
	
	generalDisassemble(args, discard);
}

void UVDUnitTest::uvdInit()
{
	CPPUNIT_ASSERT(g_config == NULL);
	CPPUNIT_ASSERT(g_uvd == NULL);
	CPPUNIT_ASSERT(m_config == NULL);
	UVCPPUNIT_ASSERT(UVDInit());
	CPPUNIT_ASSERT(g_config != NULL);
	CPPUNIT_ASSERT(g_uvd == NULL);
}

uv_err_t UVDUnitTest::configInit(const std::vector<std::string> &args, UVDConfig **configOut)
{
	uv_err_t rc = UV_ERR_GENERAL;
	std::string temp;

	temp = stringVectorToSystemArgument(args);
	printf("To exec: %s\n", temp.c_str());
	fflush(stdout);

	//Allocate as if from main
	m_argc = args.size() + 1;
	m_argv = (char **)malloc(sizeof(char *) * m_argc);
	CPPUNIT_ASSERT(m_argv);
	m_argv[0] = strdup("uvtest");
	CPPUNIT_ASSERT(m_argv[0]);
	for( int i = 0; i < (int)args.size(); ++i )
	{
		m_argv[i + 1] = strdup(args[i].c_str());
		CPPUNIT_ASSERT(m_argv[i + 1]);
	}
	
	uvdInit();
	
	m_config = g_config;
	rc = m_config->parseMain(m_argc, m_argv);
	CPPUNIT_ASSERT(UV_SUCCEEDED(rc));
	
	if( configOut )
	{
		*configOut = g_config;
	}
	return rc;
}

void UVDUnitTest::configDeinit()
{
	//This should be deleted before tearing down the library
	CPPUNIT_ASSERT(m_uvd == NULL);
	
	//Config is created by UVDInit(), so it is not users responsibility to free
	UVCPPUNIT_ASSERT(UVDDeinit());
	//This will be deleted by UVDDeinit()
	m_config = NULL;
	
	//Library depends on these being present while active, now we can delete them
	if( m_argv )
	{
		for( int i = 0; i < m_argc; ++i )
		{
			free(m_argv[i]);
		}
		free(m_argv);
	}
	m_argc = 0;
	m_argv = NULL;	
}

void UVDUnitTest::configDeinitSafe()
{
	/*
	Don't delete anything
	Don't throw exceptions
	*/
	try
	{
		m_argc = 0;
		m_argv = NULL;
		m_config = NULL;
		m_uvd = NULL;
		UVDDeinit();
	}
	catch(...)
	{
	}
}

void UVDUnitTest::generalInit(const std::vector<std::string> &args, UVD **uvdOut)
{
	std::string file = DEFAULT_DECOMPILE_FILE;
	
	CPPUNIT_ASSERT(configInit(args) == UV_ERR_OK);
	
	/*
	Currently requires a file at engine init because its suppose to guess the type
	*/
	UVCPPUNIT_ASSERT(UVD::getUVD(&m_uvd, file));
	CPPUNIT_ASSERT(m_uvd != NULL);
	CPPUNIT_ASSERT(g_uvd != NULL);

	if( uvdOut )
	{
		*uvdOut = m_uvd;
	}
}

void UVDUnitTest::generalDeinit()
{
	//This should be deleted before tearing down the library
	delete m_uvd;
	m_uvd = NULL;

	configDeinit();
}

void UVDUnitTest::generalDisassemble(const std::vector<std::string> &args, std::string &output)
{
	try
	{
		generalInit(args);
		UVCPPUNIT_ASSERT(m_uvd->disassemble(output));
		generalDeinit();
	}
	catch(...)
	{
		configDeinitSafe();
		throw;
	}
}

void UVDUnitTest::uvudecBasicRun(void)
{
	const char *argv[] = {"uvudec", "--output=/dev/null"};
	int argc = sizeof(argv) / sizeof(argv[0]);

	UVCPPUNIT_ASSERT(uvudec_uvmain(argc, (char **)argv));
}

