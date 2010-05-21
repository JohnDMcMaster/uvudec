// CppUnit-Tutorial
// file: UVDUnitTest.cc
#include "uvd.h"
#include "uvd_init.h"
#include "uvd_test.h"

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
	
	/*
	Currently requires a file at engine init because its suppose to guess the type
	*/
	printf("Opening on %s\n", file.c_str());
	UVCPPUNIT_ASSERT(UVDDataFile::getUVDDataFile(&data, file));
	CPPUNIT_ASSERT(data != NULL);	
	
	delete data;
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
	UVDData *data = NULL;
	
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
	char *argv[] = {"uvudec", "--analysis-dir=" UNITTEST_ANALYSIS_DIR};
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
	CPPUNIT_ASSERT(g_config->parseMain(argc, argv) == UV_ERR_OK);
	
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
	char *argv[] = {"uvudec"};
	std::string file = DEFAULT_DECOMPILE_FILE;
	UVD *uvd = NULL;
	UVDData *data = NULL;
	std::string output;
	uint32_t sampleLength = 200;
	
	//strncpy(arg0, "uvudec", sizeof(arg0)); 
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

	UVCPPUNIT_ASSERT(UVD::getUVD(&uvd, data));
	CPPUNIT_ASSERT(uvd != NULL);
	CPPUNIT_ASSERT(g_uvd != NULL);

	UVCPPUNIT_ASSERT(uvd->disassemble(output));
	if( output.size() < sampleLength )
	{
		sampleLength = output.size();
	}
	printf("sample output:\n%s", output.substr(0, sampleLength).c_str());

	UVCPPUNIT_ASSERT(UVDDeinit());
	CPPUNIT_ASSERT(g_config == NULL);
	CPPUNIT_ASSERT(g_uvd == NULL);

	delete data;
}
