/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/assembly/address.h"
#include "uvd/core/uvd.h"

/*
UVDAddress
*/

UVDAddress::UVDAddress()
{
	m_space = NULL;
	m_addr = 0;
}

UVDAddress::UVDAddress(uv_addr_t addr, UVDAddressSpace *space)
{
	m_space = space;
	m_addr = addr;
}

bool UVDAddress::intersects(const UVDAddressRange &range) const
{
	return UVDAddressRange(m_addr, m_addr, m_space).intersects(range);
}

//By minimum address
int UVDAddress::compare(const UVDAddress *other) const
{
	//Checked builds only
	if( other == NULL )
	{
		return 1;
	}
	
	//We assume they are in the same address space or the comparison doesn't make sense
	return m_addr - other->m_addr;
}

bool UVDAddress::operator<(const UVDAddress *other) const
{
	return compare(other) < 0;
}

bool UVDAddress::operator>(const UVDAddress *other) const
{
	return compare(other) > 0;
}

bool UVDAddress::operator==(const UVDAddress *other) const
{
	return compare(other) == 0;
}

/*
UVDAddressRange
*/

UVDAddressRange::UVDAddressRange()
{
	m_min_addr = 0;
	m_max_addr = 0;
	m_space = NULL;
}

UVDAddressRange::UVDAddressRange(uv_addr_t min_addr)
{
	m_min_addr = min_addr;
	m_max_addr = min_addr;
	m_space = NULL;
}

UVDAddressRange::UVDAddressRange(uv_addr_t min_addr, uv_addr_t max_addr, UVDAddressSpace *space)
{
	m_min_addr = min_addr;
	m_max_addr = max_addr;
	m_space = space;
}

bool UVDAddressRange::intersects(const UVDAddressRange &other) const
{
	//A aaaa B abababa A bbbbb B
	return (other.m_min_addr <= m_max_addr && other.m_max_addr >= m_min_addr)
			|| (m_min_addr <= other.m_max_addr && m_max_addr >= other.m_min_addr);
}

int UVDAddressRange::compareStatic(const UVDAddressRange *l, const UVDAddressRange *r) 
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

int UVDAddressRange::compare(const UVDAddressRange *other) const
{
	return compareStatic(this, other);
}

bool UVDAddressRange::operator<(const UVDAddressRange *other) const
{
	return compare(other) < 0;
}

bool UVDAddressRange::operator>(const UVDAddressRange *other) const
{
	return compare(other) > 0;
}

bool UVDAddressRange::operator==(const UVDAddressRange *other) const
{
	return compare(other) == 0;
}

unsigned int UVDAddressRange::size() const
{
	return m_max_addr - m_min_addr + 1;
}

uv_err_t UVDAddressRange::memoryToString(std::string &out, bool safe) const
{
	uv_assert_ret(m_space);
	uv_assert_ret(m_space->m_data);
	if( safe )
	{
		return UV_DEBUG(m_space->m_data->readDataAsSafeString(m_min_addr, size(), out));	
	}
	else
	{
		return UV_DEBUG(m_space->m_data->readDataAsString(m_min_addr, size(), out));	
	}
}

/*
UVDAddressSpace
*/

UVDAddressSpace::UVDAddressSpace()
{
	//m_type = 0;
	m_min_addr = 0;
	m_max_addr = 0;
	m_R = 0;
	m_W = 0;
	m_X = 0;
	m_word_size = 0;
	m_word_alignment = 0;
	m_data = NULL;
}

UVDAddressSpace::~UVDAddressSpace()
{
	deinit();
}

uv_err_t UVDAddressSpace::deinit()
{
	m_synonyms.clear();
	/*
	for( std::vector<UVDAddressSpaceMapper *>::iterator iter = m_mappers.begin(); iter != m_mappers.end(); ++iter )
	{
		delete *iter;
	}
	m_mappers.clear();
	*/
	
	return UV_ERR_OK;
}

uv_err_t UVDAddressSpace::setEquivMemName(uint32_t addr, const std::string &name)
{
	printf_debug("setEquivMemName: %s(0x%.8X) = %s\n", m_name.c_str(), addr, name.c_str());
	m_synonyms[addr] = name;
	return UV_ERR_OK;
}

uv_err_t UVDAddressSpace::getEquivMemName(uint32_t addr, std::string &name)
{
	if( m_synonyms.find(addr) == m_synonyms.end() )
	{
		return UV_ERR_GENERAL;
	}
	name = m_synonyms[addr];
	return UV_ERR_OK;
}

uv_err_t UVDAddressSpace::getNumberAnalyzedBytes(uint32_t *analyzedBytesOut)
{
	uv_addr_t minAddress = 0;
	uv_addr_t maxAddress = 0;

	uv_assert_err_ret(getMinValidAddress(&minAddress));
	uv_assert_err_ret(getMaxValidAddress(&maxAddress));
	uv_assert_ret(minAddress <= maxAddress);
	//printf_debug("Analyzed bytes range: 0x%.8X-0x%.8X\n", minAddress, maxAddress);

	uv_assert_ret(analyzedBytesOut);
	*analyzedBytesOut = maxAddress - minAddress + 1;
	
	//uv_assert_ret(analyzedBytesOut);
	//*analyzedBytesOut = 1;
	return UV_ERR_OK;
}

uv_err_t UVDAddressSpace::getMinValidAddress(uv_addr_t *out)
{
	//Since we assume for now that the executable is 0 to size - 1, min address here is the min address
	return UV_DEBUG(g_uvd->m_config->getAddressMin(out));
}

