/*
UVNet Universal Decompiler (uvudec)
Copyright 2011 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/architecture/architecture.h"
#include "uvd/architecture/std_iter_factory.h"
#include "uvd/core/std_iterator.h"
#include "uvd/core/uvd.h"
#include "uvd/core/runtime.h"

/*
UVDStdInstructionIteratorFactory
*/

UVDStdInstructionIteratorFactory::UVDStdInstructionIteratorFactory() {
}

UVDStdInstructionIteratorFactory::~UVDStdInstructionIteratorFactory() {
}

uv_err_t UVDStdInstructionIteratorFactory::abstractInstructionIteratorBeginByAddress( UVDAbstractInstructionIterator **out, UVDAddress address ) {
	UVDStdInstructionIterator *iter = NULL;
	
	iter = new UVDStdInstructionIterator();
	uv_assert_ret(iter);
	uv_assert_ret(address.m_space);
	uv_assert_err_ret(iter->init(g_uvd, address));
	uv_assert_err_ret(iter->check());
	uv_assert_ret(out);
	*out = iter;
 	
 	return UV_ERR_OK;
}

uv_err_t UVDStdInstructionIteratorFactory::abstractInstructionIteratorEndByAddressSpace(UVDAbstractInstructionIterator **out, UVDAddressSpace *addressSpace) {
	return UV_DEBUG(UVDStdInstructionIterator::getEnd(g_uvd, addressSpace, (UVDStdInstructionIterator **)out));
}

/*
UVDStdPrintIteratorFactory
*/

UVDStdPrintIteratorFactory::UVDStdPrintIteratorFactory() {
}

UVDStdPrintIteratorFactory::~UVDStdPrintIteratorFactory() {
}

uv_err_t UVDStdPrintIteratorFactory::abstractPrintIteratorBeginByAddress( UVDAbstractPrintIterator **out, UVDAddress address ) {
	UVDStdPrintIterator *iter = NULL;
	
	iter = new UVDStdPrintIterator();
	uv_assert_ret(iter);
	uv_assert_err_ret(iter->init(g_uvd, address, 0));
	uv_assert_err_ret(iter->check());
	uv_assert_ret(out);
	*out = iter;
 	
 	return UV_ERR_OK;
}

uv_err_t UVDStdPrintIteratorFactory::abstractPrintIteratorEndByAddressSpace(UVDAbstractPrintIterator **out, UVDAddressSpace *addressSpace) {
	return UV_DEBUG(UVDStdPrintIterator::getEnd(g_uvd, addressSpace, (UVDStdPrintIterator **)out));
}

