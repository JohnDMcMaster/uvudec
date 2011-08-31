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

UVDStdInstructionIterator::UVDStdInstructionIterator(UVD *uvd)
{
	if (uvd == NULL) {
		uvd = g_uvd;
	}
	m_uvd = uvd;
	
	m_addressSpacesIndex = 0;
	//m_iter = NULL;
}

UVDStdInstructionIterator::~UVDStdInstructionIterator()
{
	UV_DEBUG(deinit());
}

uv_err_t UVDStdInstructionIterator::init(UVD *uvd, UVDAddressSpace *addressSpace)
{
	uv_addr_t minAddress = 0;
	
	uv_assert_ret(uvd);
	uv_assert_ret(addressSpace);	
	uv_assert_err_ret(addressSpace->getMinValidAddress(&minAddress));
	
	return UV_DEBUG(init(uvd, UVDAddress(minAddress, addressSpace)));
}

uv_err_t UVDStdInstructionIterator::init(UVD *uvd, UVDAddress address)
{
	m_addressSpaces.push_back(address.m_space);
	
	m_addressSpacesIndex = 0;
	m_iter = UVDASInstructionIterator();
	uv_assert_ret(address.m_space);
	uv_assert_err_ret(m_iter.init(g_uvd, address));
	
	return UV_ERR_OK;
}

uv_err_t UVDStdInstructionIterator::beginningOfCurrentAddressSpace() {
	UVDAddress address;
	
	uv_assert_ret(m_addressSpacesIndex < m_addressSpaces.size());
	address.m_space = m_addressSpaces[m_addressSpacesIndex];
	uv_assert_ret(address.m_space);
	uv_assert_err_ret(address.m_space->getMinValidAddress(&address.m_addr));
		
	m_iter = UVDASInstructionIterator();
	uv_assert_err_ret(m_iter.init(g_uvd, address));
	
	return UV_ERR_OK;
}

uv_err_t UVDStdInstructionIterator::deinit()
{
	//m_addressSpace = NULL;
	m_uvd = NULL;
	return UV_ERR_OK;
}

/*
uv_err_t UVDStdInstructionIterator::makeEnd()
{
	uv_assert_err_ret(m_iter.makeEnd());
	return UV_ERR_OK;
}
*/

bool UVDStdInstructionIterator::isEnd()
{
	return ((m_addressSpacesIndex + 1) == m_addressSpaces.size()) && m_iter.isEnd();
}

/*
uv_addr_t UVDStdInstructionIterator::getPosition()
{
	return m_address.m_addr;
}
*/

uv_err_t UVDStdInstructionIterator::getPosition(UVDAddress *out) {
	uv_assert_ret(out);
	*out = m_iter.m_address;
	return UV_ERR_OK;
}

uv_err_t UVDStdInstructionIterator::get(UVDInstruction **out) const {
	uv_assert_ret(out);
	*out = m_iter.m_instruction;
	return UV_ERR_OK;
}


uv_err_t UVDStdInstructionIterator::previous()
{
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
	UVDInstructionIterator forwardIterator;
	UVDAddress thisAddress;
	uv_err_t rcTemp = UV_ERR_GENERAL;
	UVDAddress lastAddress;
	
	//uv_assert_err_ret(getAddress(&lastAddress));
	thisAddress = m_iter.m_address;
	
	//Return UV_ERR_DONE if there are none, but guess its an error if we get this
	//Looping to end() seems like a bad idea
	rcTemp = m_uvd->m_analyzer->getPreviousKnownInstructionAddress(m_iter.m_address, &previousKnownAddress);
	uv_assert_err_ret(rcTemp);
	uv_assert_ret(previousKnownAddress.m_addr < m_iter.m_address.m_addr);

	//Now form an iterator and forward assemble until we hit this
	uv_assert_err_ret(m_uvd->instructionBeginByAddress(previousKnownAddress, forwardIterator));
	
	for( ;; )
	{
		UVDAddress curAddress;
	
		//Advance
		//lastAddress = forwardIterator.m_address.m_addr;
		uv_assert_err_ret(forwardIterator.getAddress(&lastAddress));
		uv_assert_err_ret(forwardIterator.next());
		uv_assert_err_ret(forwardIterator.getAddress(&curAddress));

		//If we past our address, means we have inconsistent disassembly
		if( curAddress.m_addr > thisAddress.m_addr )
		{
			printf_error("instruction alignment inconsistent\n");
			printf_error("start: 0x%08X, died at: 0x%08X, iter's: 0x%08X\n",
					lastAddress.m_addr, curAddress.m_addr, thisAddress.m_addr);
			return UV_DEBUG(UV_ERR_GENERAL);
		}
		//Done?
		if( curAddress.m_addr == thisAddress.m_addr )
		{
			break;
		}
	}
	
	uv_assert_err_ret(setAddress(lastAddress));

	//yay we're done
	return UV_ERR_OK;
}

