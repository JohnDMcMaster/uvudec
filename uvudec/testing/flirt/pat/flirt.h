/*
UVNet Universal Decompiler (uvudec)
For test fixtures that need FLIRT, but not UVD
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_TESTING_LIBUVUDEC_H
#define UVD_TESTING_LIBUVUDEC_H

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "testing/common_fixture.h"

class UVDFLIRTUnitTest : public UVDTestingCommonFixture
{
protected:
	void FLIRTInit(void);
	void FLIRTDeinit(void);
	void FLIRTDeinitSafe(void);

protected:
	UVDFLIRT *m_flirt;
};

#endif

