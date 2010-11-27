/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/core/uvd.h"
#include "uvd/assembly/function.h"
#include "uvd/util/util.h"

/*
During the symbol mapping process, the symbol names remain the same, they are the PK link on both sides
*/

/*
UVDBinaryFunction
*/

UVDBinaryFunction::UVDBinaryFunction()
{
}

UVDBinaryFunction::~UVDBinaryFunction()
{
	deinit();
}

uv_err_t UVDBinaryFunction::deinit()
{
	return UV_ERR_OK;
}

uv_err_t UVDBinaryFunction::getMin(uint32_t *out)
{
	uv_addr_t offset = 0;

	uv_assert_err_ret(getSymbolAddress(&offset));

	uv_assert_ret(out);
	*out = offset;
	return UV_ERR_OK;
}

uv_err_t UVDBinaryFunction::getMax(uint32_t *out)
{
	uv_addr_t minAddress = 0;
	
	uv_assert_ret(out);
	uv_assert_ret(m_relocatableData.size(out));
	uv_assert_err_ret(getMin(&minAddress));
	*out += minAddress;

	return UV_ERR_OK;
}

uv_err_t UVDBinaryFunction::getUVDBinaryFunctionInstance(UVDBinaryFunction **out)
{
	UVDBinaryFunction *instance = NULL;
	
	instance = new UVDBinaryFunction();
	uv_assert_ret(instance);
	
	uv_assert_err_ret(instance->init());

	uv_assert_ret(out);
	*out = instance;
	return UV_ERR_OK;
}

