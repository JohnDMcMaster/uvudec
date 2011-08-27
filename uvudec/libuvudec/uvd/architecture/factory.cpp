/*
UVNet Universal Decompiler (uvudec)
Copyright 2011 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/architecture/architecture.h"
#include "uvd/core/std_iterator.h"
#include "uvd/core/uvd.h"
#include "uvd/core/runtime.h"

/*
UVDInstructionIteratorFactory
*/

UVDInstructionIteratorFactory::UVDInstructionIteratorFactory() {
}

UVDInstructionIteratorFactory::~UVDInstructionIteratorFactory() {
}

uv_err_t UVDInstructionIteratorFactory::instructionIteratorBegin( UVDInstructionIterator *out ) {
	uv_assert_ret(out);
	uv_assert_err_ret(abstractInstructionIteratorBegin(&out->m_iter));
	uv_assert_ret(out->m_iter);
	
	return UV_ERR_OK;
}

uv_err_t UVDInstructionIteratorFactory::abstractInstructionIteratorBegin( UVDAbstractInstructionIterator **out ) {
	//Find first address space
	UVDAddress address;
	UVDArchitecture *architecture = NULL;
	
	//Find the lowest address and address space
	architecture = g_uvd->m_runtime->m_architecture;
	uv_assert_ret(!architecture->m_addressSpaces.m_addressSpaces.empty());
	address.m_space = architecture->m_addressSpaces.m_addressSpaces[0];
	uv_assert_err_ret(address.m_space->getMinValidAddress(&address.m_addr));

	uv_assert_err_ret(abstractInstructionIteratorBeginByAddress(out, address));
	
	return UV_ERR_OK;
}

uv_err_t UVDInstructionIteratorFactory::instructionIteratorBeginByAddress( UVDInstructionIterator *out, UVDAddress address ) {
	uv_assert_ret(out);
	uv_assert_err_ret(abstractInstructionIteratorBeginByAddress(&out->m_iter, address));
	uv_assert_ret(out->m_iter);
	
	return UV_ERR_OK;
}

uv_err_t UVDInstructionIteratorFactory::abstractInstructionIteratorBeginByAddress( UVDAbstractInstructionIterator **out, UVDAddress address ) {
	UVDStdInstructionIterator *iter = NULL;
	
	iter = new UVDStdInstructionIterator();
	uv_assert_ret(iter);
	uv_assert_err_ret(iter->init(g_uvd, address));
	uv_assert_ret(out);
	*out = iter;
 	
 	return UV_ERR_OK;
}

uv_err_t UVDInstructionIteratorFactory::instructionIteratorEnd( UVDInstructionIterator *out ) {
	uv_assert_ret(out);
	uv_assert_err_ret(abstractInstructionIteratorEnd(&out->m_iter));
	uv_assert_ret(out->m_iter);
	
	return UV_ERR_OK;
}

uv_err_t UVDInstructionIteratorFactory::abstractInstructionIteratorEnd(UVDAbstractInstructionIterator **out) {
	UVDStdInstructionIterator *iter = NULL;
	
	iter = new UVDStdInstructionIterator();
	uv_assert_ret(iter);
	uv_assert_err_ret(UVDStdInstructionIterator::getEnd(g_uvd, NULL, &iter));
	uv_assert_ret(out);
	*out = iter;
 	
 	return UV_ERR_OK;
}

uv_err_t UVDInstructionIteratorFactory::instructionIteratorEndByAddressSpace( UVDInstructionIterator *out, UVDAddressSpace *addressSpace ) {
	uv_assert_ret(out);
	uv_assert_err_ret(abstractInstructionIteratorEndByAddressSpace(&out->m_iter, addressSpace));
	uv_assert_ret(&out->m_iter);
	
	return UV_ERR_OK;
}

