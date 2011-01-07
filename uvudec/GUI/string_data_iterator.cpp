/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "GUI/GUI.h"
#include "GUI/string_data.h"
#include "uvd/string/engine.h"
#include "uvd/util/debug.h"

/*
UVDGUIStringData::iterator_impl
*/

UVDGUIStringData::iterator_impl::iterator_impl()
{
	m_offset = 0;
	m_index = 0;
	m_dataImpl = NULL;
}

UVDGUIStringData::iterator_impl::iterator_impl(UVDGUIStringData *impl, unsigned int offset, unsigned int index)
{
	m_dataImpl = impl;
	m_offset = offset;
	m_index = index;
}

UVDGUIStringData::iterator_impl::~iterator_impl()
{
}

UVQtDynamicTextData::iterator_impl *UVDGUIStringData::iterator_impl::copy()
{
	UVDGUIStringData::iterator_impl *ret = new UVDGUIStringData::iterator_impl();
	*ret = *this;
	return ret;
}

std::string UVDGUIStringData::iterator_impl::toString()
{
	return UVDSprintf("m_offset=0x%08X, m_index=0x%08X", m_offset, m_index);
}

unsigned int UVDGUIStringData::iterator_impl::offset()
{
	return m_offset;
}

uv_err_t UVDGUIStringData::iterator_impl::get(std::string &ret)
{
	uv_assert_ret(m_dataImpl);
	uv_assert_ret(m_dataImpl->getStringEngine());
	if( m_offset >= m_dataImpl->getStringEngine()->m_strings.size() )
	{
		//Maybe instead we should adjust this to the max value?
		printf_error("string database index out of range!\n");
		printf_error("m_offset: %d, # strings: %d\n", m_offset, m_dataImpl->getStringEngine()->m_strings.size());
		//This is suppose to be a const operations and this object isn't necessarily saved anyway, so first is better
		if( true )
		{
			return UV_DEBUG(UV_ERR_GENERAL);
		}
		else
		{
			UV_DEBUG(UV_ERR_GENERAL);
			uv_assert_err_ret(m_dataImpl->getMaxOffset(&m_offset));
		}
	}
	return UV_DEBUG(m_dataImpl->getStringEngine()->m_strings[m_offset].readString(ret));
}

uv_err_t UVDGUIStringData::iterator_impl::previous()
{
	if( m_offset > 0 )
	{
		--m_offset;
	}

	return UV_ERR_OK;
}
	
uv_err_t UVDGUIStringData::iterator_impl::next()
{
	if( m_offset >= m_dataImpl->getStringEngine()->m_strings.size() )
	{
		m_offset = m_dataImpl->getStringEngine()->m_strings.size() - 1;
	}
	else
	{
		++m_offset;
	}

	return UV_ERR_OK;
}

uv_err_t UVDGUIStringData::iterator_impl::changePositionByLineDelta(int delta)
{
	//Don't do this, leads to under/overflow issues
	//m_offset += delta;
	
	if( delta > 0 )
	{
		m_offset += delta;
		if( m_offset >= m_dataImpl->getStringEngine()->m_strings.size() )
		{
			m_offset = m_dataImpl->getStringEngine()->m_strings.size() - 1;
		}
	}
	else
	{
		if( (unsigned)-delta > m_offset )
		{
			m_offset = 0;
		}
		else
		{
			m_offset += delta;
		}
	}
	return UV_ERR_OK;
}

uv_err_t UVDGUIStringData::iterator_impl::changePositionToLine(unsigned int offset, unsigned int index)
{
	//Index is ignored
	//m_offset = scrollbarPositionToOffset(offset);
	m_offset = offset;
	m_index = index;
	return UV_ERR_OK;
}

int UVDGUIStringData::iterator_impl::compare(const UVQtDynamicTextData::iterator_impl *otherIn)
{
	//Should be of this type
	const iterator_impl *other = static_cast<const iterator_impl *>(otherIn);
	
	//printf("compare %d to %d\n", m_offset, other->m_offset);
	return m_offset - other->m_offset;
}

/*
	UVDStringEngine *stringEngine = NULL;
	
	UVD_AUTOLOCK_ENGINE_BEGIN();
	
	stringEngine = m_dataImpl->getStringEngine();

	for( std::vector<UVDString>::iterator iter = stringEngine->m_strings.begin();
			iter != stringEngine->m_strings.end(); ++iter )
	{
		UVDString uvdString = *iter;
		std::string string;
		
		//Read a string
		uv_assert_err_ret(uvdString.readString(string));

		m_indexBuffer.push_back(UVDSprintf("# 0x%.8X: %s", uvdString.m_addressRange.m_min_addr, stringTableStringFormat(lines[0]).c_str()));				
	}
	
	UVD_AUTOLOCK_ENGINE_END();

	return UV_ERR_OK;
*/
