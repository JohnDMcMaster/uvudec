/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_TESTING_HELPER_MACROS_H
#define UVD_TESTING_HELPER_MACROS_H

#include "testing/test_caller.h"
#include <cppunit/extensions/HelperMacros.h>

#define UVCPPUNIT_TEST( testMethod )                        \
    CPPUNIT_TEST_SUITE_ADD_TEST(                           \
        ( new UVTestCaller<TestFixtureType>(    \
                  context.getTestNameFor( #testMethod),   \
                  &TestFixtureType::testMethod,           \
                  context.makeFixture() ) ) )

#endif

