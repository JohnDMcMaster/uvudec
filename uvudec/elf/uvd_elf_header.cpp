/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#include "uvd_elf.h"
#include "uvd_elf_relocation.h"
#include "uvd_data.h"
#include "uvd_types.h"
#include "uvd_util.h"
#include <elf.h>
#include <vector>
#include <string>

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


/*
UVDElfHeaderEntry
*/

UVDElfHeaderEntry::UVDElfHeaderEntry()
{
	//m_headerData = NULL;
	m_fileData = NULL;
	m_elf = NULL;
}

UVDElfHeaderEntry::~UVDElfHeaderEntry()
{
}

uv_err_t UVDElfSectionHeaderEntry::getUVDElfSectionHeaderEntryCore(const std::string &sSection, UVDElfSectionHeaderEntry **sectionHeaderOut)
{
	UVDElfSectionHeaderEntry *sectionHeader = NULL;
	
	//String table?
	if( sSection == UVD_ELF_SECTION_SYMBOL_STRING_TABLE
			|| sSection == UVD_ELF_SECTION_SECTION_STRING_TABLE )
	{
		sectionHeader = new UVDElfStringTableSectionHeaderEntry();
		uv_assert_ret(sectionHeader);
		sectionHeader->setType(SHT_STRTAB);
	}
	else if( sSection == UVD_ELF_SECTION_SYMBOL_TABLE )
	{
		sectionHeader = new UVDElfSymbolSectionHeaderEntry();
	}
	else if( sSection == UVD_ELF_SECTION_EXECUTABLE )
	{
		sectionHeader = new UVDElfTextSectionHeaderEntry();
	}
	//Default to generic structure
	else
	{
		sectionHeader = new UVDElfSectionHeaderEntry();
	}
	uv_assert_ret(sectionHeader);
	sectionHeader->setName(sSection);
	
	uv_assert_ret(sectionHeaderOut);
	*sectionHeaderOut = sectionHeader;
	return UV_ERR_OK;
}

uv_err_t UVDElfProgramHeaderEntry::setHeaderData(const UVDData *data)
{
	uv_assert_ret(data);
	
	int toRead = sizeof(m_programHeader);
	uv_assert_ret(data->read(0, (char *)&m_programHeader, toRead) == toRead);

	return UV_ERR_OK;
}

