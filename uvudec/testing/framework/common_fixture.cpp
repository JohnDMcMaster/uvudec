/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "main.h"
#include "testing/common_fixture.h"
#include "uvd/core/init.h"
#include "uvd/core/uvd.h"
#include "uvd/util/util.h"
#include <vector>
#include <string>
#include <string.h>

void UVDTestingCommonFixture::setUp(void)
{
	printf("UVDTestingCommonFixture::setUp()\n");
	m_argc = 0;
	m_argv = NULL;
	m_config = NULL;
	m_uvd = NULL;
	m_uvdInpuFileName = DEFAULT_DECOMPILE_FILE;
	m_args.clear();
	m_wasUVDInitCalled = false;
}

void UVDTestingCommonFixture::tearDown(void) 
{
	/*
	Don't delete anything local
	Don't throw exceptions
	*/
	printf("UVDTestingCommonFixture::tearDown\n");
	try
	{
		m_argc = 0;
		m_argv = NULL;
		m_config = NULL;
		m_uvd = NULL;
		//Try to clean up the library state
		//If this is going to crash, future tests probably would anyway
		if( m_wasUVDInitCalled )
		{
			UVDDeinit();
			m_wasUVDInitCalled = false;
		}
		deleteTempFiles();
		deleteTempDirectories();
	}
	catch(...)
	{
	}
}

void UVDTestingCommonFixture::appendArgument(const std::string &arg)
{
	m_args.push_back(arg);
}

void UVDTestingCommonFixture::argsToArgv()
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

void UVDTestingCommonFixture::libraryInit()
{
	g_config = NULL;
	
	
	
	CPPUNIT_ASSERT(g_config == NULL);
	CPPUNIT_ASSERT(g_uvd == NULL);
	CPPUNIT_ASSERT(m_config == NULL);
	UVCPPUNIT_ASSERT(UVDInit());
	m_wasUVDInitCalled = true;
	CPPUNIT_ASSERT(g_config != NULL);
	CPPUNIT_ASSERT(g_uvd == NULL);
}

uv_err_t UVDTestingCommonFixture::configInit(UVDConfig **configOut)
{
	uv_err_t rc = UV_ERR_GENERAL;

	argsToArgv();
	printf("To exec : %s\n", stringVectorToSystemArgument(m_argsFinal).c_str());
	fflush(stdout);

	libraryInit();
	
	m_config = g_config;
	rc = m_config->parseMain(m_argc, m_argv);
	CPPUNIT_ASSERT(UV_SUCCEEDED(rc));
	
	if( configOut )
	{
		*configOut = g_config;
	}
	return rc;
}

void UVDTestingCommonFixture::deinit()
{
	//This should be deleted before tearing down the library
	delete m_uvd;
	m_uvd = NULL;
	
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
	
	deleteTempFiles();
	deleteTempDirectories();
}

uv_err_t UVDTestingCommonFixture::generalInit(UVD **uvdOut)
{
	uv_err_t rc = UV_ERR_GENERAL;
	
	CPPUNIT_ASSERT(configInit() == UV_ERR_OK);
	
	/*
	Currently requires a file at engine init because its suppose to guess the type
	*/
	printf("General init on %s\n", m_uvdInpuFileName.c_str());
	rc = UVD::getUVDFromFileName(&m_uvd, m_uvdInpuFileName);
	printf("rc: %d\n", rc);
	UVCPPUNIT_ASSERT(rc);
	CPPUNIT_ASSERT(m_uvd != NULL);
	CPPUNIT_ASSERT(g_uvd != NULL);

	if( uvdOut )
	{
		*uvdOut = m_uvd;
	}
	
	return rc;
}

void UVDTestingCommonFixture::dumpAssembly(const std::string &header, const std::string &assembly)
{
	printf("\n\n\n%s\n<%s>\n\n\n", header.c_str(), limitString(assembly, 200).c_str());
}

void UVDTestingCommonFixture::generalDisassemble()
{
	std::string discard;
	
	generalDisassemble(discard);
}

void UVDTestingCommonFixture::generalDisassemble(std::string &output)
{
	generalInit();
	UVCPPUNIT_ASSERT(m_uvd->disassemble(output));
	deinit();
}

std::string UVDTestingCommonFixture::getTempFileName()
{
	std::string tempFileName;
	
	UVCPPUNIT_ASSERT(m_tempFileNames.empty());
	tempFileName = "/tmp/uvtest_file";
	m_tempFileNames.push_back(tempFileName);
	
	return tempFileName;
}

std::string UVDTestingCommonFixture::getTempDirectoryName()
{
	std::string tempDirectoryName;
	
	UVCPPUNIT_ASSERT(m_tempDirectoryNames.empty());
	tempDirectoryName = "/tmp/uvtest_dir";
	m_tempDirectoryNames.push_back(tempDirectoryName);
	
	return tempDirectoryName;
}

void UVDTestingCommonFixture::deleteTempFiles()
{
	for( std::vector<std::string>::iterator iter = m_tempFileNames.begin();
			iter != m_tempFileNames.end(); ++iter )
	{
		std::string fileName = *iter;
		std::string command;
		
		command += "rm -f ";
		command += fileName;
		system(command.c_str());
	}
	m_tempFileNames.clear();
}

void UVDTestingCommonFixture::deleteTempDirectories()
{
	for( std::vector<std::string>::iterator iter = m_tempDirectoryNames.begin();
			iter != m_tempDirectoryNames.end(); ++iter )
	{
		std::string directoryName = *iter;
		std::string command;
		
		command += "rm -rf ";
		command += directoryName;
		system(command.c_str());
	}
	m_tempDirectoryNames.clear();
}

std::string UVDTestingCommonFixture::getUnitTestDir()
{
	std::string installDir;
	
	//Assume we are doing local dev and not really installing for now
	UVCPPUNIT_ASSERT(UVDGetInstallDir(installDir));
	return installDir + "/testing";
}

