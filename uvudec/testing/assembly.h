/*
UVNet Universal Decompiler (uvudec)
Copyright 2011 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_TESTING_ASSEMBLY_H
#define UVD_TESTING_ASSEMBLY_H

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "testing/framework/common_fixture.h"

class UVDAssemblyUnitTest : public UVDTestingCommonFixture
{
	CPPUNIT_TEST_SUITE(UVDAssemblyUnitTest);
	CPPUNIT_TEST(reverseDisassembleTest);
	CPPUNIT_TEST_SUITE_END();

protected:
	/*
	Tests if UVDPrintIterator's previous() function works correctly
	Needed for scrolling back in the disassembly GUI widget
	*/
	void reverseDisassembleTest(void);
	//Start and end inclusive
	void reverseDisassemble(uv_addr_t start, uv_addr_t end, std::string &out);
};

#endif

