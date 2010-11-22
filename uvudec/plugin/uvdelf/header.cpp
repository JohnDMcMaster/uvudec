/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvdelf/object.h"
#include "uvdelf/relocation.h"
#include "uvd/data/data.h"
#include "uvd/util/types.h"
#include "uvd/util/util.h"
#include <vector>
#include <string>
#include <elf.h>
#include <string.h>
#include <typeinfo>

#if 1
#define printf_elf_header_debug(...)
#define ELF_HEADER_DEBUG(x)
#else
#define printf_elf_header_debug(format, ...)		do{ printf("ELF header: " format, ## __VA_ARGS__); fflush(stdout); } while(0)
#define ELF_HEADER_DEBUG(x)		x
#endif

#if 0
UVDStringTableRelocatableElement::UVDStringTableRelocatableElement()
{
	m_stringTable = NULL;
}

UVDStringTableRelocatableElement::UVDStringTableRelocatableElement(UVDElfStringTableSectionHeaderEntry *stringTable, std::string &sTarget)
{
	m_stringTable = stringTable;
	m_sTarget = sTarget;
}

UVDStringTableRelocatableElement::~UVDStringTableRelocatableElement()
{
}

uv_err_t UVDStringTableRelocatableElement::UVDStringTableRelocatableElement::updateDynamicValue()
{
	/*
	Search through the string table until we find our string
	The relocatable is the given offset
	*/
	uv_assert_ret(m_stringTable);
	unsigned int offset = 0;
	for( std::vector<std::string>::iterator iter = m_stringTable->m_stringTable.begin();
			iter != m_stringTable->m_stringTable.end(); ++iter )
	{
		const std::string &s = *iter;
		//Match?
		if( s == m_sTarget )
		{
			//Done!
			m_dynamicValue = offset;
			m_isDynamicValueValid = true;
		}
		//Record offset, taking into account null byte
		offset += s.size() + 1;
	}
	//If we reached here, the string was not located
	//We could consider registering it, but this probably indicates an error
	return UV_DEBUG(UV_ERR_GENERAL);
}
#endif

/*
UVDElfHeaderEntry
*/

UVDElfHeaderEntry::UVDElfHeaderEntry()
{
	m_fileData = NULL;
	m_elf = NULL;
	m_fileRelocatableData = NULL;
}

UVDElfHeaderEntry::~UVDElfHeaderEntry()
{
}

uv_err_t UVDElfHeaderEntry::init()
{
	uv_assert_err_ret(initRelocatableData());
	return UV_ERR_OK;
}

uv_err_t UVDElfHeaderEntry::initRelocatableData()
{
	m_fileRelocatableData = new UVDRelocatableData();
	uv_assert_ret(m_fileRelocatableData);
	return UV_ERR_OK;
}

uv_err_t UVDElfHeaderEntry::updateForWrite()
{
	//Default: nothing to update
	return UV_ERR_OK;
}

uv_err_t UVDElfHeaderEntry::constructForWrite()
{
	//Default: nothing to finalize
	return UV_ERR_OK;
}

uv_err_t UVDElfHeaderEntry::applyRelocationsForWrite()
{
	//Default: nothing to update
	return UV_ERR_OK;
}

/*
UVDElfSectionHeaderEntry
*/

uv_err_t UVDElfSectionHeaderEntry::getUVDElfSectionHeaderEntryCore(const std::string &sSection, UVDElfSectionHeaderEntry **sectionHeaderOut)
{
	UVDElfSectionHeaderEntry *sectionHeader = NULL;

	//printf_debug("creating section %s\n", sSection.c_str());
	
	//String table?
	if( sSection == UVD_ELF_SECTION_SYMBOL_STRING_TABLE
			|| sSection == UVD_ELF_SECTION_SECTION_STRING_TABLE )
	{
		sectionHeader = new UVDElfStringTableSectionHeaderEntry();
	}
	else if( sSection == UVD_ELF_SECTION_SYMBOL_TABLE )
	{
		sectionHeader = new UVDElfSymbolSectionHeaderEntry();
	}
	else if( sSection == UVD_ELF_SECTION_EXECUTABLE )
	{
		sectionHeader = new UVDElfTextSectionHeaderEntry();
	}
	else if( sSection.find(".rel") == 0 )
	{
		sectionHeader = new UVDElfRelocationSectionHeaderEntry();
	}
	else if( sSection == UVD_ELF_SECTION_NULL )
	{
		sectionHeader = new UVDElfSectionHeaderEntry();
		//Because this doesn't have its own type with init right now
		//SHT_NULL would make a reasonable default though
		uv_assert_ret(sectionHeader);
		sectionHeader->setType(SHT_NULL);
	}
	//Default to generic structure
	else
	{
		sectionHeader = new UVDElfSectionHeaderEntry();
	}
	uv_assert_ret(sectionHeader);
	uv_assert_err_ret(sectionHeader->init());
	sectionHeader->setName(sSection);
	
	uv_assert_ret(sectionHeaderOut);
	*sectionHeaderOut = sectionHeader;
	return UV_ERR_OK;
}

