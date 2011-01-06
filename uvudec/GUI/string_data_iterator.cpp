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
	return UV_DEBUG(g_mainWindow->m_project->m_uvd->m_analyzer->m_stringEngine->m_strings[m_offset].readString(ret));
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
	if( m_offset >= g_mainWindow->m_project->m_uvd->m_analyzer->m_stringEngine->m_strings.size() )
	{
		m_offset = g_mainWindow->m_project->m_uvd->m_analyzer->m_stringEngine->m_strings.size() - 1;
	}
	else
	{
		++m_offset;
	}

	return UV_ERR_OK;
}

uv_err_t UVDGUIStringData::iterator_impl::changePositionByLineDelta(int delta)
{
	m_offset += delta;
	/*
	//printf("**got a delta: %d\n", delta);
	int offsetDelta = scrollbarPositionDeltaToOffsetDelta(delta);
	if( delta > 0 )
	{
		m_offset += offsetDelta;
		
		printf("%d\n", maxValidOffset());
		if( m_offset > maxValidOffset() )
		{
			printf("overflow\n");
			m_offset = maxValidOffset();
		}
	}
	else
	{
		if( (unsigned)-offsetDelta > m_offset )
		{
			printf("predicted underflow\n");
			m_offset = 0;
		}
		else
		{
			m_offset += offsetDelta;
		}
	}
	//printf("end offset: %d\n", m_offset);
	*/
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
	
	stringEngine = g_mainWindow->m_project->m_uvd->m_analyzer->m_stringEngine;

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
