/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "GUI/GUI.h"
#include "GUI/assembly_data.h"
#include "uvd/string/engine.h"
#include "uvd/util/debug.h"

/*
UVDGUIAssemblyData::iterator_impl
*/

UVDGUIAssemblyData::iterator_impl::iterator_impl()
{
	m_dataImpl = NULL;
}

UVDGUIAssemblyData::iterator_impl::iterator_impl(UVDGUIAssemblyData *impl, unsigned int offset, unsigned int index)
{
	m_dataImpl = impl;
	
	UV_DEBUG(changePositionToLine(offset, index));
	if( index != 0 )
	{
		printf("bad begin with nonzero index\n");
	}
}

UVDGUIAssemblyData::iterator_impl::iterator_impl(UVDGUIAssemblyData *impl, UVDPrintIterator iter)
{
	m_dataImpl = impl;
	m_iter = iter;
}

UVDGUIAssemblyData::iterator_impl::~iterator_impl()
{
}

UVQtDynamicTextData::iterator_impl *UVDGUIAssemblyData::iterator_impl::copy()
{
	UVDGUIAssemblyData::iterator_impl *ret = new UVDGUIAssemblyData::iterator_impl();
	*ret = *this;
	return ret;
}

std::string UVDGUIAssemblyData::iterator_impl::toString()
{
	return UVDSprintf("m_iter.m_nextPosition=0x%08X", offset());
}

unsigned int UVDGUIAssemblyData::iterator_impl::offset()
{
	return m_iter.m_iter.m_address.m_addr;
}

uv_err_t UVDGUIAssemblyData::iterator_impl::get(std::string &ret)
{
	return UV_DEBUG(m_iter.getCurrent(ret));
}

uv_err_t UVDGUIAssemblyData::iterator_impl::previous()
{
	return UV_DEBUG(m_iter.previous());
}
	
uv_err_t UVDGUIAssemblyData::iterator_impl::next()
{
	return UV_DEBUG(m_iter.next());
}

uv_err_t UVDGUIAssemblyData::iterator_impl::changePositionByLineDelta(int delta)
{
	if( delta > 0 )
	{
		for( int i = 0; i < delta && m_iter != m_dataImpl->getUVD()->end(); ++i )
		{
			uv_assert_err_ret(m_iter.next());
		}
	}
	else
	{
		for( int i = delta; delta > 0 && m_iter != m_dataImpl->getUVD()->begin(); --i )
		{
			uv_assert_err_ret(m_iter.previous());
		}
	}
	return UV_ERR_OK;
}

uv_err_t UVDGUIAssemblyData::iterator_impl::changePositionToLine(unsigned int offset, unsigned int index)
{
	if( m_dataImpl->getUVD() )
	{
		m_iter = m_dataImpl->getUVD()->begin(offset);
	}
	return UV_ERR_OK;
}

int UVDGUIAssemblyData::iterator_impl::compare(const UVQtDynamicTextData::iterator_impl *otherIn)
{
	//Should be of this type
	const iterator_impl *other = static_cast<const iterator_impl *>(otherIn);
	
	//printf("compare %d to %d\n", m_offset, other->m_offset);
	return m_iter.compare(other->m_iter);
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
