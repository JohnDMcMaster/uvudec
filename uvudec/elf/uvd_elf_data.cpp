/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

/*
ELF I/O related stuff
*/

#include "uvd_data.h"
#include "uvd_elf.h"
#include "uvd_relocation.h"
#include "uvd_util.h"

/*
Elf header
*/

uv_err_t UVDElf::constructElfHeaderBinary(UVDRelocationManager *elfRelocationManager)
{
	//Header decls
	UVDDataMemory *elfHeaderData = NULL;
	UVDRelocatableData *elfHeaderRelocatable = NULL;

	uv_assert_ret(elfRelocationManager);
	
	//This must be the actual data to get relocations correct,
	//and is more efficient
	uv_assert_err_ret(UVDDataMemory::getUVDDataMemoryByTransfer(&elfHeaderData,
			(char *)&m_elfHeader, sizeof(m_elfHeader), false));
	uv_assert_ret(elfHeaderData);
	elfHeaderRelocatable = new UVDRelocatableData();
	uv_assert_ret(elfHeaderRelocatable);
	uv_assert_err_ret(elfHeaderRelocatable->transferData(elfHeaderData));
	elfRelocationManager->addRelocatableData(elfHeaderRelocatable);

	//To compute e_phoff
	uv_assert_err_ret(constructProgramHeaderBinaryPhoff(elfRelocationManager, elfHeaderRelocatable));
	//e_shoff
	uv_assert_err_ret(constructSectionHeaderBinaryShoff(elfRelocationManager, elfHeaderRelocatable));

	return UV_ERR_OK;
}

uv_err_t UVDElf::constructProgramHeaderBinaryPhoff(UVDRelocationManager *elfRelocationManager,
		UVDRelocatableData *elfHeaderRelocatable)
{
	/*
	Insert dummy element placeholder so that we can find the table start
	It is expected the entire header table will be placed after this
	*/

	UVDRelocatableData *placeholderRelocatableData = NULL;
	UVDRelocatableElement *offsetSymbol = NULL;
	UVDRelocationFixup *offsetFixup = NULL;
	
	//The data to find
	uv_assert_err_ret(UVDRelocatableData::getUVDRelocatableDataPlaceholder(&placeholderRelocatableData));
	uv_assert_ret(placeholderRelocatableData);
	
	//How to calculate the value: find our placeholder
	uv_assert_ret(elfRelocationManager);
	offsetSymbol = new UVDSelfLocatingRelocatableElement(elfRelocationManager, placeholderRelocatableData);
	uv_assert_ret(offsetSymbol);
	
	//How to apply the fixup
	offsetFixup = new UVDRelocationFixup(offsetSymbol,
			OFFSET_OF(Elf32_Ehdr, e_phoff), sizeof(m_elfHeader.e_phoff));
	uv_assert_ret(offsetFixup);
	
	uv_assert_ret(elfHeaderRelocatable);
	elfHeaderRelocatable->addFixup(offsetFixup);

	elfRelocationManager->addRelocatableData(placeholderRelocatableData);

	return UV_ERR_OK;
}

uv_err_t UVDElf::constructSectionHeaderBinaryShoff(UVDRelocationManager *elfRelocationManager,
		UVDRelocatableData *elfHeaderRelocatable)
{
	/*
	Insert dummy element placeholder so that we can find the table start
	It is expected the entire header table will be placed after this
	*/

	UVDRelocatableData *placeholderRelocatableData = NULL;
	UVDRelocatableElement *offsetSymbol = NULL;
	UVDRelocationFixup *offsetFixup = NULL;
	
	//The data to find
	uv_assert_err_ret(UVDRelocatableData::getUVDRelocatableDataPlaceholder(&placeholderRelocatableData));
	uv_assert_ret(placeholderRelocatableData);
	
	//How to calculate the value: find our placeholder
	uv_assert_ret(elfRelocationManager);
	offsetSymbol = new UVDSelfLocatingRelocatableElement(elfRelocationManager, placeholderRelocatableData);
	uv_assert_ret(offsetSymbol);
	
	//How to apply the fixup
	offsetFixup = new UVDRelocationFixup(offsetSymbol,
			OFFSET_OF(Elf32_Ehdr, e_shoff), sizeof(m_elfHeader.e_shoff));
	uv_assert_ret(offsetFixup);
	
	uv_assert_ret(elfHeaderRelocatable);
	elfHeaderRelocatable->addFixup(offsetFixup);

	elfRelocationManager->addRelocatableData(placeholderRelocatableData);

	return UV_ERR_OK;
}

