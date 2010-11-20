/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "testing/object.h"
#include "uvd/core/runtime.h"
#include "uvd/core/uvd.h"

void UVDTestingObjectFixture::setUp()
{
	m_object = NULL;
}

void UVDTestingObjectFixture::init()
{
	UVDTestingPluginFixture::init();
	CPPUNIT_ASSERT(m_uvd);
	CPPUNIT_ASSERT(m_uvd->m_runtime);
	CPPUNIT_ASSERT(m_uvd->m_runtime->m_object);
	m_object = m_uvd->m_runtime->m_object;
}

