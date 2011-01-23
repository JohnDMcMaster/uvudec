/*
UVNet Universal Decompiler (uvudec)
Copyright 2011 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "testing/assembly.h"
#include "uvd/core/uvd.h"
#include "uvd/language/language.h"
#include <string.h>

CPPUNIT_TEST_SUITE_REGISTRATION(UVDAssemblyUnitTest);

void UVDAssemblyUnitTest::reverseDisassemble(uv_addr_t start, uv_addr_t end, std::string &out)
{
	std::string output;
	UVDPrintIterator iter;
	UVDPrintIterator endIter;

	generalInit();
	UVCPPUNIT_ASSERT(m_uvd->analyze());
	//We should probably set this on the print iterator
	//Assembly should also be default since it should more or less always work
	//Decompile or disassemble as we go?
	UVCPPUNIT_ASSERT(m_uvd->setDestinationLanguage(UVD_LANGUAGE_ASSEMBLY));
	
	out.clear();
	UVCPPUNIT_ASSERT(m_uvd->begin(UVDAddress(start, NULL), iter));
	UVCPPUNIT_ASSERT(m_uvd->begin(UVDAddress(end, NULL), endIter));
	for( ;; )
	{
		std::string line;

		CPPUNIT_ASSERT(iter != m_uvd->end());
		UVCPPUNIT_ASSERT(iter.getCurrent(line));
		out += line + "\n";
		
		if( iter == endIter )
		{
			break;
		}
		UVCPPUNIT_ASSERT(iter.previous());
	}

	deinit();
}

void UVDAssemblyUnitTest::reverseDisassembleTest(void)
{
	/*
	# 0x00000000
	LJMP #0x0026
	# 0x00000003
	MOV R7, A
	# 0x00000004
	MOV R7, A
	# 0x00000005
	MOV R7, A
	# 0x00000006
	MOV R7, A
	# 0x00000007
	MOV R7, A
	# 0x00000008
	MOV R7, A
	# 0x00000009
	MOV R7, A
	# 0x0000000A
	MOV R7, A
	# 0x0000000B
	LJMP #0x0DA9
	*/
	
	//Maybe should chose something with more distinct instructions
	std::string expected = 
			"LJMP #0x0DA9\n"
			"MOV R7, A\n"
			"MOV R7, A\n"
			"MOV R7, A\n"
			"MOV R7, A\n"
			"MOV R7, A\n"
			"MOV R7, A\n"
			"MOV R7, A\n"
			"MOV R7, A\n"
			"LJMP #0x0026\n"
			;
	std::string actual;

	try
	{
		reverseDisassemble(0x0B, 0x00, actual);
	}
	catch(...)
	{
		dumpAssembly("expected", expected);
		dumpAssembly("actual", actual);
		throw;
	}	
}