uv_err_t UVDElfSectionHeaderEntry::updateForWrite()
{
	std::string name;
	
	uv_assert_err_ret(UVDElfHeaderEntry::updateForWrite());
	
	getName(name);
	printf_debug("Section header %s update for write, m_relevantSectionHeader: 0x%.8X\n", name.c_str(), (unsigned int)m_relevantSectionHeader);
	if( m_relevantSectionHeader )
	{
		uint32_t index = 0;
		
		//We must have the correct link if applicable
		uv_assert_ret(m_elf);
		uv_assert_err_ret(m_elf->getSectionHeaderIndex(m_relevantSectionHeader, &index));
		m_sectionHeader.sh_link = index;
	}

	return UV_ERR_OK;
}	

uv_err_t UVDElfSectionHeaderEntry::setHeaderData(const UVDData *data)
{
	uv_assert_ret(data);
	
	int toRead = sizeof(m_sectionHeader);
	uv_assert_ret(data->read(0, (char *)&m_sectionHeader, toRead) == toRead);

	return UV_ERR_OK;
}

uv_err_t UVDElfSectionHeaderEntry::getHeaderData(UVDData **headerDataOut)
{
	UVDData *headerData = NULL;
		
	//The actual data remains the same using this
	uv_assert_err_ret(UVDDataMemory::getUVDDataMemoryByTransfer((UVDDataMemory **)&headerData,
			(char *)&m_sectionHeader, sizeof(m_sectionHeader),
			false));
	uv_assert_ret(headerData);

	uv_assert_ret(headerDataOut);
	*headerDataOut = headerData;
	return UV_ERR_OK;
}

void UVDElfHeaderEntry::setFileData(UVDData *data)
{
	m_fileData = data;
}

/*
uv_err_t UVDElfHeaderEntry::updateData()
{
	uv_assert_err_ret(updateDataCore());
	//Data is optional
	uv_assert_err_ret(syncDataAfterUpdate());
	
	return UV_ERR_OK;
}

uv_err_t UVDElfHeaderEntry::syncDataAfterUpdate()
{
	//sync the relocatable data just in case
	if( m_fileData )
	{
		uv_assert_ret(m_fileRelocatableData);
		uv_assert_err_ret(m_fileRelocatableData->setData(m_fileData));
	}
	return UV_ERR_OK;
}

uv_err_t UVDElfHeaderEntry::updateDataCore()
{
	return UV_ERR_OK;
}
*/


uv_err_t UVDElfHeaderEntry::getFileData(UVDData **data)
{
	printf_debug("Getting file data\n");
	uv_assert_ret(data);
	//uv_assert_err_ret(updateData());
	//Optional, no assert
	*data = m_fileData;
	return UV_ERR_OK;
}

uv_err_t UVDElfHeaderEntry::getFileRelocatableData(UVDRelocatableData **supportingData)
{
	//FIXME: why aren't we just returning immediatly?  Why the check?
	//Seem sto be for updateData, which is being eliminated
	UVDData *data = NULL;

	printf_debug("Getting file relocatale data\n");
	uv_assert_ret(supportingData);

	uv_assert_err_ret(getFileData(&data));
	if( !data )
	{
		*supportingData = NULL;
	}
	else
	{
		uv_assert_ret(m_fileRelocatableData);
		//Seems the real issue is that we should have set the data early on and updated it as necessary
		if( m_fileRelocatableData->requiresDataSync() )
		//FIXME hack
		//if( typeid(m_fileRelocatableData) != UVDMultiRelocatableData )
		{
			printf_elf_header_debug("setting data\n");
			uv_assert_err_ret(m_fileRelocatableData->transferData(data, FALSE));
			printf_elf_header_debug("Just set hexdump\n");
			ELF_HEADER_DEBUG(m_fileRelocatableData->hexdump());
			printf_elf_header_debug("Just set end hexdump\n");
		}
		*supportingData = m_fileRelocatableData;
		data->hexdump();
		printf_elf_header_debug("set data end\n");
	}
	
	return UV_ERR_OK;
}