uv_err_t UVDStdInstructionIterator::setAddress(UVDAddress address) {
	for (m_addressSpacesIndex = 0; m_addressSpacesIndex < m_addressSpaces.size(); ++m_addressSpacesIndex) {
		UVDAddressSpace *addressSpace = m_addressSpaces[m_addressSpacesIndex];
		
		uv_assert_ret(addressSpace);
		if (addressSpace == address.m_space) {
			break;
		}
	}
	uv_assert_err_ret(m_addressSpacesIndex < m_addressSpaces.size());
	
	//The previous address should be what we need
	//m_address.m_addr = address;
	m_iter = UVDASInstructionIterator();
	uv_assert_err_ret(m_iter.init(m_uvd, address));
	return UV_ERR_OK;
}

uv_err_t UVDStdInstructionIterator::next()
{
	uv_err_t rcTemp = UV_ERR_GENERAL;

	//If already at end shouldn't try to advance
	uv_assert_ret(m_addressSpacesIndex < m_addressSpaces.size());
	while( true ) {
		//The first time when we iterated to end we will dwell
		//But we will not try to go past end
		if (!m_iter.isEnd()) {
			rcTemp = m_iter.next();
			//Dwell, don't error if done
			if (rcTemp != UV_ERR_DONE) {
				uv_assert_err_ret(rcTemp);
			}
			return UV_ERR_OK;
		}
		++m_addressSpacesIndex;
		//Make sure they aren't trying to advance past end
		uv_assert_ret(m_addressSpacesIndex < m_addressSpaces.size());
		
		//Not end, reset to the beginning of that address space
		uv_assert_err_ret(beginningOfCurrentAddressSpace());
	}
}


/*
uv_err_t UVDStdInstructionIterator::nextValidExecutableAddress()
{
	//XXX: should do an end check?
	//This may put us into an invalid area, but we will find the next valid if availible
	++m_address.m_addr;
	return UV_DEBUG(nextValidExecutableAddressIncludingCurrent());
}

uv_err_t UVDStdInstructionIterator::nextValidExecutableAddressIncludingCurrent()
{
	uv_err_t rcNextAddress = UV_ERR_GENERAL;
	
	//FIXME: look into considerations for an instruction split across areas, which probably doesn't make sense
	uv_assert_ret(m_address.m_space);
	rcNextAddress = m_address.m_space->nextValidExecutableAddress(m_address.m_addr, &m_address.m_addr);
	uv_assert_err_ret(rcNextAddress);
	if( rcNextAddress == UV_ERR_DONE )
	{
		//Don't do this, we might be partial through an instruction and don't want to mess up address
		//Don't try to advance further then
		//But we are in middle of decoding, so let caller figure out what to do with buffers
		//uv_assert_err_ret(makeNextEnd());
		//uv_assert_err_ret(makeEnd());
		return UV_ERR_DONE;
	}

	return UV_ERR_OK;
}

uv_err_t UVDStdInstructionIterator::consumeCurrentExecutableAddress(uint8_t *out)
{
	//Current address should always be valid unless we are at end()
	//This is not a hard error because it can happen from malformed opcodes in the input
	if( isEnd() )
	{
		return UV_ERR_DONE;
	}

	uv_assert_err_ret(m_address.m_space->m_data->readData(m_address.m_addr, (char *)out));	
	++m_currentSize;
	//We don't care if next address leads to end
	//Current address was valid and it is up to next call to return done if required
	uv_assert_err_ret(nextValidExecutableAddress());
	
	return UV_ERR_OK;
}

uv_err_t UVDStdInstructionIterator::addWarning(const std::string &lineRaw)
{
	printf_warn("%s\n", lineRaw.c_str());
	return UV_ERR_OK;
}
uv_err_t UVDStdInstructionIterator::parseCurrentInstruction()
{
	return UV_DEBUG(m_uvd->m_runtime->m_architecture->parseCurrentInstruction(*this));
}	

UVDStdInstructionIterator UVDStdInstructionIterator::operator++()
{
	next();
	return *this;
}

bool UVDStdInstructionIterator::operator==(const UVDStdInstructionIterator &other) const
{	
	//printf_debug("UVDStdInstructionIterator::operator==:\n");
	return m_address.m_addr == other.m_address.m_addr;
}

bool UVDStdInstructionIterator::operator!=(const UVDStdInstructionIterator &other) const
{
	return !operator==(other);
}
*/