uv_err_t UVDAddressSpace::getMaxValidAddress(uv_addr_t *out)
{
	uv_addr_t maxConfigAddress = 0;
	uv_addr_t maxPhysicalAddress = 0;
	
	uv_assert_ret(g_uvd);
	uv_assert_ret(g_uvd->m_config);
	uv_assert_err_ret(g_uvd->m_config->getAddressMax(&maxConfigAddress));
	uv_assert_ret(m_data);
	uv_assert_err_ret(m_data->size(&maxPhysicalAddress));
	//We got size, not end
	--maxPhysicalAddress;
	
	//Return the lower of the two
	uv_assert_ret(out);
	if( maxPhysicalAddress <= maxConfigAddress )
	{
		*out = maxPhysicalAddress;
	}
	else
	{
		*out = maxConfigAddress;
	}
	return UV_ERR_OK;
}

uv_err_t UVDAddressSpace::nextValidAddress(uv_addr_t start, uv_addr_t *ret)
{
	uv_addr_t configRet = 0;
	uv_addr_t addressMax = 0;
	uv_err_t rc = UV_ERR_GENERAL;

	rc = g_uvd->m_config->nextValidAddress(start, &configRet);
	uv_assert_err_ret(rc);
	
	//No more valid addresses based on config?
	if( rc == UV_ERR_DONE )
	{
		//printf("config says address 0x%04X is out of bounds\n", start);  
		return UV_ERR_DONE;
	}
	uv_assert_err_ret(getMaxValidAddress(&addressMax));
	//We may have also exceeded the practical file bounds
	if( configRet > addressMax )
	{
		return UV_ERR_DONE;
	}

	//Seems like its still a valid address
	*ret = configRet;
	
	return UV_ERR_OK;
}

uv_err_t UVDAddressSpace::nextCodingAddress(uv_addr_t start, uint32_t *ret)
{
	uint32_t cur = start;
	
	for( ;; )
	{
		uint32_t last = cur;
		
#if 0
		for( std::vector<UVDAddressRange>::iterator iter = m_uvd->m_noncodingAddresses.begin();
				iter != m_uvd->m_noncodingAddresses.end(); ++iter )
		{
			UVDAddressRange mem = *iter;
			
			if( mem.intersects(UVDAddressRange(cur)) )
			{
				//Are we out of addresses?
				if( mem.m_max_addr == UVD_ADDR_MAX )
				{
					return UV_ERR_DONE;
				}
				
				//We will keep intersecting until the end of this block, advance past
				cur = mem.m_max_addr + 1;
				break;
			}
		}
#endif
		
		//Done if we stopped hitting non-coding addresses
		if( last == cur )
		{
			break;
		}
	}
	
	*ret = cur;
	
	return UV_ERR_OK;
}

uv_err_t UVDAddressSpace::nextValidExecutableAddress(uv_addr_t start, uint32_t *ret)
{
	uv_addr_t cur = start;
	
	//Number of non-coding addresses is expected to be small at this point
	//Better algorithm later if necessary
	for( ;; )
	{
		//Keep iterating as long as another memory region matches
		uv_addr_t last = cur;
		uv_err_t rc = UV_ERR_GENERAL;
		
		//Filter based on generic invalid addresses
		rc = nextValidAddress(cur, &cur);
		uv_assert_err_ret(rc);
		if( rc == UV_ERR_DONE )
		{
			return UV_ERR_DONE;
		}

		//Then filter based on regions found to be non-coding
		rc = nextCodingAddress(cur, &cur);
		uv_assert_err_ret(rc);
		if( rc == UV_ERR_DONE )
		{
			return UV_ERR_DONE;
		}
		
		//If we didn't find any more advancements, break
		if( cur == last )
		{
			break;
		}
	}

	uv_assert_ret(ret);
	*ret = cur;
	return UV_ERR_OK;
}

#if 0
uv_err_t UVDAddressSpace::remap(UVDAddressSpace **out)
{
	return UV_DEBUG(remap(m_min_addr, m_max_addr, out));
}

uv_err_t UVDAddressSpace::remap(uv_addr_t minAddress, uv_addr_t maxAddress, UVDAddressSpace **out)
{
	UVDAddressSpace *addressSpace = NULL;
	UVDDataChunk *data = NULL;
	
	addressSpace = new UVDAddressSpace();
	uv_assert_ret(addressSpace);
	//Default to copying junk over
	//This calls copy constructor of the std::string I think...
	*addressSpace = *this;
	
	//The important part, remap the data
	data = new UVDDataChunk();
	uv_assert_ret(data);
	uv_assert_err_ret(data->init(m_data, minAddress, maxAddress));
	addressSpace->m_data = data;
	
	//And adjust our bounds
	//Callee can change it if its a non-trivial virtula mapping
	addressSpace->m_min_addr = 0;
	addressSpace->m_max_addr = maxAddress - minAddress;
	
	uv_assert_ret(out);
	*out = addressSpace;
	return UV_ERR_OK;
}
#endif

/*
UVDAddressSpaces
*/

UVDAddressSpaces::UVDAddressSpaces()
{
}

UVDAddressSpaces::~UVDAddressSpaces()
{
	for( std::vector<UVDAddressSpace *>::iterator iter = m_addressSpaces.begin();
			iter != m_addressSpaces.end(); ++iter )
	{
		delete *iter;
	}
}