/*
uv_err_t UVDElfHeaderEntry::applyRelocations()
{
	return UV_ERR_OK;
}
*/

/*
UVDElfSectionHeaderEntry
*/

UVDElfSectionHeaderEntry::UVDElfSectionHeaderEntry()
{
	m_relevantSectionHeader = NULL;
	memset(&m_sectionHeader, 0, sizeof(m_sectionHeader));
	m_relocationSectionHeader = NULL;
}

UVDElfSectionHeaderEntry::~UVDElfSectionHeaderEntry()
{
}

uv_err_t UVDElfSectionHeaderEntry::getName(std::string &sName)
{
	sName = m_name;
	return UV_ERR_OK;
}

void UVDElfSectionHeaderEntry::setName(const std::string &sName)
{
	m_name = sName;
}

uv_err_t UVDElfSectionHeaderEntry::getName(int *nameIndex)
{
	uv_assert_ret(nameIndex);
	*nameIndex = m_sectionHeader.sh_name;
	return UV_ERR_OK;
}

void UVDElfSectionHeaderEntry::setName(int nameIndex)
{
	m_sectionHeader.sh_name = nameIndex;
}

uv_err_t UVDElfSectionHeaderEntry::getType(int *type)
{
	uv_assert_ret(type);
	*type = m_sectionHeader.sh_type;
	return UV_ERR_OK;
}

void UVDElfSectionHeaderEntry::setType(int type)
{
	m_sectionHeader.sh_type = type;
}

uv_err_t UVDElfSectionHeaderEntry::getSupportingDataSize(uint32_t *sectionSize)
{
	uv_assert_ret(m_fileData);
	return UV_DEBUG(m_fileData->size(sectionSize));
}

uv_err_t UVDElfSectionHeaderEntry::getLinkSection(UVDElfSectionHeaderEntry **relevantSectionHeader)
{
	uv_assert_ret(relevantSectionHeader);
	//This may be null
	*relevantSectionHeader = m_relevantSectionHeader;
	return UV_ERR_OK;
}

void UVDElfSectionHeaderEntry::setLinkSection(UVDElfSectionHeaderEntry *relevantSectionHeader)
{
	//This may be null
	m_relevantSectionHeader = relevantSectionHeader;
}

/*
UVDElfProgramHeaderEntry
*/

UVDElfProgramHeaderEntry::UVDElfProgramHeaderEntry()
{
	memset(&m_programHeader, 0, sizeof(m_programHeader));
}

UVDElfProgramHeaderEntry::~UVDElfProgramHeaderEntry()
{
}

uv_err_t UVDElfProgramHeaderEntry::setHeaderData(const UVDData *data)
{
	uv_assert_ret(data);
	
	int toRead = sizeof(m_programHeader);
	uv_assert_ret(data->read(0, (char *)&m_programHeader, toRead) == toRead);

	return UV_ERR_OK;
}

uv_err_t UVDElfProgramHeaderEntry::getHeaderData(UVDData **headerDataOut)
{
	UVDData *headerData = NULL;
	
	//The actual data remains the same using this
	uv_assert_err_ret(UVDDataMemory::getUVDDataMemoryByTransfer((UVDDataMemory **)&headerData,
			(char *)&m_programHeader, sizeof(m_programHeader),
			false));
	uv_assert_ret(headerData);
	
	uv_assert_ret(headerDataOut);
	*headerDataOut = headerData;
	return UV_ERR_OK;
}

uv_err_t UVDElfProgramHeaderEntry::getSupportingDataSize(uint32_t *sectionSize)
{
	//uv_assert_err_ret(updateData());
	//Supporting data is optional
	if( !m_fileData )
	{
		uv_assert_ret(sectionSize);
		*sectionSize = 0;
	}
	return UV_DEBUG(m_fileData->size(sectionSize));
}

/*
UVDElfTextSectionHeaderEntry
*/

UVDElfTextSectionHeaderEntry::UVDElfTextSectionHeaderEntry()
{
}

UVDElfTextSectionHeaderEntry::~UVDElfTextSectionHeaderEntry()
{
}

