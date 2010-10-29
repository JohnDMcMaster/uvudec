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

void UVDUnitTest::setUp (void)
{
}

void UVDUnitTest::tearDown (void) 
{
}

void UVDUnitTest::initDeinitTest(void)
{
	CPPUNIT_ASSERT(g_config == NULL);
	CPPUNIT_ASSERT(g_uvd == NULL);

	CPPUNIT_ASSERT(UV_SUCCEEDED(UVDInit()));
	//Should initialize config: logging, arg parsing structures
	CPPUNIT_ASSERT(g_config != NULL);
	//Should not be initialized by this call
	CPPUNIT_ASSERT(g_uvd == NULL);

	CPPUNIT_ASSERT(UV_SUCCEEDED(UVDDeinit()));
	CPPUNIT_ASSERT(g_config == NULL);
	CPPUNIT_ASSERT(g_uvd == NULL);
}

void UVDUnitTest::versionTest(void)
{
	CPPUNIT_ASSERT(strcmp(UVUDEC_VER_STRING, UVDGetVersion()) == 0);
}

void UVDUnitTest::defaultDecompileFileTest(void)
{
	UVDData *data = NULL;
	std::string file = DEFAULT_DECOMPILE_FILE;
	
	CPPUNIT_ASSERT(UV_SUCCEEDED(UVDInit()));

	/*
	Currently requires a file at engine init because its suppose to guess the type
	*/
	printf("Opening on %s\n", file.c_str());
	UVCPPUNIT_ASSERT(UVDDataFile::getUVDDataFile(&data, file));
	CPPUNIT_ASSERT(data != NULL);	
	
	delete data;

	CPPUNIT_ASSERT(UV_SUCCEEDED(UVDDeinit()));
}

void UVDUnitTest::versionArgTest(void)
{
	int argc = 0;
	char arg0[32];
	char arg1[32];
	char *argv[] = {arg0, arg1};
	
	strncpy(arg0, "uvudec", sizeof(arg0)); 
	strncpy(arg1, "--version", sizeof(arg1)); 
	argc = sizeof(argv) / sizeof(argv[0]);
	
	CPPUNIT_ASSERT(UV_SUCCEEDED(UVDInit()));
	CPPUNIT_ASSERT(g_config != NULL);
	CPPUNIT_ASSERT(g_config->parseMain(argc, argv) == UV_ERR_DONE);
	CPPUNIT_ASSERT(UV_SUCCEEDED(UVDDeinit()));
}

void UVDUnitTest::helpArgTest(void)
{
	int argc = 0;
	char arg0[32];
	char arg1[32];
	char *argv[] = {arg0, arg1};
	
	strncpy(arg0, "uvudec", sizeof(arg0)); 
	strncpy(arg1, "--help", sizeof(arg1)); 
	argc = sizeof(argv) / sizeof(argv[0]);
	
	CPPUNIT_ASSERT(UV_SUCCEEDED(UVDInit()));
	CPPUNIT_ASSERT(g_config != NULL);
	CPPUNIT_ASSERT(g_config->parseMain(argc, argv) == UV_ERR_DONE);
	CPPUNIT_ASSERT(UV_SUCCEEDED(UVDDeinit()));
}

void UVDUnitTest::engineInitTest(void)
{
	int argc = 0;
	char arg0[32];
	char *argv[] = {arg0};
	std::string file = DEFAULT_DECOMPILE_FILE;
	UVD *uvd = NULL;
	UVDDataFile *data = NULL;
	
	strncpy(arg0, "uvudec", sizeof(arg0)); 
	argc = sizeof(argv) / sizeof(argv[0]);
	
	CPPUNIT_ASSERT(g_config == NULL);
	CPPUNIT_ASSERT(g_uvd == NULL);
	UVCPPUNIT_ASSERT(UVDInit());
	CPPUNIT_ASSERT(g_config != NULL);
	CPPUNIT_ASSERT(g_uvd == NULL);
	CPPUNIT_ASSERT(g_config->parseMain(argc, argv) == UV_ERR_OK);
	
	/*
	Currently requires a file at engine init because its suppose to guess the type
	*/
	UVCPPUNIT_ASSERT(UVDDataFile::getUVDDataFile(&data, file));
	CPPUNIT_ASSERT(data != NULL);
	UVD_POKE(data);

	UVCPPUNIT_ASSERT(UVD::getUVD(&uvd, data));
	CPPUNIT_ASSERT(uvd != NULL);
	CPPUNIT_ASSERT(g_uvd != NULL);

	UVCPPUNIT_ASSERT(UVDDeinit());
	CPPUNIT_ASSERT(g_config == NULL);
	CPPUNIT_ASSERT(g_uvd == NULL);
	
	delete data;
}

