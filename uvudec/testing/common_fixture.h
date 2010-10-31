/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_TESTING_COMMON_FIXTURE_H
#define UVD_TESTING_COMMON_FIXTURE_H

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "uvd/config/config.h"
#include "uvd/util/types.h"

#define UVCPPUNIT_ASSERT(x)			CPPUNIT_ASSERT(UV_SUCCEEDED(UV_DEBUG(x)))

class UVDTestingCommonFixture : public CPPUNIT_NS::TestFixture
{
public:
	void setUp(void);
	void tearDown(void);

protected:
	//Only call UVDInit(), but also check expected state variables
	void uvdInit();
	//Initialize UVDInit() and parse main
	uv_err_t configInit(UVDConfig **configOut = NULL);
	//Do standard deinit, including report errors
	void configDeinit();
	//Try to reset us to a sane state for the next test afer an error
	//This version will not throw exceptions and will not clean up memory
	void configDeinitSafe();
	
	//Initialize config and a UVD engine object
	//Returns the main parse code
	void generalInit(UVD **uvdOut = NULL);
	void generalDeinit();
	
	void argsToArgv();

	void dumpAssembly(const std::string &header, const std::string &assembly);
	//Initize and verify that we don't have any errors running these args
	//program name does not need to be supplied
	void generalDisassemble();
	void generalDisassemble(std::string &output);

public:
	std::vector<std::string> m_args;
	//After applying additional supplied args
	std::vector<std::string> m_argsFinal;
	int m_argc;
	char **m_argv;
	//Last initialized config
	UVDConfig *m_config;
	//Last initialized uvd
	UVD *m_uvd;
};

#endif