/*
Program header
*/

uv_err_t UVDElf::constructProgramHeaderBinary(UVDRelocationManager &elfRelocationManager,
		std::vector<UVDRelocatableData *> &headerSupportingData)
{
	//Program header entries
	for( std::vector<UVDElfProgramHeaderEntry *>::size_type i = 0; i < m_programHeaderEntries.size(); ++i )
	{
		uv_assert_err_ret(constructProgramHeaderSectionBinary(elfRelocationManager,
				headerSupportingData, m_programHeaderEntries[i]));
	}
	m_elfHeader.e_phnum = m_programHeaderEntries.size();
	return UV_ERR_OK;
}

uv_err_t UVDElf::constructProgramHeaderSectionBinary(UVDRelocationManager &elfRelocationManager,
		std::vector<UVDRelocatableData *> &headerSupportingData,
		UVDElfProgramHeaderEntry *entry)
{
	//Raw data peices
	UVDData *headerData = NULL;
	//Containers
	UVDRelocatableData *headerRelocatable = NULL;
	UVDRelocatableData *supportingRelocatable = NULL;
	
	printf_warn("XXX THIS CODE PROBABLY NEEDS FIXING TO MATCH SH\n");

	uv_assert_ret(entry);

	//Allocate memory for the program header	
	entry->getHeaderData(&headerData);
	uv_assert_ret(headerData);
	headerRelocatable = new UVDRelocatableData();
	uv_assert_ret(headerRelocatable);
	uv_assert_err_ret(headerRelocatable->transferData(headerData));
	
	//Store the header data
	elfRelocationManager.addRelocatableData(headerRelocatable);
	//Store the supporting data, if it is needed
	uv_assert_err_ret(entry->getFileRelocatableData(&supportingRelocatable));

	/*
	If there is supporting data, fill in the address
	Otherwise, assume address was 0'd from the earlier read request
	*/
	if( supportingRelocatable )
	{
		//Create a relocation so it can be properly placed
		UVDSelfLocatingRelocatableElement *offsetElement = NULL;
		//We must wrap the data in this, but assume it requires no patchups for now
		UVDRelocationFixup *offsetFixup = NULL;
	
		//We want it to point to the supporting data
		//And of course add the structure that will deteruvd_mine its position
		offsetElement = new UVDSelfLocatingRelocatableElement(&elfRelocationManager, supportingRelocatable);
		uv_assert_ret(offsetElement);
		
		//Add the actual data in after we have placed headers
		headerSupportingData.push_back(supportingRelocatable);
	
		//offset
		//How to compute the value: calculate the offset in our data list
		//Where to apply it
		offsetFixup = new UVDRelocationFixup(offsetElement,
				OFFSET_OF(Elf32_Phdr, p_offset), sizeof(entry->m_programHeader.p_offset));
		headerRelocatable->addFixup(offsetFixup);
		
		//size
		uint32_t sectionSize = 0;
		uv_assert_err_ret(entry->getSupportingDataSize(&sectionSize));
		entry->m_programHeader.p_filesz = sectionSize;
	}
	return UV_ERR_OK;
}

/*
Section header
*/

uv_err_t UVDElf::constructSectionHeaderBinary(UVDRelocationManager &elfRelocationManager,
			std::vector<UVDRelocatableData *> &headerSupportingData)
{
	//Section header entries
	// sh_name, sh_offset, sh_link need relocation
	//ignore sh_link for now
	for( std::vector<UVDElfSectionHeaderEntry *>::size_type i = 0; i < m_sectionHeaderEntries.size(); ++i )
	{
		UVDElfSectionHeaderEntry *sectionHeaderEntry = m_sectionHeaderEntries[i];
		
		uv_assert_ret(sectionHeaderEntry);
		if( UV_FAILED(constructSectionHeaderSectionBinary(elfRelocationManager,
				headerSupportingData, sectionHeaderEntry)) )
		{
			std::string name;
			sectionHeaderEntry->getName(name);
			printf_error("failed section: %s\n", name.c_str());
			return UV_DEBUG(UV_ERR_GENERAL);
		}
	}
	m_elfHeader.e_shnum = m_sectionHeaderEntries.size();

	return UV_ERR_OK;
}

