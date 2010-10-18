/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/elf/elf.h"
#include "uvd/data/data.h"
#include "uvd/util/types.h"
#include "uvd/util/util.h"
#include <string>
#include <vector>
#include <elf.h>
#include <stdio.h>

#if 1
#define printf_elf_string_debug(...)
#define ELF_STRING_DEBUG(x)
#else
#define printf_elf_string_debug(format, ...)		do{ printf("ELF string: " format, ## __VA_ARGS__); fflush(stdout); } while(0)
#define ELF_STRING_DEBUG(x)		x
#endif

UVDElfStringTableSectionHeaderEntry::UVDElfStringTableSectionHeaderEntry()
{
}

UVDElfStringTableSectionHeaderEntry::~UVDElfStringTableSectionHeaderEntry()
{
}

uv_err_t UVDElfStringTableSectionHeaderEntry::init()
{
	uv_assert_err_ret(UVDElfSectionHeaderEntry::init());
	setType(SHT_STRTAB);
	return UV_ERR_OK;
}

void UVDElfStringTableSectionHeaderEntry::addString(const std::string &s)
{
	for( uint32_t i = 0; i < m_stringTable.size(); ++i )
	{
		const std::string &cur = m_stringTable[i];
		//Already in there?
		if( cur == s )
		{
			return;
		}
	}
	//Not found, add it
	m_stringTable.push_back(s);
}

uv_err_t UVDElfStringTableSectionHeaderEntry::getStringOffset(const std::string &s, uint32_t *offsetOut)
{
	uint32_t netOffset = 0;
	
	uv_assert_ret(offsetOut);
	
	for( uint32_t i = 0; i < m_stringTable.size(); ++i )
	{
		const std::string &cur = m_stringTable[i];
		//Got it?
		if( cur == s )
		{
			*offsetOut = netOffset;
			return UV_ERR_OK;
		}
		netOffset += cur.size() + 1;
	}
	printf_error("Could not find string: %s in string table section %s\n", s.c_str(), m_name.c_str());
	//Not found
	return UV_DEBUG(UV_ERR_GENERAL);
}

uv_err_t UVDElfStringTableSectionHeaderEntry::constructForWrite()
{
	uv_assert_err_ret(UVDElfHeaderEntry::constructForWrite());

	//FIXME
	//m_elf->m_elfHeader.e_shstrndx = 1;

	//If our data object didn't previously exist, create it so we can fill it in
	//We should only do this once with current architecture
	uv_assert_ret(!m_fileData);
	m_fileData = new UVDDataMemory(0);
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
		printf_elf_string_debug("copying in string: %s\n", s.c_str());
		uv_assert_err_ret(m_fileData->writeData(offset, s.c_str(), bufferSize));		
		offset += bufferSize;
	}
	
	printf_elf_string_debug("copied %d strings into %s string table, data addr = 0x%.8X\n", m_stringTable.size(), m_name.c_str(), (unsigned int)m_fileData);
	ELF_STRING_DEBUG(m_fileData->hexdump());
	
	return UV_ERR_OK;
}

uv_err_t UVDElfStringTableSectionHeaderEntry::getSupportingDataSize(uint32_t *sectionSize)
{
	//This should not be called until after constructForWrite()
	uv_assert_ret(m_fileData);
	return UV_DEBUG(m_fileData->size(sectionSize));
}

