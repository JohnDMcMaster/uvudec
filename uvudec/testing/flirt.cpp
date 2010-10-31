/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "testing/flirt.h"

void UVDFLIRTUnitTest::FLIRTInit()
{
	CPPUNIT_ASSERT(configInit() == UV_ERR_OK);
	
	/*
	Currently requires a file at engine init because its suppose to guess the type
	*/
	UVCPPUNIT_ASSERT(UVDFLIRT::getFLIRT(&m_flirt));
	CPPUNIT_ASSERT(m_flirt != NULL);
}

void UVDFLIRTUnitTest::FLIRTDeinit()
{
	delete m_flirt;
	m_flirt = NULL;
	
	configDeinit();
}

void UVDFLIRTUnitTest::FLIRTDeinitSafe()
{
	m_flirt = NULL;
	
	configDeinitSafe();
}

