/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_ADDRESS_H
#define UVD_ADDRESS_H

#include <string>
#include "uvd/data/data.h"
//uv_addr_t is somewhat arbitrarily defined in here
#include "uvd/util/types.h"

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

	uv_err_t check();

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
	//In bytes
	unsigned int size() const;
	//Return string representation of the referenced memory area
	//See string utils to explanation of safe
	uv_err_t memoryToString(std::string &out, bool safe = true) const;

public:
	//Address space
	UVDAddressSpace *m_space;
	//Address
	uv_addr_t m_min_addr;
	//Inclusive
	uv_addr_t m_max_addr;
};

//Shared information about an address space
class UVDAddressSpace
{
public:
	UVDAddressSpace();
	~UVDAddressSpace();
	uv_err_t deinit();
	
	uv_err_t setEquivMemName(uv_addr_t addr, const std::string &name);
	uv_err_t getEquivMemName(uv_addr_t addr, std::string &out);
	
	//Next valid address capable of having any sort of analysis on it
	uv_err_t nextValidAddress(uv_addr_t start, uv_addr_t *ret);
	//Like above, but also must be a canidate for an executable area
	uv_err_t nextValidExecutableAddress(uv_addr_t start, uv_addr_t *ret);
	//Force a rebuild of the internal database
	//uv_err_t rebuildDb();
	//Only based on coding list, not any other validity
	//Used internally by nextValidExecutionAddress()
	uv_err_t nextCodingAddress(uv_addr_t start, uv_addr_t *ret);

	//How many bytes we have to analyze in total
	//Based on size of program and analysis exclusions
	uv_err_t getNumberAnalyzedBytes(uint32_t *analyzedBytesOut);
	//Actual allowable address limits, not just what config says
	uv_err_t getMinValidAddress(uv_addr_t *out);
	uv_err_t getMaxValidAddress(uv_addr_t *out);

	//Ignores config values
	uv_err_t getMinAddress(uv_addr_t *out);
	uv_err_t getMaxAddress(uv_addr_t *out);
	
	/*
	Returns an address spaced with data mapped onto this one
	Callee owns the returned object
	*/
	//uv_err_t remap(UVDAddressSpace **out);	
	//uv_err_t remap(uv_addr_t minAddress, uv_addr_t maxAddress, UVDAddressSpace **out);	

public:
	//short name
	std::string m_name;
	//longer description
	std::string m_desc;
	//EPROM, RAM, etc.  Defines prefixed with UV_DISASM_MEM_
	//uint32_t m_type;
	//Valid addresses (inclusive)
	uv_addr_t m_min_addr;
	uv_addr_t m_max_addr;
	//Used for output
	//XXX: this is policy, separate it out
	std::string m_print_prefix;
	std::string m_print_suffix;
	/*
	Capabilities, how the memory behaves as a whole.  Read, write, etc
	Does not include policy based capabilities such as process 1 cannot write to address 0x1234
	TODO: change this to 3 tristate variables
	*/
	//uint32_t m_cap;
	uvd_tri_t m_R;
	uvd_tri_t m_W;
	uvd_tri_t m_X;
	//Non-volatile
	//If the power is turned off, does the data persist?
	uvd_tri_t m_NV;
	
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
	
	/*
	If we have have a live feed on this data (ROM, being debugged, etc), given here
	We own this
	FIXME: we need to provide a mapped version that is consistent with the address mappings
	This is currently treated as a raw pointer to the space
	ie it will break if we are using virtual addressing
	*/
	UVDData *m_data;
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

