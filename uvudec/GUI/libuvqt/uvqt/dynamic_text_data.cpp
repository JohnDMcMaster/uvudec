/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include <QPainter>
#include <QPaintEvent>
#include <QScrollBar>
#include "stdio.h"
#include <stdint.h>
#include <math.h>
#include "uvqt/dynamic_text.h"
#include "uvqt/util.h"
#include "uvd/util/util.h"
#include "uvqt/dynamic_text_plugin_impl.h"

/*
UVQtDynamicTextData::iterator_impl
*/

UVQtDynamicTextData::iterator_impl::iterator_impl()
{
}

UVQtDynamicTextData::iterator_impl::iterator_impl(unsigned int offset, unsigned int index)
{
}

UVQtDynamicTextData::iterator_impl::~iterator_impl()
{
}

std::string UVQtDynamicTextData::iterator_impl::toString()
{
	return "";
}

/*
iterator *UVQtDynamicTextData::iterator_impl::copy()
{
	return 
}

uv_err_t UVQtDynamicTextData::iterator_impl::get(std::string &ret)
{
	(void)ret;
	return UV_DEBUG(UV_ERR_GENERAL);
}

uv_err_t UVQtDynamicTextData::iterator_impl::next()
{
	return UV_DEBUG(UV_ERR_GENERAL);
}

uv_err_t UVQtDynamicTextData::iterator_impl::changePositionByLineDelta(int delta)
{
	(void)delta;
	return UV_DEBUG(UV_ERR_GENERAL);
}

uv_err_t UVQtDynamicTextData::iterator_impl::changePositionToLine(unsigned int offset, unsigned int index)
{
	(void)offset;
	(void)index;
	return UV_DEBUG(UV_ERR_GENERAL);
}
*/

/*
UVQtDynamicTextData::iterator
*/

UVQtDynamicTextData::iterator::iterator()
{
	//printf("no impl construction on 0x%08X\n", (int)this);
	m_impl = NULL;
	//UVD_PRINT_STACK();
}

UVQtDynamicTextData::iterator::iterator(iterator_impl *impl)
{
	//printf("setting iterator_impl m_impl: 0x%08X on 0x%08X\n", (int)impl, (int)this);
	m_impl = impl;
}

UVQtDynamicTextData::iterator::~iterator()
{
	delete m_impl;
}

UVQtDynamicTextData::iterator &UVQtDynamicTextData::iterator::operator=(const UVQtDynamicTextData::iterator &source)
{
	//printf("oper called\n");
	//exit(1);

	//printf("assigning new m_impl, old: 0x%08X, template 0x%08X\n", (int)m_impl, (int)source.m_impl);
	delete m_impl;
	m_impl = source.m_impl->copy();
	//printf("new m_impl: 0x%08X\n", (int)m_impl);	
	if( m_impl == NULL )
	{
		printf("iterator_impl m_impl null, boom\n");
		fflush(stdout);
		//UVD_PRINT_STACK();
	}
	
	//	iterator_impl *retImpl = m_impl->copy();
	//printf("ret iter m_impl: 0x%08X\n", (int)retImpl);	
	//	return UVQtDynamicTextData::iterator(retImpl);
	return *this;
}

bool UVQtDynamicTextData::iterator::operator==(const iterator &other)
{
	return compare(&other) == 0;
}

bool UVQtDynamicTextData::iterator::operator!=(const iterator &other)
{
	return compare(&other) != 0;
}

int UVQtDynamicTextData::iterator::compare(const iterator *other)
{
	/*
	if( m_impl == NULL || other.m_impl == NULL )
	{
		return 0;
	}
	*/
	return m_impl->compare(other->m_impl);
}

uv_err_t UVQtDynamicTextData::iterator::get(std::string &ret)
{
	uv_assert_ret(m_impl);
	return UV_DEBUG(m_impl->get(ret));
}

uv_err_t UVQtDynamicTextData::iterator::next()
{
	uv_assert_ret(m_impl);
	return UV_DEBUG(m_impl->next());
}

uv_err_t UVQtDynamicTextData::iterator::changePositionByLineDelta(int delta)
{
	uv_assert_ret(m_impl);
	return UV_DEBUG(m_impl->changePositionByLineDelta(delta));
}

uv_err_t UVQtDynamicTextData::iterator::changePositionToLine(unsigned int offset, unsigned int index)
{
	uv_assert_ret(m_impl);
	return UV_DEBUG(m_impl->changePositionToLine(offset, index));
}

unsigned int UVQtDynamicTextData::iterator::offset()
{
	uv_assert_ret(m_impl);
	return UV_DEBUG(m_impl->offset());
}

/*
UVQtDynamicTextData::iterator & operator=(UVQtDynamicTextData::iterator &dest, const UVQtDynamicTextData::iterator &source)
{
printf("global call\n");
	return dest;
}
*/


/*
UVQtDynamicTextData
*/

UVQtDynamicTextData::UVQtDynamicTextData()
{
}

UVQtDynamicTextData::iterator UVQtDynamicTextData::begin(unsigned int offset, unsigned int index)
{
	UVQtDynamicTextData::iterator ret;
	UV_DEBUG(begin(offset, index, &ret));
	return ret;
}

UVQtDynamicTextData::iterator UVQtDynamicTextData::end()
{
	UVQtDynamicTextData::iterator ret;
	UV_DEBUG(end(&ret));
	return ret;
}

