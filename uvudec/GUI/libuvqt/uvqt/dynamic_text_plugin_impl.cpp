/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the GPL V3 or later, see COPYING for details
*/

#include "uvqt/dynamic_text_plugin_impl.h"
#include "uvd/util/debug.h"

/*
UVQtDynamicTextDataPluginImpl::iterator_impl
*/

UVQtDynamicTextDataPluginImpl::iterator_impl::iterator_impl()
{
	//printf("new blank UVQtDynamicTextDataPluginImpl = 0x%08X\n", (int)this);
	m_dataImpl = NULL;
	m_offset = 0;
	m_index = 0;
}

UVQtDynamicTextDataPluginImpl::iterator_impl::iterator_impl(UVQtDynamicTextDataPluginImpl *impl, unsigned int offset, unsigned int index)
{
	//printf("new UVQtDynamicTextDataPluginImpl = 0x%08X\n", (int)this);
	m_dataImpl = impl;
	/*
	if( m_dataImpl == NULL )
	{
	printf("bad data impl\n");
	//UVD_PRINT_STACK();
	}
	*/
	m_offset = offset;
	m_index = index;
}

UVQtDynamicTextDataPluginImpl::iterator_impl::~iterator_impl()
{
	//printf("killing UVQtDynamicTextDataPluginImpl::iterator_impl = 0x%08X\n", (int)this);
	//UVD_PRINT_STACK();
}

UVQtDynamicTextData::iterator_impl *UVQtDynamicTextDataPluginImpl::iterator_impl::copy()
{
	UVQtDynamicTextDataPluginImpl::iterator_impl *ret = new UVQtDynamicTextDataPluginImpl::iterator_impl();
	*ret = *this;
	//printf("copy with %s\n", toString().c_str());
	return ret;
}

std::string UVQtDynamicTextDataPluginImpl::iterator_impl::toString()
{
	return UVDSprintf("m_offset=0x%08X, m_index=0x%08X", m_offset, m_index);
}

unsigned int UVQtDynamicTextDataPluginImpl::iterator_impl::offset()
{
	return m_offset;
}

uv_err_t UVQtDynamicTextDataPluginImpl::iterator_impl::get(std::string &ret)
{
	ret = UVDSprintf("%d:%d", m_offset, m_index);
	return UV_ERR_OK;
}

uv_err_t UVQtDynamicTextDataPluginImpl::iterator_impl::previous()
{
	uv_assert_ret(this);
	UVD_POKE_THIS();
	UVDPoke((uint8_t *)&m_index, sizeof(m_index));
	UVD_POKE(m_dataImpl);
	uv_assert_ret(m_dataImpl);
	//UVD_POKE(m_dataImpl->m_numberOffsetIndexes);
	//If we went out of bounds somehow, perform basic error correction
	if( m_index >= m_dataImpl->m_numberOffsetIndexes )
	{
		printf("WARNING: index out of bounds\n");
		m_index = m_dataImpl->m_numberOffsetIndexes - 1;
	}
	if( m_offset > m_dataImpl->m_offsetMax )
	{
		printf("WARNING: offset above max\n");
		m_offset = m_dataImpl->m_offsetMax;
	}
	if( m_offset < m_dataImpl->m_offsetMin )
	{
		printf("WARNING: offset below min\n");
		m_offset = m_dataImpl->m_offsetMin;
	}

	if( m_index > 0 )
	{
		printf("previous: adjusting m_index\n");
		--m_index;
	}
	//No more room?
	else if( m_offset <= m_dataImpl->m_offsetMin )
	{
		return UV_ERR_OK;
	}
	else
	{
		printf("previous: adjusting m_offset\n");
		--m_offset;
		m_index = m_dataImpl->m_numberOffsetIndexes - 1;
	}

	return UV_ERR_OK;
}
	
