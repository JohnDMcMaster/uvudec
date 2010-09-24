/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd_address.h"

UVDMemorySharedMapper::UVDMemorySharedMapper()
{
	m_src_shared = NULL;
	m_dst_shared = NULL;
	
	m_src_min_addr = 0;
	m_src_max_addr = 0;
	m_dst_min_addr = 0;
	m_dst_max_addr = 0;
}

uv_err_t UVDMemorySharedMapper::finalizeConfig()
{
	//Sizes in bits
	//In future may allow for slight descrepency between source and dest, but for now must be same
	uint32_t src_data_size = 0;
	uint32_t dst_data_size = 0;
	
	printf_debug("initial src; min: %d, max: %d\n", m_src_min_addr, m_src_max_addr);
	printf_debug("initial dst; min: %d, max: %d\n", m_dst_min_addr, m_dst_max_addr);

	/*
	For source, assume full range if not specified
	This is because we usually will map to a larger space
	*/
	if( m_src_max_addr == 0 )
	{
		m_src_max_addr = m_src_shared->m_max_addr;
	}
	
	uv_assert_ret(m_src_min_addr <= m_src_max_addr);
	uv_assert_ret(m_src_shared);
	uv_assert_ret(m_dst_shared);
	uv_assert_ret(m_src_shared->m_word_size);
	uv_assert_ret(m_dst_shared->m_word_size);
	src_data_size = (m_src_max_addr - m_src_min_addr + 1) * m_src_shared->m_word_size;
	
	//If destination address is 0, assume not mapped
	if( m_dst_max_addr != 0 )
	{
		uv_assert_ret(m_dst_min_addr <= m_dst_max_addr);
		uv_assert_ret(m_dst_shared);
	}
	else
	{
		//Assume equal then
		//Set max addr to min + sizeof(src) / word size
		m_dst_max_addr = m_dst_min_addr + src_data_size / m_dst_shared->m_word_size - 1;
		//Assume equal then
	}

	//Make sure in the end they are equal
	dst_data_size = (m_dst_max_addr - m_dst_min_addr + 1) * m_dst_shared->m_word_size;
	printf_debug("final src; min: %d, max: %d\n", m_src_min_addr, m_src_max_addr);
	printf_debug("final dst; min: %d, max: %d\n", m_dst_min_addr, m_dst_max_addr);
	printf_debug("word; src : %d, dst: %d\n", m_src_shared->m_word_size, m_dst_shared->m_word_size);
	printf_debug("size; src: %d, dst: %d\n", src_data_size, dst_data_size);
	uv_assert_ret(src_data_size == dst_data_size);

	return UV_ERR_OK;
}

UVDMemoryShared::UVDMemoryShared()
{
	m_type = 0;
	m_min_addr = 0;
	m_max_addr = 0;
	m_cap = 0;
	m_word_size = 0;
	m_word_alignment = 0;
}

UVDMemoryShared::~UVDMemoryShared()
{
	deinit();
}

uv_err_t UVDMemoryShared::deinit()
{
	m_synonyms.clear();
	for( std::vector<UVDMemorySharedMapper *>::iterator iter = m_mappers.begin(); iter != m_mappers.end(); ++iter )
	{
		delete *iter;
	}
	m_mappers.clear();
	
	return UV_ERR_OK;
}

uv_err_t UVDMemoryShared::setEquivMemName(uint32_t addr, const std::string &name)
{
	printf_debug("setEquivMemName: %s(0x%.8X) = %s\n", m_name.c_str(), addr, name.c_str());
	m_synonyms[addr] = name;
	return UV_ERR_OK;
}

uv_err_t UVDMemoryShared::getEquivMemName(uint32_t addr, std::string &name)
{
	if( m_synonyms.find(addr) == m_synonyms.end() )
	{
		return UV_ERR_GENERAL;
	}
	name = m_synonyms[addr];
	return UV_ERR_OK;
}

UVDMemoryLocation::UVDMemoryLocation()
{
	m_min_addr = 0;
	m_max_addr = 0;
	m_space = NULL;
}

UVDMemoryLocation::UVDMemoryLocation(unsigned int min_addr)
{
	m_min_addr = min_addr;
	m_max_addr = min_addr;
	m_space = NULL;
}

UVDMemoryLocation::UVDMemoryLocation(unsigned int min_addr, unsigned int max_addr, UVDMemoryShared *space)
{
	m_min_addr = min_addr;
	m_max_addr = max_addr;
	m_space = space;
}

bool UVDMemoryLocation::intersects(UVDMemoryLocation other) const
{
	//A aaaa B abababa A bbbbb B
	return (other.m_min_addr <= m_max_addr && other.m_max_addr >= m_min_addr)
			|| (m_min_addr <= other.m_max_addr && m_max_addr >= other.m_min_addr);
}

int UVDMemoryLocation::compareStatic(const UVDMemoryLocation *l, const UVDMemoryLocation *r) 
{
	if( l == r )
	{
		return 0;
	}
	if( r == NULL )
	{
		return -1;
	}
	if( l == NULL )
	{
		return 1;
	}
	
	return r->m_min_addr - l->m_min_addr;
}

int UVDMemoryLocation::compare(const UVDMemoryLocation *other) const
{
	return compareStatic(this, other);
}

bool UVDMemoryLocation::operator<(const UVDMemoryLocation *other) const
{
	return compare(other) < 0;
}

bool UVDMemoryLocation::operator>(const UVDMemoryLocation *other) const
{
	return compare(other) > 0;
}

bool UVDMemoryLocation::operator==(const UVDMemoryLocation *other) const
{
	return compare(other) == 0;
}

/*
UVDMemorySegment
*/

#if 0
UVDMemorySegment::UVDMemorySegment()
{
	m_start = 0;
	m_data = NULL;
}

UVDMemorySegment::~UVDMemorySegment()
{
}
#endif

#if 0
/*
UVDSegmentedMemory
*/

UVDSegmentedMemory::UVDSegmentedMemory()
{
}

UVDSegmentedMemory::~UVDSegmentedMemory()
{
}
#endif