#define UNITTEST_ANALYSIS_DIR	"analysis.unittest"
void UVDUnitTest::analysisDirTest(void)
{
	int argc = 0;
	//char arg0[32];
	const char *argv[] = {"uvudec", "--analysis-dir=" UNITTEST_ANALYSIS_DIR};
	std::string file = DEFAULT_DECOMPILE_FILE;
	UVD *uvd = NULL;
	UVDData *data = NULL;
	
	//May not exist, ignore errors
	remove(UNITTEST_ANALYSIS_DIR);
	
	//strncpy(arg0, "uvudec", sizeof(arg0)); 
	argc = sizeof(argv) / sizeof(argv[0]);
	
	CPPUNIT_ASSERT(g_config == NULL);
	CPPUNIT_ASSERT(g_uvd == NULL);
	UVCPPUNIT_ASSERT(UVDInit());
	CPPUNIT_ASSERT(g_config != NULL);
	CPPUNIT_ASSERT(g_uvd == NULL);
	CPPUNIT_ASSERT(g_config->parseMain(argc, (char **)argv) == UV_ERR_OK);
	
	/*
	Currently requires a file at engine init because its suppose to guess the type
	*/
	UVCPPUNIT_ASSERT(UVDDataFile::getUVDDataFile(&data, file));
	CPPUNIT_ASSERT(data != NULL);	

	UVCPPUNIT_ASSERT(UVD::getUVD(&uvd, data));
	CPPUNIT_ASSERT(uvd != NULL);
	CPPUNIT_ASSERT(g_uvd != NULL);

	UVCPPUNIT_ASSERT(uvd->createAnalysisDir());
	//TODO: can we use objdump to do a sanity check on output files?
	//ie give it an expected symbol list and verify they exist
	
	//Lame...what is a good C func to do this recursivly?
	CPPUNIT_ASSERT(system("rm -rf " UNITTEST_ANALYSIS_DIR) == 0);

	UVCPPUNIT_ASSERT(UVDDeinit());
	CPPUNIT_ASSERT(g_config == NULL);
	CPPUNIT_ASSERT(g_uvd == NULL);

	delete data;
}

void UVDUnitTest::disassembleTest(void)
{
	int argc = 0;
	//char arg0[32];
	const char *argv[] = {"uvudec"};
	std::string file = DEFAULT_DECOMPILE_FILE;
	UVD *uvd = NULL;
	UVDData *data = NULL;
	std::string output;
	
	//strncpy(arg0, "uvudec", sizeof(arg0)); 
	argc = sizeof(argv) / sizeof(argv[0]);
	
	CPPUNIT_ASSERT(g_config == NULL);
	CPPUNIT_ASSERT(g_uvd == NULL);
	UVCPPUNIT_ASSERT(UVDInit());
	CPPUNIT_ASSERT(g_config != NULL);
	CPPUNIT_ASSERT(g_uvd == NULL);
	CPPUNIT_ASSERT(g_config->parseMain(argc, (char **)argv) == UV_ERR_OK);
	
	/*
	Currently requires a file at engine init because its suppose to guess the type
	*/
	UVCPPUNIT_ASSERT(UVDDataFile::getUVDDataFile(&data, file));
	CPPUNIT_ASSERT(data != NULL);	

	UVCPPUNIT_ASSERT(UVD::getUVD(&uvd, data));
	CPPUNIT_ASSERT(uvd != NULL);
	CPPUNIT_ASSERT(g_uvd != NULL);

	UVCPPUNIT_ASSERT(uvd->disassemble(output));
	printf("sample output:\n%s", limitString(output, 200).c_str());

	UVCPPUNIT_ASSERT(UVDDeinit());
	CPPUNIT_ASSERT(g_config == NULL);
	CPPUNIT_ASSERT(g_uvd == NULL);

	delete data;
}

