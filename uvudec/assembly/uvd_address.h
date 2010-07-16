/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_ADDRESS_H
#define UVD_ADDRESS_H

#include <string>
#include "uvd_data.h"
#include "uvd_types.h"

/* Internal RAM */
#define UV_DISASM_MEM_RAM_INT			1
/* External RAM */
#define UV_DISASM_MEM_RAM_EXT			2
/* Some sort of read only memory */
#define UV_DISASM_MEM_RAM_ROM			3
/* Flash storage */
#define UV_DISASM_MEM_RAM_FLASH			4

/* Capabilities */
/* Read */
#define UV_DISASM_MEM_R					0x01
/* Write */
#define UV_DISASM_MEM_W					0x02
/* Execute */
#define UV_DISASM_MEM_X					0x04
/* Nonvalitile */
#define UV_DISASM_MEM_NV				0x08

/*
Often times memory one memory space will intersect with another
Controls mapping of spaces
Mapping size must line up: if not, will assume error for now.
	Will change if a needed exception arises
	Ex: Bit in main address space corresponds to ZFLAG
	Could map to intermediate BRAM and then to the flag to avoid issue of byte to single bit

Ex:
	Mapping from 8-51 bit RAM to main address space
	Source
		BRAM (Bit RAM)
		8 bytes, 64 bits total
		word size: 1
		start address: 0
		end address: 63
	Dest
		IRAM (Internal RAM)
		8 bytes, 64 biits total
		word size: 8
		start adddress: 32
		end address: 39
Ex:
	Mapping 8051 direct addressing to main IRAM
	Source
		Direct addressing
		128 bytes
		word size: 8
		start address: 0
		end address: 0x7F
	Dest
		Indirect addressing space (or could use instead a main phsyiscal memory)
		256 bytes
		word size: 8
		start address: 0
		end address: 0xFF
*/
class UVDMemoryShared;
class UVDMemorySharedMapper
{
public:
	UVDMemorySharedMapper();
	
	uv_err_t finalizeConfig();
	
public:
	//Other address space
	//These are not owned by this object, only mapped
	UVDMemoryShared *m_dst_shared;
	UVDMemoryShared *m_src_shared;
	
	//Source memory
	//Start map address
	uint32_t m_src_min_addr;
	//Stop map address
	uint32_t m_src_max_addr;
	//Destination memory
	uint32_t m_dst_min_addr;
	uint32_t m_dst_max_addr;
};

//Shared information about an address space
class UVDMemoryShared
//struct uv_disasm_mem_shared_t
{
public:
	UVDMemoryShared();
	~UVDMemoryShared();
	uv_err_t deinit();
	
	uv_err_t setEquivMemName(uint32_t addr, const std::string &name);
	uv_err_t getEquivMemName(uint32_t addr, std::string &name);
	
public:
	/* short name */
	std::string m_name;
	/* longer description */
	std::string m_desc;
	/* EPROM, RAM, etc.  Defines prefixed with UV_DISASM_MEM_ */
	unsigned int m_type;
	/* Valid addresses */
	unsigned int m_min_addr;
	unsigned int m_max_addr;
	/* Used for output */
	std::string m_print_prefix;
	std::string m_print_suffix;
	/*
	Capabilities, how the memory behaves as a whole.  Read, write, etc
	Does not include policy based capabilities such as process 1 cannot write to address 0x1234
	*/
	unsigned int m_cap;
	
	/*
	Minimum amount of memory that can be transferred in bits
	Note that single bits is observed on processors such as 8051
	Might also be useful for CPU flags
	*/
	unsigned int m_word_size;
	unsigned int m_word_alignment;
	
	/*
	Address synonyms
	Ex: 8051 direct addresses IRAM @ F0 is B register
	Should these be address space mappings instead?
	*/
	std::map<uint32_t, std::string> m_synonyms;
	/*
	Does this map to something more absolute?
	If so, address that this is mapped to
	*/
	std::vector<UVDMemorySharedMapper *> m_mappers;
	/*
	struct uv_disasm_mem_shared_t *mapped;
	Start address, in target address space words
	unsigned int mapped_start_addr;
	Start address bit offet from above
	unsigned int mapped_start_addr_bit_offset;
	*/
};

class UVDMemoryLocation
//struct uv_disasm_mem_loc_t
{
public:
	UVDMemoryLocation();
	//UVDMemoryLocation(UVDMemoryLocation *other);
	//UVDMemoryLocation(const UVDMemoryLocation &other);
	UVDMemoryLocation(unsigned int min_addr);
	UVDMemoryLocation(unsigned int min_addr, unsigned int max_addr, UVDMemoryShared *space = NULL);
	bool intersects(UVDMemoryLocation other) const;

	//By minimum address
	static int compareStatic(const UVDMemoryLocation *l, const UVDMemoryLocation *r);	
	int compare(const UVDMemoryLocation *other) const;	
	bool operator<(const UVDMemoryLocation *other) const;
	bool operator>(const UVDMemoryLocation *other) const;
	bool operator==(const UVDMemoryLocation *other) const;

public:
	/* Address space */
	UVDMemoryShared *m_space;
	/* Address */
	uint32_t m_min_addr;
	uint32_t m_max_addr;
};

/*
struct uv_disasm_mem_tag_t
{
	struct uv_disasm_mem_loc_t mem_loc;
	struct uv_disasm_mem_tag_t *next;
};
*/

/*
This is for virtual memory segments as opposed to above that deal with physical address spaces
For now, assume all segments hold valid data of some sort
Later, add flags and such
	Capabilities of a physical address space probably should ovveride those of any virtual segment?
Currently need some basic idea of what segments are active for determing total program size for progress bars
*/
class UVDMemorySegment
{
public:
	UVDMemorySegment();
	~UVDMemorySegment();

public:
	//Where the data starts
	uv_addr_t m_start;
	//The data held at this address
	UVDData *m_data;
};

//Virtual memory space
class UVDSegmentedMemory
{
public:
	UVDSegmentedMemory();
	~UVDSegmentedMemory();
	
public:
	/*
	The machine may or may not support actual segments (ie might really be a paged system)
	This more refers to how the address space layout logUVDSegmentedMemoryically is discontiguous with programs loading data at various addresses
	Rename later if needed
	*/
	std::vector<UVDMemorySegment *> m_segments;
};

#endif
