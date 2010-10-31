/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "main.h"
#include "main_hook.h"
#include "uvd/core/uvd.h"
#include "uvd/core/init.h"
#include "uvd_test.h"
#include "uvd/util/util.h"
#include <vector>
#include <string>
#include <string.h>

CPPUNIT_TEST_SUITE_REGISTRATION (UVDUnitTest);

void dumpAssembly(const std::string &header, const std::string &assembly)
{
	printf("\n\n\n%s\n<%s>\n\n\n", header.c_str(), limitString(assembly, 200).c_str());
}

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

void UVDUnitTest::argsToArgv()
{
	std::vector<std::string>::size_type i = 0;

	//Allocate as if from main
	m_argc = m_args.size() + g_extraArgs.size() + 1;
	m_argv = (char **)malloc(sizeof(char *) * m_argc);
	CPPUNIT_ASSERT(m_argv);
	
	m_argv[i] = strdup("uvtest");
	CPPUNIT_ASSERT(m_argv[i]);
	++i;
	
	for( std::vector<std::string>::size_type j = 0; i < (unsigned int)m_argc && j < m_args.size(); ++i, ++j )
	{
		m_argv[i] = strdup(m_args[j].c_str());
		CPPUNIT_ASSERT(m_argv[i]);
	}
	
	for( std::vector<std::string>::size_type j = 0; i < (unsigned int)m_argc && j < g_extraArgs.size(); ++i, ++j )
	{
		m_argv[i] = strdup(g_extraArgs[j].c_str());
		CPPUNIT_ASSERT(m_argv[i]);
	}

	//Copy in so we have for debugging	
	m_argsFinal.clear();
	for( int j = 0; j < m_argc; ++j )
	{
		m_argsFinal.push_back(m_argv[j]);
	}
}

/*
Utility functions
*/

void UVDUnitTest::uvdInit()
{
	CPPUNIT_ASSERT(g_config == NULL);
	CPPUNIT_ASSERT(g_uvd == NULL);
	CPPUNIT_ASSERT(m_config == NULL);
	UVCPPUNIT_ASSERT(UVDInit());
	CPPUNIT_ASSERT(g_config != NULL);
	CPPUNIT_ASSERT(g_uvd == NULL);
}

uv_err_t UVDUnitTest::configInit(UVDConfig **configOut)
{
	uv_err_t rc = UV_ERR_GENERAL;

	argsToArgv();
	printf("To exec : %s\n", stringVectorToSystemArgument(m_argsFinal).c_str());
	fflush(stdout);

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

void UVDUnitTest::generalInit(UVD **uvdOut)
{
	std::string file = DEFAULT_DECOMPILE_FILE;
	
	CPPUNIT_ASSERT(configInit() == UV_ERR_OK);
	
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

void UVDUnitTest::generalDisassemble()
{
	std::string discard;
	
	generalDisassemble(discard);
}

void UVDUnitTest::generalDisassemble(std::string &output)
{
	try
	{
		generalInit();
		UVCPPUNIT_ASSERT(m_uvd->disassemble(output));
		generalDeinit();
	}
	catch(...)
	{
		configDeinitSafe();
		throw;
	}
}

/*
Tests
*/

void UVDUnitTest::initDeinitTest(void)
{
	try
	{
		m_args.clear();	
		CPPUNIT_ASSERT_EQUAL(UV_ERR_OK, configInit());
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
		m_args.clear();
		m_args.push_back("--version");
		
		CPPUNIT_ASSERT_EQUAL(UV_ERR_DONE, configInit());
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
		m_args.clear();
		m_args.push_back("--help");		

		CPPUNIT_ASSERT(configInit() == UV_ERR_DONE);
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
		m_args.clear();

		generalInit();
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
		m_args.clear();	
		m_args.push_back("--analysis-dir=" UNITTEST_ANALYSIS_DIR);

		generalInit();
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
	std::string output;
	
	m_args.clear();

	generalDisassemble(output);
	//This just dumps unneeded junk to output
	//dumpAssembly("sample output", output);
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

void UVDUnitTest::disassembleRangeTestDefaultEquivilence(void)
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
	std::string output;
	
	printf("Range deliminator compound\n");
	
	//Try all possible range delminators

	printf("Multiple inclusion range\n");
	m_args.clear();
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

void UVDUnitTest::uvudecBasicRun(void)
{
	m_args.clear();
	m_args.push_back("--output=/dev/null");
	argsToArgv();

	try
	{
		UVCPPUNIT_ASSERT(uvudec_uvmain(m_argc, m_argv));
	}
	catch(...)
	{
		delete m_argv;
		m_argv = NULL;
		throw;
	}
}

