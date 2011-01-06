/*
UVNet Universal Decompiler (uvudec)
Copyright 2011 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvqt/simple_dynamic_text.h"
#include "uvd/util/debug.h"
#include <math.h>

/*
UVQtSimpleDynamicTextData
*/

UVQtSimpleDynamicTextData::UVQtSimpleDynamicTextData()
{
	m_minOffset = 0;
	m_maxOffset = 0;
}

uv_err_t UVQtSimpleDynamicTextData::begin(unsigned int offset, unsigned int index, UVQtDynamicTextData::iterator *out)
{
	UVQtSimpleDynamicTextData::iterator_impl *iter_impl = getIterator();
	iter_impl->m_offset = offset;
	iter_impl->m_index = index;
	UVQtDynamicTextData::iterator iter = UVQtDynamicTextData::iterator(iter_impl);
	*out = iter;
	return UV_ERR_OK;
}

uv_err_t UVQtSimpleDynamicTextData::end(iterator *out)
{
	unsigned int maxOffset = 0;
	
	uv_assert_err_ret(getMaxOffset(&maxOffset);

	UVQtSimpleDynamicTextData::iterator_impl *iter_impl = getIterator();
	iter_impl->m_offset = maxOffset + 1;
	iter_impl->m_index = 0;
	UVQtDynamicTextData::iterator iter = UVQtDynamicTextData::iterator(iter_impl);
	*out = iter;
	return UV_ERR_OK;
}

uv_err_t UVQtSimpleDynamicTextData::getMinOffset(unsigned int *out)
{
	*out = m_minOffset;
	return UV_ERR_OK;
}

uv_err_t UVQtSimpleDynamicTextData::getMaxOffset(unsigned int *out)
{
	*out = m_maxOffset;
	return UV_ERR_OK;
}