int UVDStdInstructionIterator::compare(const UVDAbstractInstructionIterator &otherIn) const
{
	//int temp = 0;
	const UVDStdInstructionIterator *other = dynamic_cast<const UVDStdInstructionIterator *>(&otherIn);

	uv_assert_ret(other);
	//AS iterator will compare address spaces
	/*
	uv_assert_ret(m_addressSpacesIndex < m_addressSpaces.size());
	uv_assert_ret(other->m_addressSpacesIndex < other->m_addressSpaces.size());
	temp = m_addressSpaces[m_addressSpacesIndex] - other->m_addressSpaces[other->m_addressSpacesIndex];
	printf("this index: %d, other: %d, AS diff: %d\n", m_addressSpacesIndex, other->m_addressSpacesIndex, temp);
	if (temp) {
		return temp;
	}
	*/
	return m_iter.compare(other->m_iter);
}

uv_err_t UVDStdInstructionIterator::copy(UVDAbstractInstructionIterator **out) const {
	UVDStdInstructionIterator *ret = NULL;
	
	ret = new UVDStdInstructionIterator();
	uv_assert_ret(ret);
	//nothing special needed yet
	*ret = *this;
	uv_assert_ret(out);
	*out = ret;

	return UV_ERR_OK;
}

uv_err_t UVDStdInstructionIterator::getEnd(UVD *uvd, UVDAddressSpace *addressSpace, UVDStdInstructionIterator **out) {
	UVDStdInstructionIterator *iter = NULL;
	
	uv_assert_ret(addressSpace);
	
	uv_assert_ret(uvd);
	iter = new UVDStdInstructionIterator();
	uv_assert_ret(iter);
	iter->m_addressSpaces.push_back(addressSpace);
	iter->m_addressSpacesIndex = 0;
	iter->m_uvd = uvd;
	
	uv_assert_err_ret(UVDASInstructionIterator::getEnd(uvd, addressSpace, iter->m_iter));
	uv_assert_ret(out);
	*out = iter;
	
	return UV_ERR_OK;
}

/*
uv_err_t UVDStdInstructionIterator::getEndFromExisting(UVD *uvd, UVDAddressSpace *addressSpace, UVDStdInstructionIterator *iter)
{
	uv_assert_ret(iter);
	iter->m_uvd = uvd;
	
	uv_assert_ret(addressSpace);
	
	//regardless we must specify the end of a particular address space
	iter->m_addressSpaces.push_back(addressSpace);
	
	//End of a particular address space
	iter->m_addressSpacesIndex = 0;
	uv_assert_err_ret(UVDASInstructionIterator::getEnd(uvd, iter->m_iter));
	return UV_ERR_OK;
}
*/

uv_err_t UVDStdInstructionIterator::check() {
	uv_assert_ret(!m_addressSpaces.empty());
	uv_assert_ret(m_addressSpacesIndex < m_addressSpaces.size());
	uv_assert_ret(m_uvd);
	return UV_DEBUG(m_iter.check());
}

