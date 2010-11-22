/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/core/uvd.h"
#include "uvd/assembly/function.h"
#include "uvd/language/language.h"
#include "uvd/hash/md5.h"
#include "uvd/util/util.h"
#include "uvd/core/runtime.h"

uv_err_t UVDBinaryFunction::setData(UVDData *data)
{
	delete m_data;
	uv_assert_ret(data);
	uv_assert_err_ret(data->deepCopy(&m_data));

	return UV_ERR_OK;
}

uv_err_t UVDBinaryFunction::transferData(UVDData *data)
{
	delete m_data;
	m_data = data;

	return UV_ERR_OK;
}

UVDData *UVDBinaryFunction::getData()
{
	return m_data;
}

/*
During the symbol mapping process, the symbol names remain the same, they are the PK link on both sides
*/

/*
UVDBinaryFunction
*/

UVDBinaryFunction::UVDBinaryFunction()
{
	m_data = NULL;
	m_uvd = NULL;
	m_offset = 0;
}

UVDBinaryFunction::~UVDBinaryFunction()
{
	deinit();
}

uv_err_t UVDBinaryFunction::deinit()
{
	delete m_data;
	m_data = NULL;
	return UV_ERR_OK;
}

uv_err_t UVDBinaryFunction::getMin(uint32_t *out)
{
	uv_assert_ret(out);
	*out = m_offset;
	return UV_ERR_OK;
}

uv_err_t UVDBinaryFunction::getMax(uint32_t *out)
{
	uv_assert_ret(m_data);
	uv_assert_ret(out);
	uv_assert_ret(m_data->size(out));
	*out += m_offset;
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