uv_err_t UVDElfSectionHeaderEntry::setHeaderData(const UVDData *data)
{
	uv_assert_ret(data);
	
	int toRead = sizeof(m_sectionHeader);
	uv_assert_ret(data->read(0, (char *)&m_sectionHeader, toRead) == toRead);

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

uv_err_t UVDElfHeaderEntry::getFileData(UVDData **data)
{
	uv_assert_ret(data);
	//Optional, no assert
	*data = m_fileData;
	return UV_ERR_OK;
}

uv_err_t UVDElfProgramHeaderEntry::getSupportingDataSize(uint32_t *sectionSize)
{
	uv_assert_ret(m_fileData);
	return UV_DEBUG(m_fileData->size(sectionSize));
}

/*
UVDElfSectionHeaderEntry
*/

UVDElfSectionHeaderEntry::UVDElfSectionHeaderEntry()
{
	memset(&m_sectionHeader, 0, sizeof(m_sectionHeader));
}

UVDElfSectionHeaderEntry::~UVDElfSectionHeaderEntry()
{
}

uv_err_t UVDElfSectionHeaderEntry::getName(std::string &sName)
{
	sName = m_sName;
	return UV_ERR_OK;
}

void UVDElfSectionHeaderEntry::setName(const std::string &sName)
{
	m_sName = sName;
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

#if 0
uv_err_t UVDElfSectionHeaderEntry::getVirtualAddress(int *address)
{
	uv_assert_ret(type);
	*address = m_sectionHeader.sh_addr;
	return UV_ERR_OK;
}

void UVDElfSectionHeaderEntry::setVirtualAddress(int address)
{
	m_sectionHeader.sh_addr = address;
}

uv_err_t UVDElfSectionHeaderEntry::getPhysicalAddress(int *address)
{
	uv_assert_ret(type);
	*nameIndex = m_sectionHeader.sh_name;
	return UV_ERR_OK;
}

void UVDElfSectionHeaderEntry::setPhysicalAddress(int address)
{
	m_sectionHeader.sh_name = nameIndex;
}

uv_err_t UVDElfSectionHeaderEntry::getMemorySize(int *size)
{
	uv_assert_ret(type);
	*nameIndex = m_sectionHeader.sh_name;
	return UV_ERR_OK;
}

void UVDElfSectionHeaderEntry::setMemorySize(int size)
{
	m_sectionHeader.sh_name = nameIndex;
}

uv_err_t UVDElfSectionHeaderEntry::getFlags(int *flags)
{
	uv_assert_ret(type);
	*flags = m_sectionHeader.sh_flags;
	return UV_ERR_OK;
}

void UVDElfSectionHeaderEntry::setFlags(int flags)
{
	m_sectionHeader.sh_flags = flags;
}

uv_err_t UVDElfSectionHeaderEntry::getAlignment(int *alignment)
{
	uv_assert_ret(type);
	*alignment = m_sectionHeader.sh_addralign;
	return UV_ERR_OK;
}

void UVDElfSectionHeaderEntry::setAligntment(int alignment)
{
	m_sectionHeader.sh_addralign = alignment;
}
#endif

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

#if 0
uv_err_t UVDElfProgramHeaderEntry::getName(int *name)
{
}

uv_err_t UVDElfProgramHeaderEntry::getName(std::string &sName)
{
}

void UVDElfProgramHeaderEntry::setName(int name)
{
}

void UVDElfProgramHeaderEntry::setName(const std::string &sName)
{
}

uv_err_t UVDElfProgramHeaderEntry::getType(int *type)
{
}

void UVDElfProgramHeaderEntry::setType(int type)
{
}

uv_err_t UVDElfProgramHeaderEntry::getFlags(int *flags)
{
}

void UVDElfProgramHeaderEntry::setFlags(int flags)
{
}

uv_err_t UVDElfProgramHeaderEntry::getVirtualAddress(int *address)
{
}

void UVDElfProgramHeaderEntry::setVirtualAddress(int address)
{
}

uv_err_t UVDElfProgramHeaderEntry::getSectionLink(int *index)
{
}

void UVDElfProgramHeaderEntry::setSectionLink(int index)
{
}

uv_err_t UVDElfProgramHeaderEntry::getInfo(int *info)
{
}

void UVDElfProgramHeaderEntry::setInfo(int info)
{
}

uv_err_t UVDElfProgramHeaderEntry::getAlignment(int *alignment)
{
}

void UVDElfProgramHeaderEntry::setAlignment(int alignment)
{
}

uv_err_t UVDElfProgramHeaderEntry::getTableEntrySize(int *entrySize)
{
}

void UVDElfProgramHeaderEntry::setTableEntrySize(int entrySize)
{
}

#endif

/*
UVDElfTextSectionHeaderEntry
*/

UVDElfTextSectionHeaderEntry::UVDElfTextSectionHeaderEntry()
{
}

UVDElfTextSectionHeaderEntry::~UVDElfTextSectionHeaderEntry()
{
}

uv_err_t UVDElfTextSectionHeaderEntry::getFileData(UVDData **dataOut)
{
	/*
	Collect all of the symbols in relocatable (relocations 0'd form) and concatentate
	*/
	std::vector<UVDData *> dataVector;
	UVDElfSymbolSectionHeaderEntry *symbolSection = NULL;
	
	uv_assert_ret(m_elf);
	uv_assert_err_ret(m_elf->getSymbolTableSectionHeaderEntry(&symbolSection));
	
	/*
	NOTE: not all symbols are defined
	If they aren't, they won't have a symbol->m_relocatableData.m_data
	*/
	//printf("num symbols: %d\n", symbolSection->m_symbols.size());	
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
		if( !relocatableData->m_data )
		{
			continue;
		}
		uv_assert_err_ret(relocatableData->getDefaultRelocatableData(&symbolData));
		uv_assert_ret(symbolData);
		
		//printf("Symbol dtata 0x%.8X\n", (unsigned int)symbolData);
		dataVector.push_back(symbolData);
	}

	uv_assert_err_ret(UVDData::concatenate(dataVector, dataOut));

	return UV_ERR_OK;
}
