/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/architecture/architecture.h"
#include "uvd/assembly/cpu_vector.h"
#include "uvd/core/uvd.h"
#include "uvd/core/std_iterator.h"

UVDArchitecture::UVDArchitecture()
{
	m_uvd = NULL;
}

UVDArchitecture::~UVDArchitecture()
{
	UV_DEBUG(deinit());
}

uv_err_t UVDArchitecture::init()
{
	uv_assert_err_ret(getInstructionIteratorFactory(&m_instructionIteratorFactory));
	uv_assert_err_ret(getPrintIteratorFactory(&m_printIteratorFactory));

	return UV_ERR_OK;
}

uv_err_t UVDArchitecture::deinit()
{
	for( std::vector<UVDCPUVector *>::iterator iter = m_vectors.begin();
			iter != m_vectors.end(); ++iter )
	{
		delete *iter;
	}

	return UV_ERR_OK;
}

uv_err_t UVDArchitecture::getInstructionIteratorFactory(UVDInstructionIteratorFactory **out) {
	*out = new UVDInstructionIteratorFactory();
	return UV_ERR_OK;
}

uv_err_t UVDArchitecture::getPrintIteratorFactory(UVDPrintIteratorFactory **out) {
	*out = new UVDPrintIteratorFactory();
	return UV_ERR_OK;
}

uv_err_t UVDArchitecture::readByte(UVDAddress address, uint8_t *out)
{
	//uv_assert_ret(m_uvd);
	//uv_assert_ret(m_uvd->m_runtime->m_object);
	uv_assert_err_ret(address.m_space->m_data->readData(address.m_addr, out));
	
	return UV_ERR_OK;
}
uv_err_t UVDArchitecture::fixupDefaults()
{
	if( m_vectors.empty() )
	{
		UVDCPUVector *vector = NULL;
	
		vector = new UVDCPUVector();
		uv_assert_ret(vector);
		vector->m_name = "start";
		vector->m_description = "auto added";
		vector->m_offset = 0;

		m_vectors.push_back(vector);
	}
	
	return UV_ERR_OK;
}

#if 0

/*
uv_err_t UVDArchitecture::getInstructionIterator( UVDInstructionIterator **out, UVDAddress address ) {
	uv_assert_ret(out);
	*out = new UVDInstructionIterator();
	uv_assert_ret(*out);
	uv_assert_err_ret((*out)->init(g_uvd, address));

	return UV_ERR_OK;
}
*/

uv_err_t UVDArchitecture::getInstructionIterator( UVDInstructionIterator *out, UVDAddress address ) {
	UVDAbstractInstructionIterator *inner = NULL;
	
	uv_assert_err_ret(getInstructionIterator(&inner, address));
	uv_assert_ret(out);
	*out = UVDInstructionIterator(inner);
	return UV_ERR_OK;
}
#endif

/*
uv_err_t UVDArchitecture::instructionIteratorBegin( UVDInstructionIterator *out ) {
	uv_assert_err_ret(abstractInstructionIteratorBegin(&out->m_iter));
	return UV_ERR_OK;
}

uv_err_t UVDArchitecture::instructionIteratorBeginByAddress( UVDInstructionIterator *out, UVDAddress address ) {
	uv_assert_err_ret(abstractInstructionIteratorBeginByAddress(&out->m_iter, address));
	return UV_ERR_OK;
}

uv_err_t UVDArchitecture::instructionIteratorEnd( UVDInstructionIterator *out) {
	uv_assert_err_ret(abstractInstructionIteratorEnd(&out->m_iter));
	return UV_ERR_OK;
}

uv_err_t UVDArchitecture::instructionIteratorEndByAddressSpace( UVDInstructionIterator *out, UVDAddressSpace addressSpace ) {
	uv_assert_err_ret(abstractInstructionIteratorEndByAddressSpace(&out->m_iter, addressSpace));
	return UV_ERR_OK;
}

uv_err_t UVDArchitecture::printIteratorBegin( UVDPrintIterator *out ) {
	uv_assert_err_ret(abstractPrintIteratorBegin( &out->m_iter));
	return UV_ERR_OK;
}

uv_err_t UVDArchitecture::printIteratorBeginByAddress( UVDPrintIterator *out, UVDAddress address ) {
	uv_assert_err_ret(abstractPrintIteratorBeginByAddress( &out->m_iter, address));
	return UV_ERR_OK;
}

uv_err_t UVDArchitecture::printIteratorEnd( UVDPrintIterator *out ) {
	uv_assert_err_ret(abstractPrintIteratorEnd(&out->m_iter));
	return UV_ERR_OK;
}

uv_err_t UVDArchitecture::printIteratorEndByAddressSpace( UVDPrintIterator *out, UVDAddressSpace addressSpace ) {
	uv_assert_err_ret(abstractPrintIteratorEndByAddressSpace(&out->m_iter, addressSpace));
	return UV_ERR_OK;
}
*/

uv_err_t UVDArchitecture::parseCurrentInstruction(UVDASInstructionIterator &iter) {
	//Implement if you want to use std iterator
	return UV_ERR_NOTSUPPORTED;
}