uv_err_t UVDElf::constructSectionHeaderSectionBinary(UVDRelocationManager &elfRelocationManager,
		std::vector<UVDRelocatableData *> &headerSupportingData,
		UVDElfSectionHeaderEntry *entry)
{
	//Raw data peices
	UVDData *headerData = NULL;
	//Containers
	UVDRelocatableData *headerRelocatable = NULL;
	UVDRelocatableElement *nameRelocatable = NULL;
	//UVDRelocatableData *linkRelocatable = NULL;
	UVDRelocationFixup *nameFixup = NULL;
	UVDRelocatableData *supportingRelocatable = NULL;
	
	uv_assert_ret(entry);

	//Allocate memory for the section header	
	entry->getHeaderData(&headerData);
	uv_assert_ret(headerData);
	headerRelocatable = new UVDRelocatableData();
	uv_assert_ret(headerRelocatable);
	uv_assert_err_ret(headerRelocatable->transferData(headerData));
	
	//printf_debug("Constructing section %s\n", entry->m_sName.c_str());

	//Section name
	//Get a token to the location the data will eventually be fixed to
	uv_assert_err_ret(getSectionHeaderStringRelocatableElement(entry->m_sName, &nameRelocatable, &elfRelocationManager));
	uint32_t nameOffset = OFFSET_OF(Elf32_Shdr, sh_name);
	uint32_t nameSize = sizeof(Elf32_Word);
	//Create a fixup to that item at given offset within the header chunk
	nameFixup = new UVDRelocationFixup(nameRelocatable, nameOffset, nameSize);
	uv_assert_ret(nameFixup);
	//Register that offset to the header chunk
	headerRelocatable->addFixup(nameFixup);
		
	//Store the header data
	elfRelocationManager.addRelocatableData(headerRelocatable);
	//Store the supporting data, if it is needed
	uv_assert_err_ret(entry->getFileRelocatableData(&supportingRelocatable));
	/*
	If there is supporting data, fill in the address
	Otherwise, assume address was 0'd from the earlier read request
	*/
	printf_debug("considering adding supporting data for section %s 0x%.8X\n", entry->m_sName.c_str(), (unsigned int)supportingRelocatable);
	uv_assert_ret(entry->m_sName != ".text" || supportingRelocatable);
	uv_assert_ret(entry->m_sName != ".rel.text" || supportingRelocatable);
	uv_assert_ret(entry->m_sName != ".symtab" || supportingRelocatable);
	uv_assert_ret(entry->m_sName != ".strtab" || supportingRelocatable);

	if( supportingRelocatable )
	{
		//We must wrap the data in this, but assume it requires no patchups for now
		UVDRelocationFixup *offsetFixup = NULL;
		UVDRelocatableElement *offsetRelocatableElement = NULL;
		UVDRelocatableElement *sizeRelocatableElement = NULL;
		UVDRelocationFixup *sizeFixup = NULL;
	
		{
			UVDData *data = NULL;
			uv_assert_err_ret(supportingRelocatable->getRelocatableData(&data));
			uv_assert_ret(data);
		}
	
		//Add it in after we have placed headers to preserve table order
		headerSupportingData.push_back(supportingRelocatable);
		//printf_debug("supporting data (relocatable = 0x%.8X) size: 0x%.8X\n", (unsigned int)supportingRelocatable, supportingData->size());

		//sh_offset
		//We want it to point to the supporting data
		//And of course add the structure that will deteruvd_mine its position
		offsetRelocatableElement = new UVDSelfLocatingRelocatableElement(&elfRelocationManager, supportingRelocatable);
		uv_assert_ret(offsetRelocatableElement);
		//How to compute the value: calculate the offset in our data list
		//Where to apply it
		offsetFixup = new UVDRelocationFixup(offsetRelocatableElement,
				OFFSET_OF(Elf32_Shdr, sh_offset), sizeof(entry->m_sectionHeader.sh_offset));
		headerRelocatable->addFixup(offsetFixup);
		
		//sh_size		
		sizeRelocatableElement = new UVDSelfSizingRelocatableElement(supportingRelocatable);
		uv_assert_ret(sizeRelocatableElement);
		sizeFixup = new UVDRelocationFixup(sizeRelocatableElement,
				OFFSET_OF(Elf32_Shdr, sh_size), sizeof(entry->m_sectionHeader.sh_size));
		headerRelocatable->addFixup(sizeFixup);
	}
	
	return UV_ERR_OK;
}