uv_err_t UVDElfTextSectionHeaderEntry::init()
{
	uv_assert_err_ret(UVDElfSectionHeaderEntry::init());
	setType(SHT_PROGBITS);
	return UV_ERR_OK;
}

uv_err_t UVDElfTextSectionHeaderEntry::constructForWrite()
{
	printf_debug(".text update core\n");
	/*
	Collect all of the symbols in relocatable (relocations 0'd form) and concatentate
	*/
	std::vector<UVDData *> dataVector;
	UVDElfSymbolSectionHeaderEntry *symbolSection = NULL;
	
	uv_assert_err_ret(UVDElfHeaderEntry::constructForWrite());
	
	uv_assert_ret(m_elf);
	uv_assert_err_ret(m_elf->getSymbolTableSectionHeaderEntry(&symbolSection));
	
	/*
	NOTE: not all symbols are defined
	If they aren't, they won't have a symbol->m_relocatableData.m_data
	*/
	//printf_debug("num symbols: %d\n", symbolSection->m_symbols.size());	
	for( std::vector<UVDElfSymbol *>::iterator iter = symbolSection->m_symbols.begin();
			iter != symbolSection->m_symbols.end(); ++iter )
	{
		UVDElfSymbol *symbol = *iter;
		UVDData *symbolData = NULL;
		UVDRelocatableData *relocatableData = NULL;
		
		uv_assert_ret(symbol);
		relocatableData = &(symbol->m_relocatableData);
		uv_assert_ret(relocatableData);
		//Skip undefined symbols, they don't form the data set
		uint32_t isEmpty = 0;
		uv_assert_err_ret(relocatableData->isEmpty(&isEmpty));
		if( isEmpty )
		{
			continue;
		}
		uv_assert_err_ret(relocatableData->getDefaultRelocatableData(&symbolData));
		uv_assert_ret(symbolData);
		
		//printf_debug("Symbol dtata 0x%.8X\n", (unsigned int)symbolData);
		dataVector.push_back(symbolData);
	}

	uv_assert_err_ret(UVDData::concatenate(dataVector, &m_fileData));
	//printf(".text table (m_fileData):\n");
	//m_fileData->hexdump();

	return UV_ERR_OK;
}

/*
UVDElfSectionHeaderEntry
*/

uv_err_t UVDElfSectionHeaderEntry::useRelocatableSection()
{
	std::string sectionName;
	std::string relocationSectionName;
	UVDElfSymbolSectionHeaderEntry *symbolSection = NULL;

	//Already present?
	if( m_relocationSectionHeader )
	{
		return UV_ERR_OK;
	}
	
	uv_assert_ret(m_elf);

	uv_assert_err_ret(m_elf->getSymbolTableSectionHeaderEntry(&symbolSection));
	uv_assert_ret(symbolSection);

	//Set other section name to .rel<section name>
	//note section name usually starts with .
	uv_assert_err_ret(getName(sectionName));
	relocationSectionName = std::string(".rel") + sectionName;


	//note a relocation sections sh_link is the symbol table, not the section to relocate on

	m_relocationSectionHeader = new UVDElfRelocationSectionHeaderEntry();
	//uv_assert_err_ret(UVDElfSectionHeaderEntry::getUVDElfSectionHeaderEntryCore(relocationSectionName, &m_relocationSectionHeader));

	uv_assert_ret(m_relocationSectionHeader);
	uv_assert_err_ret(m_relocationSectionHeader->init());
	uv_assert_err_ret(m_relocationSectionHeader->setSymbolSection(symbolSection));
	uv_assert_err_ret(m_relocationSectionHeader->setRelocationSection(this));
	
	m_relocationSectionHeader->setName(relocationSectionName);

	m_relocationSectionHeader->setType(SHT_REL);
	
	//Add the link entry...because we can I guess
	//I'm not sure its real purpose, but this seems to be a case where its required
	//TODO: research the above.  Why did I come to this conclusion?  Did objdump break without it?
	uv_assert_err_ret(symbolSection->addSectionSymbol(sectionName));

	uv_assert_err_ret(m_elf->addSectionHeaderSection(m_relocationSectionHeader));
	
	return UV_ERR_OK;
}

uv_err_t UVDElfSectionHeaderEntry::getRelocatableSection(UVDElfRelocationSectionHeaderEntry **entry)
{
	uv_assert_ret(entry);
	//This will be null if its not a relocatable section
	*entry = m_relocationSectionHeader;
	return UV_ERR_OK;
}