uv_err_t UVQtDynamicTextDataPluginImpl::iterator_impl::next()
{
	uv_assert_ret(this);
	//If we went out of bounds somehow, perform basic error correction
	if( m_index >= m_dataImpl->m_numberOffsetIndexes )
	{
		printf("WARNING: index out of bounds\n");
		m_index = m_dataImpl->m_numberOffsetIndexes - 1;
	}
	if( m_offset > m_dataImpl->m_offsetMax )
	{
		printf("WARNING: offset above max\n");
		m_offset = m_dataImpl->m_offsetMax;
	}
	if( m_offset < m_dataImpl->m_offsetMin )
	{
		printf("WARNING: offset below min\n");
		m_offset = m_dataImpl->m_offsetMin;
	}

	if( m_index < m_dataImpl->m_numberOffsetIndexes - 1 )
	{
		++m_index;
	}
	/*
	this would make us not advance to end
	//No more room?
	else if( m_offset >= m_dataImpl->m_offsetMax )
	{
		return UV_ERR_OK;
	}
	*/
	else
	{
		++m_offset;
		m_index = 0;
	}

	return UV_ERR_OK;
}

uv_err_t UVQtDynamicTextDataPluginImpl::iterator_impl::changePositionByDelta(int delta)
{
	if( delta > 0 )
	{
		for( int i = 0; i < delta; ++i )
		{
			//About to go out of bounds?
			if( m_offset == m_dataImpl->m_offsetMax && m_index == m_dataImpl->m_numberOffsetIndexes - 1 )
			{
				break;
			}
			uv_assert_err_ret(next());
		}
	}
	else
	{
		for( int i = 0; i > delta; --i )
		{
			//About to go out of bounds?
			if( m_offset == m_dataImpl->m_offsetMin && m_index == 0 )
			{
				break;
			}
			uv_assert_err_ret(previous());
		}
	}
	return UV_ERR_OK;
}

uv_err_t UVQtDynamicTextDataPluginImpl::iterator_impl::changePositionToAbsolute(unsigned int offset, unsigned int index)
{
	m_offset = offset;
	m_index = index;
	return UV_ERR_OK;
}

int UVQtDynamicTextDataPluginImpl::iterator_impl::compare(const UVQtDynamicTextData::iterator_impl *otherIn)
{
	//Should be of this type
	const iterator_impl *other = static_cast<const iterator_impl *>(otherIn);
	int delta = 0;
	
	delta = m_offset - other->m_offset;
	if( delta )
	{
		return delta;
	}

	delta = m_index - other->m_index;
	if( delta )
	{
		return delta;
	}

	return 0;	
}

/*
UVQtDynamicTextDataPluginImpl
*/

UVQtDynamicTextDataPluginImpl::UVQtDynamicTextDataPluginImpl()
{
	m_offsetMin = 1;
	m_offsetMax = 20;
	m_numberOffsetIndexes = 3;
}

uv_err_t UVQtDynamicTextDataPluginImpl::begin(unsigned int offset, unsigned int index, UVQtDynamicTextData::iterator *out)
{
	/*
	if( this == NULL )
	{
	printf("bad this\n");
	}
	printf("this: 0x%08X\n", (int)this);
	*/
	//printf("begin stuff\n");
	//fflush(stdout);
	UVQtDynamicTextDataPluginImpl::iterator_impl *iter_impl = new UVQtDynamicTextDataPluginImpl::iterator_impl(this, offset, index);
	//printf("data m_dataImpl val 0x%08X\n", (int)iter_impl->m_dataImpl);
	UVQtDynamicTextData::iterator iter = UVQtDynamicTextData::iterator(iter_impl);
	//printf("copy operation\n");
	//printf("iter m_dataImpl: 0x%08X\n", (int)iter.m_dataImpl);
	*out = iter;
	return UV_ERR_OK;
}

uv_err_t UVQtDynamicTextDataPluginImpl::end(iterator *out)
{
	UVQtDynamicTextDataPluginImpl::iterator_impl *iter_impl = new UVQtDynamicTextDataPluginImpl::iterator_impl(this, m_offsetMax + 1, 0);
	UVQtDynamicTextData::iterator iter = UVQtDynamicTextData::iterator(iter_impl);
	*out = iter;
	return UV_ERR_OK;
}
	
uv_err_t UVQtDynamicTextDataPluginImpl::getMinOffset(unsigned int *out)
{
	*out = m_offsetMin;
	return UV_ERR_OK;
}

uv_err_t UVQtDynamicTextDataPluginImpl::getMaxOffset(unsigned int *out)
{
	*out = m_offsetMax;
	return UV_ERR_OK;
}

