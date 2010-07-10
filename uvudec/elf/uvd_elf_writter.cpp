/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#include "uvd_elf_writter.h"
#include "uvd_data.h"
#include "uvd_elf.h"
#include "uvd_relocation.h"
#include "uvd_util.h"
#include <stdio.h>
#include <string.h>

#if 1
#define printf_elf_writter_debug(...)
#define ELF_WRITTER_DEBUG(x)
#else
#define printf_elf_writter_debug(format, ...)		do{ printf("ELF writter: " format, ## __VA_ARGS__); fflush(stdout); } while(0)
#define ELF_WRITTER_DEBUG(x)		x
#endif

UVDElfWritter::UVDElfWritter()
{
	m_programHeaderPlaceholderRelocatableData = NULL;
	m_sectionHeaderPlaceholderRelocatableData = NULL;
	//m_phase = UVD__ELF_WRITTER__PHASE__UNKNOW;
}

UVDElfWritter::~UVDElfWritter()
{
}

uv_err_t UVDElfWritter::init(UVDElf *elf)
{
	uv_assert_ret(elf);
	m_elf = elf;
	
	return UV_ERR_OK;
}

uv_err_t UVDElfWritter::constructBinary(UVDData **dataOut)
{
	printf_elf_writter_debug("constructBinary()\n");
	/*
	There are two common techniques for placing relocatables:
	-Two pass approach
		First pass: place the data
		Second pass: query each reolcatable info from the placed data
	-Fixup approach
		As each data item is placed, record locations that need fixup
		Apply all the fixups now that the data is placed
	The fixup approach is used
	It probably resulted in longer code than the two pass approach and is slower,
	but seemed more easy to follow at the time, especially with regards to
	maintaining the code
	*/

	
	//Do we already have a data representation?
	//FIXME: might be cached or previously loaded, make sure cache updated correctly
	uv_assert_ret(dataOut);
	if( m_elf->m_data )
	{
		*dataOut = m_elf->m_data;
		return UV_ERR_OK;
	}
	
	//Phase 1: construct data peices to be assembled
	//These should have as much information filled in as possible with the exception of fields depending on locations in the file
	printf_elf_writter_debug("\n***\nupdateForWrite()\n");
	uv_assert_err_ret(updateForWrite());

	//Phase 2: place all of the data
	//Each section should have all of the information it needs from the previous phase to be sized and placed
	//It is reccomended to construct the data here (as opposed to phase 1) as updateForWrite() call order is not
	//gauranteed and may not have all updates until end
	//getFileRelocatableData() should be ready to be called
	printf_elf_writter_debug("\n***\nconstruct()\n");
	uv_assert_err_ret(construct());
	ELF_WRITTER_DEBUG(hexdump());

	//Phase 3: now that we have placed all of the data, fixup references to the final addresses
	printf_elf_writter_debug("\n***\napplyRelocations()\n");
	uv_assert_err_ret(applyRelocations());
	ELF_WRITTER_DEBUG(hexdump());
	
	//Compute the final object
	printf_elf_writter_debug("\n***\nApplying relocations\n");
	if( UV_FAILED(m_relocationManager.applyPatch(dataOut)) )
	{
		printf_error("Could not apply relocations for ELF object\n");
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	uv_assert_ret(*dataOut);
	//*dataOut = m_elf->m_data;

	//Should we be saving m_data here?

	return UV_ERR_OK;
}

#if 0
Looking closer this might not be such a good idea
Certain areas need special ordering and structure might actually in fact prove harder to follow
uv_err_t doPhase(uint32_t phase)
{
	m_phase = phase;
	/*
	uv_assert_err_ret(updateHeaderForPhase());
	uv_assert_err_ret(updateProgramHeadersForPhase());
	uv_assert_err_ret(updateSectionHeadersForPhase());	
	*/
	
	switch(m_phase)
	{
	case UVD__ELF_WRITTER__PHASE__UPDATE_FOR_WRITE:
		//Phase 1: construct data peices to be assembled
		//These should have as much information filled in as possible with the exception of fields depending on locations in the file
		uv_assert_err_ret(updateForWrite());
		break;

	case UVD__ELF_WRITTER__PHASE__PLACE:
		//Phase 2: place all of the data
		uv_assert_err_ret(construct());
		break;

	case UVD__ELF_WRITTER__PHASE__APPLY_RELOCATIONS:
		//Phase 3: now that we have placed all of the data, fixup references to the final addresses
		uv_assert_err_ret(applyRelocations());
		break;
	
	default:
		return UV_DEBUG(UV_ERR_GENERAL);
	};
	
	return UV_ERR_OK;
}
#endif

uv_err_t UVDElfWritter::construct()
{
	//Construct the header
	uv_assert_err_ret(constructElfHeaderBinary());	
	//The program header table + supporting data
	uv_assert_err_ret(constructProgramHeaderBinary());
	//And the section header table + supporting data
	uv_assert_err_ret(constructSectionHeaderBinary());	

	//Now pack in the sections we were waiting on after the headers
	for( std::vector<UVDRelocatableData *>::size_type i = 0; i < m_headerSupportingData.size(); ++i )
	{
		UVDRelocatableData *relocatableData = m_headerSupportingData[i];
		uv_assert_ret(relocatableData);
		m_relocationManager.addRelocatableData(relocatableData);
		printf_debug("adding supporting data 0x%.8X\n", (unsigned int)relocatableData);
	}
	//These have been processed/transfered
	//FIXME: Why is this commented out?  Because its uncessary?
	//m_headerSupportingData.clear();

	return UV_ERR_OK;
}

/*
Elf header
*/

uv_err_t UVDElfWritter::constructElfHeaderBinary()
{
	//Header decls
	UVDDataMemory *elfHeaderData = NULL;
	UVDRelocatableData *elfHeaderRelocatable = NULL;
	
	//This must be the actual data to get relocations correct,
	//and is more efficient
	uv_assert_err_ret(UVDDataMemory::getUVDDataMemoryByTransfer(&elfHeaderData,
			(char *)&m_elf->m_elfHeader, sizeof(m_elf->m_elfHeader), false));
	uv_assert_ret(elfHeaderData);
	elfHeaderRelocatable = new UVDRelocatableData();
	uv_assert_ret(elfHeaderRelocatable);
	uv_assert_err_ret(elfHeaderRelocatable->transferData(elfHeaderData, true));
	m_relocationManager.addRelocatableData(elfHeaderRelocatable);

	return UV_ERR_OK;
}

/*
Program header
*/

uv_err_t UVDElfWritter::constructProgramHeaderBinary()
{
	/*
	Insert dummy element placeholders so that we can find the table start
	It is expected the entire header table will be placed after this
	*/
	uv_assert_err_ret(UVDRelocatableData::getUVDRelocatableDataPlaceholder(&m_programHeaderPlaceholderRelocatableData));
	uv_assert_ret(m_programHeaderPlaceholderRelocatableData);
	m_relocationManager.addRelocatableData(m_programHeaderPlaceholderRelocatableData);

	//Program header entries
	for( std::vector<UVDElfProgramHeaderEntry *>::size_type i = 0; i < m_elf->m_programHeaderEntries.size(); ++i )
	{
		uv_assert_err_ret(constructProgramHeaderSectionBinary(m_elf->m_programHeaderEntries[i]));
	}

	m_elf->m_elfHeader.e_phnum = m_elf->m_programHeaderEntries.size();

	return UV_ERR_OK;
}

uv_err_t UVDElfWritter::constructProgramHeaderSectionBinary(UVDElfProgramHeaderEntry *entry)
{
	//Raw data peices
	UVDData *headerData = NULL;
	//Containers
	UVDRelocatableData *headerRelocatable = NULL;
	UVDRelocatableData *supportingRelocatable = NULL;
	
	printf_warn("XXX THIS CODE PROBABLY NEEDS FIXING TO MATCH SH\n");

	uv_assert_ret(entry);
	uv_assert_err_ret(entry->constructForWrite());

	//Allocate memory for the program header	
	uv_assert_err_ret(entry->getHeaderData(&headerData));
	uv_assert_ret(headerData);
	headerRelocatable = new UVDRelocatableData();
	uv_assert_ret(headerRelocatable);
	uv_assert_err_ret(headerRelocatable->transferData(headerData, false));
	
	//Store the header data
	m_relocationManager.addRelocatableData(headerRelocatable);
	//Store the supporting data, if it is needed
	uv_assert_err_ret(entry->getFileRelocatableData(&supportingRelocatable));

	/*
	If there is supporting data, fill in the address
	Otherwise, assume address was 0'd from the earlier read request
	*/
	if( supportingRelocatable )
	{
		//Add the actual data in after we have placed headers
		m_headerSupportingData.push_back(supportingRelocatable);
			
		//size
		//This is done after update because we need to wait for sizes to settle after things like string tables are being constructed
		uint32_t sectionSize = 0;
		uv_assert_err_ret(entry->getSupportingDataSize(&sectionSize));
		entry->m_programHeader.p_filesz = sectionSize;
	}

	return UV_ERR_OK;
}

/*
Section header
*/

uv_err_t UVDElfWritter::constructSectionHeaderBinary()
{
	printf_elf_writter_debug("constructSectionHeaderBinary: num sections: %d\n", m_elf->m_sectionHeaderEntries.size());
	uv_assert_err_ret(UVDRelocatableData::getUVDRelocatableDataPlaceholder(&m_sectionHeaderPlaceholderRelocatableData));
	uv_assert_ret(m_sectionHeaderPlaceholderRelocatableData);
	m_relocationManager.addRelocatableData(m_sectionHeaderPlaceholderRelocatableData);

	//Section header entries
	// sh_name, sh_offset, sh_link need relocation
	//ignore sh_link for now
	for( std::vector<UVDElfSectionHeaderEntry *>::size_type i = 0; i < m_elf->m_sectionHeaderEntries.size(); ++i )
	{
		UVDElfSectionHeaderEntry *sectionHeaderEntry = m_elf->m_sectionHeaderEntries[i];
		
		uv_assert_ret(sectionHeaderEntry);
	printf_elf_writter_debug("Constructing %s\n", sectionHeaderEntry->m_name.c_str());
		if( UV_FAILED(constructSectionHeaderSectionBinary(sectionHeaderEntry)) )
		{
			std::string name;
			sectionHeaderEntry->getName(name);
			printf_error("failed section: %s\n", name.c_str());
			return UV_DEBUG(UV_ERR_GENERAL);
		}
	printf_elf_writter_debug("construcSectionHeaderBinary hexdump\n");
	ELF_WRITTER_DEBUG(hexdump());
	}
	m_elf->m_elfHeader.e_shnum = m_elf->m_sectionHeaderEntries.size();

	return UV_ERR_OK;
}

uv_err_t UVDElfWritter::constructSectionHeaderSectionBinary(UVDElfSectionHeaderEntry *entry)
{
	//Raw data peices
	UVDData *headerData = NULL;
	//Containers
	UVDRelocatableData *headerRelocatable = NULL;
	//UVDRelocatableData *linkRelocatable = NULL;
	UVDRelocatableData *supportingRelocatable = NULL;

	printf_elf_writter_debug("Constructing section %s\n", entry->m_name.c_str());
	
	uv_assert_ret(entry);
	uv_assert_err_ret(entry->constructForWrite());

	//Allocate memory for the section header	
	uv_assert_err_ret(entry->getHeaderData(&headerData));
	uv_assert_ret(headerData);
	headerRelocatable = new UVDRelocatableData();
	uv_assert_ret(headerRelocatable);
	uv_assert_err_ret(headerRelocatable->transferData(headerData, false));
	
	//printf_debug("Constructing section %s\n", entry->m_name.c_str());

	//Store the header data
	m_relocationManager.addRelocatableData(headerRelocatable);
	//Store the supporting data, if it is needed
	uv_assert_err_ret(entry->getFileRelocatableData(&supportingRelocatable));
	/*
	If there is supporting data, fill in the address
	Otherwise, assume address was 0'd from the earlier read request
	*/
	printf_debug("considering adding supporting data for section %s 0x%.8X\n", entry->m_name.c_str(), (unsigned int)supportingRelocatable);
	uv_assert_ret(entry->m_name != ".text" || supportingRelocatable);
	uv_assert_ret(entry->m_name != ".rel.text" || supportingRelocatable);
	uv_assert_ret(entry->m_name != ".symtab" || supportingRelocatable);
	uv_assert_ret(entry->m_name != ".strtab" || supportingRelocatable);

	if( supportingRelocatable )
	{
		/*
		{
			UVDData *data = NULL;
			uv_assert_err_ret(supportingRelocatable->getRelocatableData(&data));
			uv_assert_ret(data);
		}
		*/
	
		//Add it in after we have placed headers to preserve table order
		m_headerSupportingData.push_back(supportingRelocatable);
		//printf_debug("supporting data (relocatable = 0x%.8X) size: 0x%.8X\n", (unsigned int)supportingRelocatable, supportingData->size());
		
		//sh_size		
		//Why was this a relocatable element?
		uint32_t sectionSize = 0;
		uv_assert_err_ret(entry->getSupportingDataSize(&sectionSize));
		entry->m_sectionHeader.sh_size = sectionSize;
	}

//DEBUG_BREAK();

	return UV_ERR_OK;
}

uv_err_t UVDElfWritter::updateForWrite()
{
	uv_assert_err_ret(updateHeaderForWrite());
	uv_assert_err_ret(updateProgramHeadersForWrite());
	uv_assert_err_ret(updateSectionHeadersForWrite());	
	return UV_ERR_OK;
}

uv_err_t UVDElfWritter::updateHeaderForWrite()
{
	/*
	Set padding
	This should be set to 0
	Based on value of EI_PAD
	Issues from elf.h defining additional fields that in my spec doc say are reserved and should be set to 0
	Since I don't recognize them, set to garbage
	*/
	uint32_t padRef = 7;
	
	for( uint32_t i = padRef; i < EI_NIDENT; ++i )
	{
		m_elf->m_elfHeader.e_ident[i] = 0;
	}
	m_elf->m_elfHeader.e_phnum = m_elf->m_programHeaderEntries.size();
	return UV_ERR_OK;
}

uv_err_t UVDElfWritter::updateProgramHeadersForWrite()
{
	for( std::vector<UVDElfProgramHeaderEntry *>::size_type i = 0; i < m_elf->m_programHeaderEntries.size(); ++i )
	{
		UVDElfProgramHeaderEntry *entry = m_elf->m_programHeaderEntries[i];
		uv_assert_ret(entry);
		uv_assert_err_ret(entry->updateForWrite());
	}

	return UV_ERR_OK;
}

uv_err_t UVDElfWritter::updateSectionHeadersForWrite()
{
	UVDElfStringTableSectionHeaderEntry *stringTableSection = NULL;

	uv_assert_err_ret(m_elf->getSectionHeaderStringTableEntry(&stringTableSection));
	uv_assert_ret(stringTableSection);

	for( std::vector<UVDElfSectionHeaderEntry *>::size_type i = 0; i < m_elf->m_sectionHeaderEntries.size(); ++i )
	{
		UVDElfSectionHeaderEntry *entry = m_elf->m_sectionHeaderEntries[i];
		std::string sectionName;

		uv_assert_ret(entry);

		uv_assert_err_ret(entry->getName(sectionName));
		stringTableSection->addString(sectionName);

		uv_assert_err_ret(entry->updateForWrite());

#if 0
		{
			UVDData *data = NULL;
			
			std::string name;
			uv_assert_err_ret(entry->getName(name));

			printf_elf_writter_debug("section %s post updateForWrite() m_fileData:\n", name.c_str());
			uv_assert_err_ret(entry->getFileData(&data));
			if( data )
			{
				data->hexdump();
			}
			else
			{
				printf_elf_writter_debug("no data\n");
			}
		}
#endif
	}

	return UV_ERR_OK;
}

uv_err_t UVDElfWritter::applyRelocations()
{
	uv_assert_err_ret(applyHeaderRelocations());
	uv_assert_err_ret(applyProgramHeaderRelocations());
	uv_assert_err_ret(applySectionHeaderRelocations());	
	return UV_ERR_OK;
}

uv_err_t UVDElfWritter::applyHeaderRelocations()
{
	uint32_t offset = 0;
	uint32_t stringTableIndex = 0;

	//How to calculate the value: find our placeholder which is the start of the table
	uv_assert_err_ret(m_relocationManager.getOffset(m_programHeaderPlaceholderRelocatableData, &offset));
	m_elf->m_elfHeader.e_phoff = offset;
	uv_assert_err_ret(m_relocationManager.getOffset(m_sectionHeaderPlaceholderRelocatableData, &offset));
	m_elf->m_elfHeader.e_shoff = offset;

	uv_assert_err_ret(m_elf->getSectionHeaderIndexByName(UVD_ELF_SECTION_SECTION_STRING_TABLE, &stringTableIndex));
	m_elf->m_elfHeader.e_shstrndx = stringTableIndex;

	return UV_ERR_OK;
}

uv_err_t UVDElfWritter::applyProgramHeaderRelocations()
{
	for( std::vector<UVDElfProgramHeaderEntry *>::size_type i = 0; i < m_elf->m_programHeaderEntries.size(); ++i )
	{
		UVDElfProgramHeaderEntry *entry = m_elf->m_programHeaderEntries[i];
		uv_assert_ret(entry);
		uv_assert_err_ret(applyProgramHeaderEntryRelocations(entry));
	}
	
	return UV_ERR_OK;
}

uv_err_t UVDElfWritter::applySectionHeaderRelocations()
{
	for( std::vector<UVDElfSectionHeaderEntry *>::size_type i = 0; i < m_elf->m_sectionHeaderEntries.size(); ++i )
	{
		UVDElfSectionHeaderEntry *entry = m_elf->m_sectionHeaderEntries[i];
		uv_assert_ret(entry);
		uv_assert_err_ret(applySectionHeaderSectionRelocations(entry));
	}
	
	return UV_ERR_OK;
}

uv_err_t UVDElfWritter::applyProgramHeaderEntryRelocations(UVDElfProgramHeaderEntry *entry)
{
	UVDRelocatableData *supportingRelocatable = NULL;
	
	uv_assert_ret(entry);
	uv_assert_err_ret(entry->applyRelocationsForWrite());

	uv_assert_err_ret(entry->getFileRelocatableData(&supportingRelocatable));
	if( supportingRelocatable )
	{
		uint32_t offset;
		
		uv_assert_err_ret(m_relocationManager.getOffset(supportingRelocatable, &offset));
		entry->m_programHeader.p_offset = offset;
	}
	else
	{
		entry->m_programHeader.p_offset = 0;
	}

	return UV_ERR_OK;
}

uv_err_t UVDElfWritter::applySectionHeaderSectionRelocations(UVDElfSectionHeaderEntry *entry)
{
	UVDRelocatableData *supportingRelocatable = NULL;
	uint32_t offset = 0;
	
	uv_assert_ret(entry);

	uv_assert_err_ret(entry->applyRelocationsForWrite());

	uv_assert_err_ret(m_elf->getSectionHeaderStringIndex(entry->m_name, &offset));
	entry->m_sectionHeader.sh_name = offset;
	
	uv_assert_err_ret(entry->getFileRelocatableData(&supportingRelocatable));
	if( supportingRelocatable )
	{
		uint32_t offset;
		
		uv_assert_err_ret(m_relocationManager.getOffset(supportingRelocatable, &offset));
		entry->m_sectionHeader.sh_offset = offset;
	}
	else
	{
		entry->m_sectionHeader.sh_offset = 0;
	}

	return UV_ERR_OK;
}

uv_err_t UVDElfWritter::hexdump()
{
	printf_elf_writter_debug("Section headers: %d\n", m_elf->m_sectionHeaderEntries.size());
	for( std::vector<UVDElfSectionHeaderEntry *>::size_type i = 0; i < m_elf->m_sectionHeaderEntries.size(); ++i )
	{
		UVDElfSectionHeaderEntry *entry = m_elf->m_sectionHeaderEntries[i];
		UVDRelocatableData *supportingRelocatable = NULL;

		uv_assert_ret(entry);
		
		printf_elf_writter_debug("section header %s\n", entry->m_name.c_str());
		uv_assert_err_ret(entry->getFileRelocatableData(&supportingRelocatable));
		if( supportingRelocatable )
		{
			supportingRelocatable->hexdump();
		}
		printf_elf_writter_debug("\n");
	}
	
	return UV_ERR_OK;
}

