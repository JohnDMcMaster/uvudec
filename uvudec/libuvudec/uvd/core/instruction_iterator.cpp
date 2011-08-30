/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <vector>
#include "uvd/architecture/architecture.h"
#include "uvd/assembly/address.h"
#include "uvd/assembly/instruction.h"
#include "uvd/core/analysis.h"
#include "uvd/core/runtime.h"
#include "uvd/core/std_iterator.h"
#include "uvd/core/uvd.h"
#include "uvd/data/data.h"
#include "uvd/language/format.h"
#include "uvd/string/string.h"
#include "uvd/util/benchmark.h"
#include "uvd/util/debug.h"
#include "uvd/util/error.h"
#include "uvd/util/types.h"
#include "uvd/util/util.h"

uv_err_t UVDInstructionIterator::operator=(const UVDInstructionIterator &other) {
	uv_assert_ret(other.m_iter);
	delete m_iter;
	m_iter = NULL;
	uv_assert_err_ret(other.m_iter->copy(&m_iter));
	uv_assert_ret(m_iter);
	//return *this;
	return UV_ERR_OK;
}

int UVDInstructionIterator::compare(const UVDInstructionIterator &other) const {
	uv_assert_ret(m_iter != NULL);
	return m_iter->compare(*other.m_iter);
}

/*
UVDAbstractInstructionIterator
*/
UVDAbstractInstructionIterator::UVDAbstractInstructionIterator(/*UVD *uvd*/) {
}

UVDAbstractInstructionIterator::~UVDAbstractInstructionIterator() {
}

uv_err_t UVDAbstractInstructionIterator::previous() {
	return UV_ERR_NOTIMPLEMENTED;
}


#if 0
/*
UVDStdInstructionIterator
*/
UVDStdInstructionIterator::UVDStdInstructionIterator(UVD *uvd)
	: UVDAbstractInstructionIterator() {
	m_uvd = uvd;
}

UVDStdInstructionIterator::~UVDStdInstructionIterator() {
}

uv_err_t UVDStdInstructionIterator::previous()
{
	//FIXME: 
	return UV_DEBUG(UV_ERR_GENERAL);
	/*
	This is a difficult problem if the instruction set is in any way variable length
	For now, lets assume a function will occur every so often
	Use that functions closest jump, call, etc points as an assembly reference
	
	Idea: find the first known location and forward disassemble until we hit our current address
	Take the previous location as our previous
	Known locations should include functions and vectors at a minimum
		Store jump information?
	
	Consider accelerating this for fixed length instruction sets like MIPS?
		Is it worth it?
	*/

	UVDAddress previousKnownAddress;
	UVDStdInstructionIterator forwardIterator;
	UVDAddress ourAddress;
	UVDAddress lastAddress;
	uv_err_t rcTemp = UV_ERR_GENERAL;
	
	//Return UV_ERR_DONE if there are none, but guess its an error if we get this
	//Looping to end() seems like a bad idea
	rcTemp = m_uvd->m_analyzer->getPreviousKnownInstructionAddress(m_address, &previousKnownAddress);
	uv_assert_err_ret(rcTemp);
	uv_assert_ret(previousKnownAddress.m_addr < m_address.m_addr);

	//Now form an iterator and forward assemble until we hit this
	uv_assert_err_ret(m_uvd->instructionBeginByAddress(previousKnownAddress, forwardIterator));
	uv_assert_err_ret(getPosition(&ourAddress));
	
	for( ;; )
	{
		UVDAddress curAddress;
		
		//Advance
		uv_assert_err_ret(forwardIterator.getPosition(&lastAddress));
		uv_assert_err_ret(forwardIterator.next());
		uv_assert_err_ret(forwardIterator.getPosition(&curAddress));

		//If we past our address, means we have inconsistent disassembly
		//Maybe should set an option for best guess?
		if( curAddress > ourAddress )
		{
			printf_error("instruction alignment inconsistent\n");
			printf_error("start: 0x%%08X, died at: 0x%08X, iter's: 0x%08X\n",
					curAddress.m_addr, m_address.m_addr);
			return UV_DEBUG(UV_ERR_GENERAL);
		}
		//Done?
		if( curAddress == ourAddress )
		{
			break;
		}
	}
	
	//The previous address should be what we need
	m_address.m_addr = lastAddress;
	//Parse current
	uv_assert_err_ret(parseCurrentInstruction());

	//yay we're done
	return UV_ERR_OK;
}
#endif

/*
uv_err_t UVDStdInstructionIterator::copy(UVDStdInstructionIterator *other) {
	return UV_ERR_NOTSUPPORTED;
}
*/

/*
Deprecated, use UVDInstructionIterator wrapper

UVDStdInstructionIterator UVDStdInstructionIterator::operator++()
{
	UV_DEBUG(next());
	return *this;
}

bool UVDStdInstructionIterator::operator==(const UVDStdInstructionIterator &other) const
{	
	//printf_debug("UVDStdInstructionIterator::operator==:\n");
	return compare(other) == 0;
}

bool UVDStdInstructionIterator::operator!=(const UVDStdInstructionIterator &other) const
{
	return !operator==(other);
}
*/

uv_err_t UVDInstructionIterator::check() {
	uv_assert_ret(m_iter);
	return UV_DEBUG(m_iter->check());
}

uv_err_t UVDAbstractInstructionIterator::check() {
	return UV_ERR_OK;
}

