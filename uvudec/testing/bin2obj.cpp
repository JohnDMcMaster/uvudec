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

CPPUNIT_TEST_SUITE_REGISTRATION(UVDBin2objTestFixture);

/*
Tests
*/

#define UNITTEST_ANALYSIS_DIR	"analysis.unittest"
void UVDBin2objTestFixture::analysisDirTest(void)
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
}


