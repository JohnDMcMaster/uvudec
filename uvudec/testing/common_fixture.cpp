/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "main.h"
#include "testing/common_fixture.h"
#include "uvd/core/init.h"
#include "uvd/core/uvd.h"
#include <vector>
#include <string>
#include <string.h>

void UVDTestingCommonFixture::setUp(void)
{
	m_argc = 0;
	m_argv = NULL;
	m_config = NULL;
	m_uvd = NULL;
}

void UVDTestingCommonFixture::tearDown(void) 
{
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

void UVDTestingCommonFixture::uvdInit()
{
	CPPUNIT_ASSERT(g_config == NULL);
	CPPUNIT_ASSERT(g_uvd == NULL);
	CPPUNIT_ASSERT(m_config == NULL);
	UVCPPUNIT_ASSERT(UVDInit());
	CPPUNIT_ASSERT(g_config != NULL);
	CPPUNIT_ASSERT(g_uvd == NULL);
}

uv_err_t UVDTestingCommonFixture::configInit(UVDConfig **configOut)
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

void UVDTestingCommonFixture::configDeinit()
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

void UVDTestingCommonFixture::configDeinitSafe()
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

void UVDTestingCommonFixture::generalInit(UVD **uvdOut)
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

void UVDTestingCommonFixture::generalDeinit()
{
	//This should be deleted before tearing down the library
	delete m_uvd;
	m_uvd = NULL;

	configDeinit();
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