/*
class UVDIERange
{
public:
	UVDIERange()
	{
		m_ie = 0;
		m_low = 0;
		m_high = 0;
	}
	
public:
	//Include or exclude
	//UVD_ADDRESS_ANALYSIS_UNKNOWN, UVD_ADDRESS_ANALYSIS_INCLUDE, UVD_ADDRESS_ANALYSIS_EXCLUDE
	uint32_t m_ie;
	uv_addr_t m_low;
	uv_addr_t m_high;
};

void disassembleSpecificRangeTest(const std::vector<UVDIERange> &ranges)
{
}
*/

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

	CPPUNIT_ASSERT(UV_SUCCEEDED(UVDInit()));
	
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
	CPPUNIT_ASSERT(UV_SUCCEEDED(UVDDeinit()));
}

void UVDUnitTest::disassembleRangeTestDefaultEquivilence(void)
{
	std::vector<std::string> args;
	//Default and full range should be identical
	std::string defaultRange;
	std::string sameAsDefaultRange;
	std::string uselessExcludedrange;
	
	printf("Default range equivilence to specified\n");
	CPPUNIT_ASSERT(UV_SUCCEEDED(UVDInit()));
	
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
	CPPUNIT_ASSERT(UV_SUCCEEDED(UVDDeinit()));
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
	CPPUNIT_ASSERT(UV_SUCCEEDED(UVDInit()));
	
	//Try all possible range delminators
	
	printf("Multiple inclusion range\n");
	args.clear();
	args.push_back("--addr-include=0x0000-0x0002");
	args.push_back("--addr-include=0x000B-0x000E");
	generalDisassemble(args, output);
	printf("\n\n\noutput<%s>\n\n\n", limitString(output, 200).c_str());
	printf("\n\n\ntarget<%s>\n\n\n", limitString(target, 200).c_str());
	CPPUNIT_ASSERT(output == target);
	CPPUNIT_ASSERT(UV_SUCCEEDED(UVDDeinit()));
}

void UVDUnitTest::generalDisassemble(const std::vector<std::string> &args)
{
	std::string discard;
	
	generalDisassemble(args, discard);
}

void UVDUnitTest::generalDisassemble(const std::vector<std::string> &args, std::string &output)
{
	int argc = 0;
	//char arg0[32];
	const char *argv[16] = {"uvudec"};
	std::string file = DEFAULT_DECOMPILE_FILE;
	UVD *uvd = NULL;
	UVDData *data = NULL;
	
	//Make sure we can fit it in the buffer
	CPPUNIT_ASSERT(sizeof(argv) / sizeof(argv[0]) > args.size());
	
	std::string toExec;
	
	//Prog name
	argc = 1;
	toExec += argv[0];
	//Copy all args into the buffer
	for( std::vector<std::string>::size_type i = 0; i < args.size(); ++i )
	{
		std::string arg = args[i];
		
		toExec += " ";
		toExec += arg;
		argv[argc] = (char *)arg.c_str();
		++argc;
	}
	printf("To exec: %s\n", toExec.c_str());
	
	CPPUNIT_ASSERT(g_config == NULL);
	CPPUNIT_ASSERT(g_uvd == NULL);
	UVCPPUNIT_ASSERT(UVDInit());
	CPPUNIT_ASSERT(g_config != NULL);
	CPPUNIT_ASSERT(g_uvd == NULL);
	CPPUNIT_ASSERT(g_config->parseMain(argc, (char **)argv) == UV_ERR_OK);
	
	/*
	Currently requires a file at engine init because its suppose to guess the type
	*/
	UVCPPUNIT_ASSERT(UVDDataFile::getUVDDataFile(&data, file));
	CPPUNIT_ASSERT(data != NULL);	

	UVCPPUNIT_ASSERT(UVD::getUVD(&uvd, data));
	CPPUNIT_ASSERT(uvd != NULL);
	CPPUNIT_ASSERT(g_uvd != NULL);

	UVCPPUNIT_ASSERT(uvd->disassemble(output));

	UVCPPUNIT_ASSERT(UVDDeinit());
	CPPUNIT_ASSERT(g_config == NULL);
	CPPUNIT_ASSERT(g_uvd == NULL);

	delete data;
}

void UVDUnitTest::uvudecBasicRun(void)
{
	const char *argv[] = {"uvudec", "--output=/dev/null"};
	int argc = sizeof(argv) / sizeof(argv[0]);

	UVCPPUNIT_ASSERT(uvudec_uvmain(argc, (char **)argv));
}
