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
	//This does NOT call any init functions
	//Only tries to clean up the current state
	virtual void setUp(void);
	virtual void tearDown(void);

protected:
	//Not only call UVDInit(), but also check expected state variables
	virtual void libraryInit();
	//Initialize UVDInit() and parse main
	uv_err_t configInit(UVDConfig **configOut = NULL);
	//Do standard deinit, including report errors
	virtual void deinit();
	//Initialize config and a UVD engine object
	//Returns the main parse code
	void generalInit(UVD **uvdOut = NULL);
	
	void argsToArgv();

	void dumpAssembly(const std::string &header, const std::string &assembly);
	//Initize and verify that we don't have any errors running these args
	//program name does not need to be supplied
	void generalDisassemble();
	void generalDisassemble(std::string &output);
	
	std::string getTempFileName();
	void deleteTempFiles();
	std::string getTempDirectoryName();
	void deleteTempDirectories();
	std::string getUnitTestDir();

public:
	std::string m_uvdInpuFileName;
	std::vector<std::string> m_tempFileNames;
	std::vector<std::string> m_tempDirectoryNames;
	std::vector<std::string> m_args;
	//After applying additional supplied args
	std::vector<std::string> m_argsFinal;
	int m_argc;
	char **m_argv;
	//Last initialized config
	UVDConfig *m_config;
	//Last initialized uvd
	UVD *m_uvd;
	bool m_wasUVDInitCalled;
};

void UVDUnitTestDir(const std::string &unitTestDir);

#endif

