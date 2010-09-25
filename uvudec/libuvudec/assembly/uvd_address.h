/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_ADDRESS_H
#define UVD_ADDRESS_H

#include <string>
#include "uvd_data.h"
//uv_addr_t is somewhat arbitrarily defined in here
#include "uvd_types.h"

/*
An address resolved to an address space
Fully specifies the address within the architecture
*/
class UVDAddressRange;
class UVDAddressSpace;
class UVDAddress
{
public:
	UVDAddress();
	UVDAddress(uv_addr_t addr, UVDAddressSpace *space = NULL);
	
	bool intersects(const UVDAddressRange &range) const;

	//By minimum address
	int compare(const UVDAddress *other) const;	
	bool operator<(const UVDAddress *other) const;
	bool operator>(const UVDAddress *other) const;
	bool operator==(const UVDAddress *other) const;

public:
	//Address space
	UVDAddressSpace *m_space;
	//Address
	uv_addr_t m_addr;
};

/*
A fully qualified memory range
*/
class UVDAddressRange
{
public:
	UVDAddressRange();
	UVDAddressRange(uv_addr_t min_addr);
	UVDAddressRange(uv_addr_t min_addr, uv_addr_t max_addr, UVDAddressSpace *space = NULL);
	bool intersects(const UVDAddressRange &other) const;

	//By minimum address
	static int compareStatic(const UVDAddressRange *l, const UVDAddressRange *r);	
	int compare(const UVDAddressRange *other) const;	
	bool operator<(const UVDAddressRange *other) const;
	bool operator>(const UVDAddressRange *other) const;
	bool operator==(const UVDAddressRange *other) const;

public:
	//Address space
	UVDAddressSpace *m_space;
	//Address
	uv_addr_t m_min_addr;
	uv_addr_t m_max_addr;
};

//Shared information about an address space
class UVDAddressSpace
{
public:
	UVDAddressSpace();
	~UVDAddressSpace();
	uv_err_t deinit();
	
	uv_err_t setEquivMemName(uint32_t addr, const std::string &name);
	uv_err_t getEquivMemName(uint32_t addr, std::string &name);
	
public:
	//short name
	std::string m_name;
	//longer description
	std::string m_desc;
	//EPROM, RAM, etc.  Defines prefixed with UV_DISASM_MEM_
	uint32_t m_type;
	//Valid addresses
	uv_addr_t m_min_addr;
	uv_addr_t m_max_addr;
	//Used for output
	//XXX: this is policy, separate it out
	std::string m_print_prefix;
	std::string m_print_suffix;
	/*
	Capabilities, how the memory behaves as a whole.  Read, write, etc
	Does not include policy based capabilities such as process 1 cannot write to address 0x1234
	*/
	uint32_t m_cap;
	
	/*
	Minimum amount of memory that can be transferred in bits
	Each address moves forward in the address space this much
	Note that single bit is observed on processors such as 8051
	Might also be useful for CPU flags
	*/
	uint32_t m_word_size;
	uint32_t m_word_alignment;
	
	/*
	Address synonyms
	Ex: 8051 direct addresses IRAM @ F0 is B register
	Should these be address space mappings instead?
	*/
	std::map<uv_addr_t, std::string> m_synonyms;
	/*
	Does this map to something more absolute?
	If so, address that this is mapped to
	FIXME: implement this with polymorphism instead
	*/
	//std::vector<UVDAddressSpaceMapper *> m_mappers;
	/*
	struct uv_disasm_mem_shared_t *mapped;
	Start address, in target address space words
	unsigned int mapped_start_addr;
	Start address bit offet from above
	unsigned int mapped_start_addr_bit_offset;
	*/
};

/*
An entire set of address spaces, like might be found in a complete architecture
*/
class UVDAddressSpaces
{
public:
	UVDAddressSpaces();
	~UVDAddressSpaces();

public:
	//We own these
	std::vector<UVDAddressSpace *> m_addressSpaces;
};

#endif

