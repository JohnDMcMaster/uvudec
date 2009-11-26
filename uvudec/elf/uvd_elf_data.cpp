/*
Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

/*
ELF I/O related stuff
*/

#include "uvd_elf.h"
#include "uvd_data.h"

uv_err_t UVDElf::constructBinary(UVDData **dataOut)
{
	//TODO: this function is big, break it up

	uv_assert_ret(dataOut);
	//Do we alreayd have a data representation?
	//Might be cached or previously loaded
	if( m_data )
	{
		*dataOut = m_data;
		return UV_ERR_OK;
	}
		
	//Apply/refresh all pending relocation operations on our data structures
	for( std::vector<UVDSimpleRelocationFixup *>::iterator iter = m_relocationFixups.begin();
			iter != m_relocationFixups.end(); ++iter )
	{
		UVDSimpleRelocationFixup *fixup = *iter;
		uv_assert_ret(fixup);
		uv_assert_err_ret(fixup->applyPatch());
	}

//printf("num section header: %d\n", m_elfHeader.e_shnum);
//printf("item address: 0x%.8X\n", &(m_elfHeader.e_shnum));
//DEBUG_BREAK();
	
	//Stores all the relocations so we can apply it to the assembled data
	UVDRelocationManager elfRelocationManager;
	//Since we construct the section header entries before placing supporting data, these must be stored in a temporary location
	std::vector<UVDRelocatableData *> headerSupportingData;
	
	//Header
	UVDDataMemory *elfHeaderData = NULL;
	UVDRelocatableData *elfHeaderRelocatable = NULL;
	//To compute ph/sh_offset
	UVDRelocatableData *firstSectionHeaderRelocatableData = NULL;
	UVDRelocatableData *firstProgramHeaderRelocatableData = NULL;
	
	//Header
	//elfHeaderData = new UVDDataMemory((const char *)&m_elfHeader, sizeof(m_elfHeader));
	//This must be the actual data to get relocations correct
	uv_assert_err_ret(UVDDataMemory::getUVDDataMemoryByTransfer(&elfHeaderData,
			(char *)&m_elfHeader, sizeof(m_elfHeader)));

	elfHeaderRelocatable = new UVDRelocatableData(elfHeaderData);
	uv_assert_ret(elfHeaderRelocatable);
	elfRelocationManager.addRelocatableData(elfHeaderRelocatable);

	//Core sections
	
	//TODO: the program and section header loops are virtually the same, reduce the code somehow

	//Program header entries
	for( std::vector<UVDElfProgramHeaderEntry *>::size_type i = 0; i < m_programHeaderEntries.size(); ++i )
	{
		UVDElfProgramHeaderEntry *entry = m_programHeaderEntries[i];
		uv_assert_ret(entry);
		//Raw data peices
		UVDData *headerData = NULL;
		UVDData *supportingData = NULL;
		//Containers
		UVDRelocatableData *headerRelocatable = NULL;
		
		//Allocate memory for the program header	
		entry->getHeaderData(&headerData);
		uv_assert_ret(headerData);
		headerRelocatable = new UVDRelocatableData(headerData);
		uv_assert_ret(headerRelocatable);
		
		if( !firstProgramHeaderRelocatableData )
		{
			firstProgramHeaderRelocatableData = headerRelocatable;
		}

		//Store the header data
		elfRelocationManager.addRelocatableData(headerRelocatable);
		//Store the supporting data, if it is needed
		uv_assert_err_ret(entry->getFileData(&supportingData));
		/*
		If there is supporting data, fill in the address
		Otherwise, assume address was 0'd from the earlier read request
		*/
		if( supportingData )
		{
			//Create a relocation so it can be properly placed
			UVDSelfLocatingRelocatableElement *relocationElement = NULL;
			//We must wrap the data in this, but assume it requires no patchups for now
			UVDRelocatableData *supportingRelocatable = NULL;
		
			supportingRelocatable = new UVDRelocatableData(supportingData);
			uv_assert_ret(supportingRelocatable);

			//We want it to point to the supporting data
			//And of course add the structure that will deteruvd_mine its position
			relocationElement = new UVDSelfLocatingRelocatableElement(&elfRelocationManager, supportingRelocatable);
			uv_assert_ret(relocationElement);
			//Register this symbol
			elfRelocationManager.addRelocatableElement(relocationElement);	
			
			//Add it in after we have placed headers
			headerSupportingData.push_back(supportingRelocatable);
		
		
			//Add stuff so we can locate it
			
			//And its size
			//uv_assert_err_ret(supportingRelocatable->getSize(&entry->m_programHeader.p_filesz));
		}
	}
	m_elfHeader.e_phnum = m_programHeaderEntries.size();
		
	//Section header entries
	// sh_name, sh_offset, sh_link need relocation
	//ignore sh_link for now
	for( std::vector<UVDElfSectionHeaderEntry *>::size_type i = 0; i < m_sectionHeaderEntries.size(); ++i )
	{
		UVDElfSectionHeaderEntry *entry = m_sectionHeaderEntries[i];
		uv_assert_ret(entry);
		//Raw data peices
		UVDData *headerData = NULL;
		UVDData *supportingData = NULL;
		//Containers
		UVDRelocatableData *headerRelocatable = NULL;
		UVDRelocatableElement *nameRelocatable = NULL;
		//UVDRelocatableData *linkRelocatable = NULL;
		UVDRelocationFixup *nameFixup = NULL;
		
		//Allocate memory for the section header	
		entry->getHeaderData(&headerData);
		uv_assert_ret(headerData);
		headerRelocatable = new UVDRelocatableData(headerData);
		uv_assert_ret(headerRelocatable);
		
		//Section name
		//Get a token to the location the data will eventually be fixed to
		uv_assert_err_ret(getSectionHeaderStringRelocatableElement(entry->m_sName, &nameRelocatable, &elfRelocationManager));
		//sh_name is first element, see elf.h
		unsigned int nameOffset = 0;
		unsigned int nameSize = 4 /*sizeof(Elf32_Shdr.sh_name)*/;
		//Create a fixup to that item at given offset within the header chunk
		nameFixup = new UVDRelocationFixup(nameRelocatable, nameOffset, nameSize);
		uv_assert_ret(nameFixup);
		//Register that offset to the header chunk
		headerRelocatable->addFixup(nameFixup);
		
		if( !firstSectionHeaderRelocatableData )
		{
			firstSectionHeaderRelocatableData = headerRelocatable;
		}
		
		//Store the header data
		elfRelocationManager.addRelocatableData(headerRelocatable);
		//Store the supporting data, if it is needed
		uv_assert_err_ret(entry->getFileData(&supportingData));
		/*
		If there is supporting data, fill in the address
		Otherwise, assume address was 0'd from the earlier read request
		*/
		if( supportingData )
		{
			//Create a relocation so it can be properly placed
			UVDSelfLocatingRelocatableElement *relocationElement = NULL;
			//We must wrap the data in this, but assume it requires no patchups for now
			UVDRelocatableData *supportingRelocatable = NULL;
		
			supportingRelocatable = new UVDRelocatableData(supportingData);
			uv_assert_ret(supportingRelocatable);

			//We want it to point to the supporting data
			//And of course add the structure that will deteruvd_mine its position
			relocationElement = new UVDSelfLocatingRelocatableElement(&elfRelocationManager, supportingRelocatable);
			uv_assert_ret(relocationElement);
			//Register this symbol
			elfRelocationManager.addRelocatableElement(relocationElement);
			
			//Add it in after we have placed headers
			headerSupportingData.push_back(supportingRelocatable);
		}
	}
	m_elfHeader.e_shnum = m_sectionHeaderEntries.size();
	
	
	//Table offsets
	//Fixup data to the location of the first table element
	//Section header offset
	UVDSimpleRelocationFixup *sectionHeaderOffsetFixup = NULL;
	UVDRelocatableElement *sectionHeaderOffsetSymbol = NULL;
	uv_assert_ret(firstSectionHeaderRelocatableData);
	sectionHeaderOffsetSymbol = new UVDSelfLocatingRelocatableElement(&elfRelocationManager, firstSectionHeaderRelocatableData);
	uv_assert_err_ret(UVDSimpleRelocationFixup::getUVDSimpleRelocationFixup(
			&sectionHeaderOffsetFixup, sectionHeaderOffsetSymbol,
			(char *)&m_elfHeader.e_shoff, sizeof(m_elfHeader.e_shoff)));
	uv_assert_err_ret(sectionHeaderOffsetFixup->applyPatch());
	//m_relocationFixups.push_back(sectionHeaderOffsetFixup);


	//Now pack in the sections we were waiting on after the headers
	for( std::vector<UVDRelocatableData *>::size_type i = 0; i < headerSupportingData.size(); ++i )
	{
		UVDRelocatableData *relocatableData = headerSupportingData[i];
		uv_assert_ret(relocatableData);
		elfRelocationManager.addRelocatableData(relocatableData);
	}
	//These have been processed
	headerSupportingData.clear();
	
	//Compute the final object
	uv_assert_err_ret(elfRelocationManager.applyPatch(dataOut));
	uv_assert_ret(*dataOut);

	return UV_ERR_OK;
}

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

