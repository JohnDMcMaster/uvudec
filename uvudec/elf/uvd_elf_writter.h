/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd_elf.h"

#ifndef UVD_ELF_WRITTER_H
#define UVD_ELF_WRITTER_H

/*
#define UVD__ELF_WRITTER__PHASE__UNKNOWN					0
#define UVD__ELF_WRITTER__PHASE__UPDATE_FOR_WRITE			1
//better name for construct
#define UVD__ELF_WRITTER__PHASE__PLACE						2
#define UVD__ELF_WRITTER__PHASE__APPLY_RELOCATIONS			3
*/

class UVDElfWritter
{
public:
	UVDElfWritter();
	~UVDElfWritter();

	uv_err_t init(UVDElf *elf);

	uv_err_t constructBinary(UVDData **data);

	/*
	There are three iterations currently that essentially iterate over all of the same stuff:
	-updateforWrite
		Assemble data
		No operations should depend on the size of any other objects
	-construct
		Sections are assembled into a 
	-applyRelocations
	Pros and cons of a design decision
		It may be beneficial to pass in a phase/iteration parameter and use phase controlled switch statements to keep the 
		interface more regular as opposed to having a function set for each
			A state machines
		However, current approach is less error prone at least on short run, but wwon't scale as well and requires more code
		It is faster
		Quick fix hacks would be easy and easy to follow, but result in largish performance hits if iteration number was used
	*/

	//Phase 1
	//Update all of the members so that it can be written to disk
	uv_err_t updateForWrite();
	uv_err_t updateHeaderForWrite();
	uv_err_t updateProgramHeadersForWrite();
	uv_err_t updateSectionHeadersForWrite();

	//Phase 2
	uv_err_t construct();
	//Elf header
	uv_err_t constructElfHeaderBinary();
	uv_err_t constructProgramHeaderBinaryPhoff(
			UVDRelocatableData *elfHeaderRelocatable);
	uv_err_t constructSectionHeaderBinaryShoff(
			UVDRelocatableData *elfHeaderRelocatable);
	//Section header
	uv_err_t constructProgramHeaderBinary();
	uv_err_t constructProgramHeaderSectionBinary(UVDElfProgramHeaderEntry *entry);
	//Program header
	uv_err_t constructSectionHeaderBinary();
	uv_err_t constructSectionHeaderSectionBinary(UVDElfSectionHeaderEntry *entry);

	//Phase 3
	uv_err_t applyRelocations();
	uv_err_t applyHeaderRelocations();
	uv_err_t applyProgramHeaderRelocations();
	uv_err_t applyProgramHeaderEntryRelocations(UVDElfProgramHeaderEntry *entry);
	uv_err_t applySectionHeaderRelocations();	
	uv_err_t applySectionHeaderSectionRelocations(UVDElfSectionHeaderEntry *entry);

protected:
	uv_err_t hexdump();

public:
	UVDElf *m_elf;
	//For assembling an output executable
	//This is not used during norma
	UVDRelocationManager m_relocationManager;

	UVDRelocatableData *m_programHeaderPlaceholderRelocatableData;
	UVDRelocatableData *m_sectionHeaderPlaceholderRelocatableData;

	//Section/program header supporting data
	//Things such as the string table that are not part of the table and need to be added after the table is constructed
	//Since we construct the section header entries before placing supporting data,
	//these must be stored in a temporary location
	//otherwise, the program/section header tables would become fragmented when should be contiguous 
	std::vector<UVDRelocatableData *> m_headerSupportingData;

	//uint32_t m_phase;
};

#endif

