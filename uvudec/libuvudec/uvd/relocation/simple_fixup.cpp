/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/relocation/relocation.h"

UVDSimpleRelocationFixup::UVDSimpleRelocationFixup()
{
	m_relocationFixup = NULL;
	m_data = NULL;
}

UVDSimpleRelocationFixup::~UVDSimpleRelocationFixup()
{
	deinit();
}

uv_err_t UVDSimpleRelocationFixup::deinit()
{
	delete m_relocationFixup;
	m_relocationFixup = NULL;
	
	return UV_ERR_OK;
}

uv_err_t UVDSimpleRelocationFixup::getUVDSimpleRelocationFixup(
		UVDSimpleRelocationFixup **simpleFixupOut, UVDRelocatableElement *relocatableElement,
		char *data, int size)
{
	UVDSimpleRelocationFixup *simpleFixup = NULL;
	UVDRelocationFixup *relocationFixup = NULL;
	//Apply at location
	int offset = 0;
	
	uv_assert_ret(relocatableElement);
	
	simpleFixup = new UVDSimpleRelocationFixup();
	uv_assert_ret(simpleFixup);

	relocationFixup = new UVDRelocationFixup(relocatableElement, offset, size);
	uv_assert_ret(relocationFixup);
	simpleFixup->m_relocationFixup = relocationFixup;
	
	//We need to transfer the buffer (as opposed to copy contents) since it needs to be applied to the real data
	uv_assert_err_ret(UVDDataMemory::getUVDDataMemoryByTransfer((UVDDataMemory **)&simpleFixup->m_data,
			//We are just applying patches, don't try to free the memory
			data, size, false));
	uv_assert_ret(simpleFixup->m_data);
	
	uv_assert_ret(simpleFixupOut);
	*simpleFixupOut = simpleFixup;
	return UV_ERR_OK;
}

uv_err_t UVDSimpleRelocationFixup::applyPatch()
{
	uv_assert_ret(m_relocationFixup);
	uv_assert_err_ret(m_relocationFixup->applyPatch(m_data));
	return UV_ERR_OK;
}
