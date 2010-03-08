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
}
