/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#include <string.h>
#include <stdio.h>
#include "uvd_types.h"
#include "uvd_debug.h"

UVDVarient::UVDVarient()
{
	m_type = UVD_VARIENT_UNKNOWN;
}

UVDVarient::UVDVarient(std::string s)
{
	setString(s);
}

UVDVarient::UVDVarient(int32_t i)
{
	setI32(i);
}

UVDVarient::UVDVarient(uint32_t i)
{
	setUI32(i);
}

uvd_varient_t UVDVarient::getType() const
{
	return m_type;
}

uv_err_t UVDVarient::getString(std::string &s) const
{
	if( getType() != UVD_VARIENT_STRING )
	{
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	s = m_sVal;
	return UV_ERR_OK;
}

void UVDVarient::setString(const std::string &s)
{
	m_type = UVD_VARIENT_STRING;
	m_sVal = s;
}

uv_err_t UVDVarient::getI32(int32_t &i) const
{
	if( getType() != UVD_VARIENT_INT32 )
	{
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	i = m_I32Val;
	return UV_ERR_OK;
}

void UVDVarient::setI32(int32_t i)
{
	m_type = UVD_VARIENT_INT32;
	m_I32Val = i;
}

uv_err_t UVDVarient::getUI32(uint32_t &i) const
{
	if( getType() != UVD_VARIENT_UINT32 )
	{
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	i = m_UI32Val;
	return UV_ERR_OK;
}

void UVDVarient::setUI32(uint32_t i)
{
	m_type = UVD_VARIENT_UINT32;
	m_UI32Val = i;
}

std::string UVDVarient::toString() const
{
	std::string sRet;
	char buff[32];
	
	switch( getType() )
	{
	case UVD_VARIENT_INT32:
		snprintf(buff, 32, "%d", m_I32Val);
		sRet = buff;
		break;
	case UVD_VARIENT_UINT32:
		snprintf(buff, 32, "%u", m_UI32Val);
		sRet = buff;
		break;
	case UVD_VARIENT_STRING:
		sRet = m_sVal;
		break;
	default:
		sRet = "<UNKNOWN>";
	}
	
	return sRet;
}

/*
UVDUint32RangePair
*/

UVDUint32RangePair::UVDUint32RangePair()
{
	m_min = 0;
	m_max = 0;
}

UVDUint32RangePair::UVDUint32RangePair(uint32_t min, uint32_t max)
{
	m_min = min;
	m_max = max;
}

uint32_t UVDUint32RangePair::size() const
{
	if( m_min > m_max )
	{
		return 0;
	}
	return m_max - m_min + 1;
}

bool UVDUint32RangePair::contains(uint32_t val)
{
	return val >= m_min && val <= m_max;
}