uv_err_t UVDInstructionIteratorFactory::abstractInstructionIteratorEndByAddressSpace(UVDAbstractInstructionIterator **out, UVDAddressSpace *addressSpace) {
	return UV_DEBUG(UVDStdInstructionIterator::getEnd(g_uvd, addressSpace, (UVDStdInstructionIterator **)out));
}








/*
UVDPrintIteratorFactory
*/

UVDPrintIteratorFactory::UVDPrintIteratorFactory() {
}

UVDPrintIteratorFactory::~UVDPrintIteratorFactory() {
}

uv_err_t UVDPrintIteratorFactory::printIteratorBegin( UVDPrintIterator *out ) {
	uv_assert_ret(out);
	uv_assert_err_ret(abstractPrintIteratorBegin(&out->m_iter));
	uv_assert_ret(out->m_iter);
	
	return UV_ERR_OK;
}

uv_err_t UVDPrintIteratorFactory::abstractPrintIteratorBegin( UVDAbstractPrintIterator **out ) {
	//Find first address space
	UVDAddress address;
	UVDArchitecture *architecture = NULL;
	
	//Find the lowest address and address space
	architecture = g_uvd->m_runtime->m_architecture;
	uv_assert_ret(!architecture->m_addressSpaces.m_addressSpaces.empty());
	address.m_space = architecture->m_addressSpaces.m_addressSpaces[0];
	uv_assert_err_ret(address.m_space->getMinValidAddress(&address.m_addr));

	uv_assert_err_ret(abstractPrintIteratorBeginByAddress(out, address));
	
	return UV_ERR_OK;
}

uv_err_t UVDPrintIteratorFactory::printIteratorBeginByAddress( UVDPrintIterator *out, UVDAddress address ) {
	uv_assert_ret(out);
	uv_assert_err_ret(abstractPrintIteratorBeginByAddress(&out->m_iter, address));
	uv_assert_ret(out->m_iter);
	
	return UV_ERR_OK;
}

uv_err_t UVDPrintIteratorFactory::abstractPrintIteratorBeginByAddress( UVDAbstractPrintIterator **out, UVDAddress address ) {
	UVDStdPrintIterator *iter = NULL;
	
	iter = new UVDStdPrintIterator();
	uv_assert_ret(iter);
	uv_assert_err_ret(iter->init(g_uvd, address, 0));
	uv_assert_ret(out);
	*out = iter;
 	
 	return UV_ERR_OK;
}

uv_err_t UVDPrintIteratorFactory::printIteratorEnd( UVDPrintIterator *out ) {
	uv_assert_ret(out);
	uv_assert_err_ret(abstractPrintIteratorEnd(&out->m_iter));
	uv_assert_ret(out->m_iter);
	
	return UV_ERR_OK;
}

uv_err_t UVDPrintIteratorFactory::abstractPrintIteratorEnd(UVDAbstractPrintIterator **out) {
	UVDStdPrintIterator *iter = NULL;
	
	iter = new UVDStdPrintIterator();
	uv_assert_ret(iter);
	uv_assert_err_ret(UVDStdPrintIterator::getEnd(g_uvd, NULL, &iter));
	uv_assert_ret(out);
	*out = iter;
 	
 	return UV_ERR_OK;
}

uv_err_t UVDPrintIteratorFactory::printIteratorEndByAddressSpace( UVDPrintIterator *out, UVDAddressSpace *addressSpace ) {
	uv_assert_ret(out);
	uv_assert_err_ret(abstractPrintIteratorEndByAddressSpace(&out->m_iter, addressSpace));
	uv_assert_ret(&out->m_iter);
	
	return UV_ERR_OK;
}

uv_err_t UVDPrintIteratorFactory::abstractPrintIteratorEndByAddressSpace(UVDAbstractPrintIterator **out, UVDAddressSpace *addressSpace) {
	return UV_DEBUG(UVDStdPrintIterator::getEnd(g_uvd, addressSpace, (UVDStdPrintIterator **)out));
}

