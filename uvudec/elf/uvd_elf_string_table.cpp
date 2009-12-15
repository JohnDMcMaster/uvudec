/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#include "uvd_elf.h"
#include "uvd_data.h"
#include "uvd_types.h"
#include "uvd_util.h"
#include <elf.h>
#include <vector>
#include <string>

UVDElfStringTableSectionHeaderEntry::UVDElfStringTableSectionHeaderEntry()
{
}

UVDElfStringTableSectionHeaderEntry::~UVDElfStringTableSectionHeaderEntry()
{
}

void UVDElfStringTableSectionHeaderEntry::addString(const std::string &sIn, unsigned int *index)
{
	uint32_t netIndex = 0;
	for( uint32_t i = 0; i < m_stringTable.size(); ++i )
	{
		const std::string &sCur = m_stringTable[i];
		//Already in there?
		if( sCur == sIn )
		{
			//Its present, return it
			if( index )
			{
				*index = netIndex;
			}
			return;
		}
		netIndex += sCur.size() + 1;
	}
	//Not found, add it
	if( index )
	{
		*index = netIndex;
	}
	m_stringTable.push_back(sIn);
}

uv_err_t UVDElfStringTableSectionHeaderEntry::updateDataCore()
{

	//If our data object didn't previously exist, create it
	if( m_fileData == NULL )
	{
		m_fileData = new UVDDataMemory(0);
	}
	uv_assert_ret(m_fileData);

	//Construct the data table
	//Is there a way we can check for dirty?
	
	//Check needed size
	uint32_t dataSize = 0;
	//First element should be empty (null) string
	uv_assert_ret(m_stringTable.size() >= 1);
	uv_assert_ret(m_stringTable[0].empty());
	for( std::vector<std::string>::iterator iter = m_stringTable.begin(); iter != m_stringTable.end(); ++iter )
	{
		std::string s = *iter;
		//Include null space
		dataSize += s.size() + 1;
	}
	
	//Reallocate space, if needed
	UVDDataMemory *fileData = dynamic_cast<UVDDataMemory *>(m_fileData);
	uv_assert_err_ret(fileData->realloc(dataSize));

	//And copy strings in
	uint32_t offset = 0;
	for( std::vector<std::string>::iterator iter = m_stringTable.begin(); iter != m_stringTable.end(); ++iter )
	{
		std::string s = *iter;
		//Include null space
		uint32_t bufferSize = s.size() + 1;

		uv_assert_err_ret(m_fileData->writeData(offset, s.c_str(), bufferSize));		
		offset += bufferSize;
	}
		
	return UV_ERR_OK;
}

uv_err_t UVDElfStringTableSectionHeaderEntry::getSupportingDataSize(uint32_t *sectionSize)
{
	uv_assert_err_ret(updateData());
	uv_assert_ret(m_fileData);
	return UV_DEBUG(m_fileData->size(sectionSize));
}
