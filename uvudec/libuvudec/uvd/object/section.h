/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_OBJECT_SECTION_H
#define UVD_OBJECT_SECTION_H

#include "uvd/assembly/address.h"
#include "uvd/util/types.h"
#include "uvd/relocation/relocation.h"

/*
#define UVD_SECTION_FLAG_NONE			0x0000
#define UVD_SECTION_FLAG_HAS_VMA		0x0001
#define UVD_SECTION_FLAG_
*/

/*
Note that we don't actually define memory spaces here, only hints and places to place data
Since object files (usually) don't make direct references as to where to place things on a specific system,
its up to UVDRuntime to translate between UVDSection hints to actual memory locations

Formats
Binary
	One section
	Unspecified everything...details/considerations follow
	Load address
		Should be loaded at the programming vector (usually 0) by the UVDArchitecture
	Data permissions
		Unspecified
ELF
	Program headers
		Loadable section information
		Ex
			p_type (type):              0x00000006 (6)
				PT_PHDR     (entry for header table itself)
			p_offset (file offset):     0x00000034 (52)
			p_filesz (size in file):    0x00000100 (256)
			p_flags (flags):            0x00000005 (5)
				PF_X (executable)
				PF_R (readable)
			p_align (alignment):        0x00000004 (4)
			p_vaddr (virtual address):  0x08048034 (134512692)
			p_paddr (physical address): 0x08048034 (134512692)
			p_memsz (size in memory):   0x00000100 (256)
	Section headers
		Misc information
		Vary greatly depending on type
PE/COFF

Object libbraries
bfd
	Philosiphy
		"The raw data contained within a BFD is maintained through the
		section abstraction.  A single BFD may have any number of
		sections.  It keeps hold of them by pointing to the first;
		each one points to the next in the list."
	See section/section.h:typedef struct bfd_section
		interestingly enough this header is constructed from a comment in the .c file...hmmm
	Big code base, but good reference
	Biased towards ELF?
		ex: Nonzero if this section uses RELA relocations, rather than REL.
		although it does have some other arch specific stuff, maybe I just happen to recognize the ELF stuff
	Code seems more cleaned up since the revisions I looked at years ago
		...although it still uses ugly GNU indent style
	struct bfd_section major members
		const char *name;
			some formats (ex: ELF) allow naming sections
		flagword flags;
			type of section
			Determines if its data, linker info etc
			Seems to more or less specify RWX attributes of section
				SEC_CODE => X
				SEC_DATA => R
				SEC_READONLY => !W
				SEC_ROM => !W
				How do these last two differ?
		bfd_vma vma;
			where it will be loaded at run time
			Has a matching is vma set field
		bfd_size_type size;
			data size
			vs bfd_size_type rawsize;
				original size on disk
				.bss would have size, but no rawsize
			void *userdata;
			unsigned char *contents;
			actual data, not sure quite difference
		unsigned int alignment_power;
			required alignment
		Relocations
			struct reloc_cache_entry *relocation;
				input relocations
			struct reloc_cache_entry **orelocation;
				output relocations
			need to keep track of these somehow
	Next level importance:
		alent *lineno;
			line number stuff
			for debugging object code?
		struct relax_table *relax;
			something related to linking
				"Relax table contains information about instructions which can
				be removed by relaxation -- replacing a long address with a 
				short address."
   			This could be important for FLIRT
   			But, its an advanced feature and don't worry about for now
		int id;
			File dependent index
			do we care about this?
			note this is not the same as int index; which is a BFD internal number
		bfd_vma lma;
			load address
			Do we need this?  Seems like some internal thing
		struct bfd_section *output_section;
			?
			"The output section through which to map on output"
		int target_index;
			probably for SH_LINK related
		struct relent_chain *constructor_chain;
			for initialization code
		struct bfd_symbol *symbol;
			symbol representing this section
IDA
	Not familar with the codebase
	They have a SDK, but I've never worked with it
bastard
	Not familar with the codebase
s-record
	eh dunno this one has a lot of obscure formats, but doesn't use good open source practices
	Not sure how much it should be used as a reference
*/

class UVDAddressSpace;
class UVDSection
{
public:
	UVDSection();
	virtual ~UVDSection();

	//Not all sections may have names
	//In this case, we return empty string
	//they also might be duplicate
	//Just use variable
	//virtual uv_err_t getName(std::string &out);
	//We own the returned object
	//If its a address space only initialized at runtime or such, UV_ERR_NOTSUPPORTED should be returned
	//virtual uv_err_t getData(UVDData **out);

	//is versions: known to be able to do operation
	//can versions: possible we could do the operation
	//bool isWrite();
	//bool canWrite();

	//Get all relocations, including those we find by analyzing the ISA
	virtual uv_err_t getAllRelocations(std::vector<UVDRelocationFixup> &out);
	
	/*
	Make a template address space from the data enclosed
	Returns UV_ERR_NOTSUPPORTED if this doesn't make sense 
	*/
	virtual uv_err_t toAddressSpace(UVDAddressSpace **out);

public:
	//Consider switching the following two to a UVDBinarySymbol
	//WHATS MY NAME B***!?!?!
	std::string m_name;
	//Not reccomended to access this directly
	//if NULL, section does not have any data associated with it
	//We own this
	UVDData *m_data;
	
	//If this represents a memory section, this should be filled in
	//If it doesn't map to an address space, m_space should be NULL
	//We own this
	//UVDAddress m_dataStart;
	//UVD_SECTION_HAS_*
	//uint32_t m_validMembers;
	//Required alignment in bits (powers of 2) if this section needs to be aligned
	//Set to 0 if does not apply
	uint32_t m_alignment;
	
	//Various section info flags
	//Create additional flag members if needed
	uint32_t m_flags;
	
	//Virtual address to be loaded at
	//Valid if m_flags & UVD_SECTION_FLAG_HAS_VMA
	uv_addr_t m_VMA;
	//Only valid for above condition
	//We may have uninitialized storage that is just bulk filled
	//In this case we have a VMASize even when data is null
	//For now, data->size() is dominent if availible, but you should try to keep this synced
	uv_addr_t m_VMASize;
	
	//Keep this out of m_flags for now, but we might consider moving them into it
	//Read access?
	uvd_tri_t m_R;
	//Write access?
	uvd_tri_t m_W;
	//Execute access?
	uvd_tri_t m_X;
	
	/*
	This only includes relocations specified in the binary format, ie not ones we discovered ourself
	This is not enough currently to do actual linking
	as all of these are assumed to be absolute relocations, which may not be the case
	However, currently we only care about where relocations occur for FLIRT, so fix later if we care
	*/
	std::vector<UVDRelocationFixup> m_relocations;
	
	UVDAddressSpace *m_addressSpace;
};

#endif