uv_err_t UVDElf::updateForWrite()
{
	uv_assert_err_ret(updateHeaderForWrite());
	uv_assert_err_ret(updateProgramHeadersForWrite());
	uv_assert_err_ret(updateSectionHeadersForWrite());	
	return UV_ERR_OK;
}

uv_err_t UVDElf::updateHeaderForWrite()
{
	m_elfHeader.e_phnum = m_programHeaderEntries.size();
	return UV_ERR_OK;
}

uv_err_t UVDElf::updateProgramHeadersForWrite()
{
	for( std::vector<UVDElfProgramHeaderEntry *>::size_type i = 0; i < m_programHeaderEntries.size(); ++i )
	{
		UVDElfProgramHeaderEntry *entry = m_programHeaderEntries[i];
		uv_assert_ret(entry);
		uv_assert_err_ret(entry->updateForWrite());
	}

	return UV_ERR_OK;
}

uv_err_t UVDElf::updateSectionHeadersForWrite()
{
	for( std::vector<UVDElfSectionHeaderEntry *>::size_type i = 0; i < m_sectionHeaderEntries.size(); ++i )
	{
		UVDElfSectionHeaderEntry *entry = m_sectionHeaderEntries[i];
		uv_assert_ret(entry);
		uv_assert_err_ret(entry->updateForWrite());
	}

	return UV_ERR_OK;
}

/*
Top level construction
*/
	
uv_err_t UVDElf::constructBinary(UVDData **dataOut)
{
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

	//Stores all the relocations so we can apply it to the assembled data
	UVDRelocationManager elfRelocationManager;
	//Since we construct the section header entries before placing supporting data,
	//these must be stored in a temporary location
	//otherwise, the program/section header tables would become fragmented when should be contiguous 
	std::vector<UVDRelocatableData *> headerSupportingData;
	
	//Do we already have a data representation?
	//FIXME: might be cached or previously loaded, make sure cache updated correctly
	uv_assert_ret(dataOut);
	if( m_data )
	{
		*dataOut = m_data;
		return UV_ERR_OK;
	}
	
	uv_assert_err_ret(updateForWrite());

	//Construct the header
	uv_assert_err_ret(constructElfHeaderBinary(&elfRelocationManager));	
	//The program header table + supporting data
	uv_assert_err_ret(constructProgramHeaderBinary(elfRelocationManager, headerSupportingData));
	//And the section header table + supporting data
	uv_assert_err_ret(constructSectionHeaderBinary(elfRelocationManager, headerSupportingData));	

	//Now pack in the sections we were waiting on after the headers
	for( std::vector<UVDRelocatableData *>::size_type i = 0; i < headerSupportingData.size(); ++i )
	{
		UVDRelocatableData *relocatableData = headerSupportingData[i];
		uv_assert_ret(relocatableData);
		elfRelocationManager.addRelocatableData(relocatableData);
		printf_debug("adding supporting data 0x%.8X\n", (unsigned int)relocatableData);
	}
	//These have been processed
	//headerSupportingData.clear();

	//Compute the final object
	if( UV_FAILED(elfRelocationManager.applyPatch(dataOut)) )
	{
		printf_error("Could not apply relocations for ELF object\n");
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	uv_assert_ret(*dataOut);

	return UV_ERR_OK;
}

/*
Misc functions
*/

uv_err_t UVDElf::saveToFile(const std::string &file)
{
	UVDData *data = NULL;
	//Get the raw binary data
	uv_assert_err_ret(constructBinary(&data));
	uv_assert_ret(data);
	//And save it
	uv_assert_err_ret(data->saveToFile(file));
	
	return UV_ERR_OK;
}
