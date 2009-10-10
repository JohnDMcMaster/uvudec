/*
Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under the terms of the BSD license.  See LICENSE for details.
*/


#include "uvd_elf.h"
#include "uvd_data.h"
#include "uvd_types.h"
#include "uv_util.h"
#include <elf.h>

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
	
	headerData = new UVDDataMemory((const char *)&m_programHeader, sizeof(m_programHeader));
	uv_assert_ret(headerData);
	
	uv_assert_ret(headerDataOut);
	*headerDataOut = headerData;
	return UV_ERR_OK;
}

uv_err_t UVDElfSectionHeaderEntry::getHeaderData(UVDData **headerDataOut)
{
	UVDData *headerData = NULL;
	
	headerData = new UVDDataMemory((const char *)&m_sectionHeader, sizeof(m_sectionHeader));
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

/*
UVDElfSectionHeaderEntry
*/

UVDElfSectionHeaderEntry::UVDElfSectionHeaderEntry()
{
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

UVDElf::UVDElf()
{
	m_data = NULL;
}

UVDElf::~UVDElf()
{
}

uv_err_t UVDElf::getFromFile(const std::string &sFile, UVDElf **elfOut)
{
	UVDElf *elf = NULL;
	
	elf = new UVDElf();
	uv_assert_ret(elf);
	
	uv_assert_err_ret(elf->loadFromFile(sFile));
	
	uv_assert_ret(elfOut);
	*elfOut = elf;
	return UV_ERR_OK;
}
	
uv_err_t UVDElf::loadFromFile(const std::string &sFile)
{
	uv_assert_err_ret(UVDDataFile::getUVDDataFile(&m_data, sFile));
	uv_assert_ret(m_data);

	//Read in the ELF header
	uv_assert_ret(m_data->read(0, (char *)&m_elfHeader, sizeof(Elf32_Ehdr)) == sizeof(Elf32_Ehdr));

	//Check for magic
	if( m_elfHeader.e_ident[EI_MAG0] != ELFMAG0
			|| m_elfHeader.e_ident[EI_MAG1] != ELFMAG1
			|| m_elfHeader.e_ident[EI_MAG2] != ELFMAG2
			|| m_elfHeader.e_ident[EI_MAG3] != ELFMAG3 )
	{
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	
	uv_assert_ret(m_elfHeader.e_ehsize < sizeof(Elf32_Ehdr));
	
	//Extra header data
	int toRead = m_elfHeader.e_ehsize - sizeof(Elf32_Ehdr);
	uv_assert_ret(m_data->read(sizeof(Elf32_Ehdr), (char *)&m_elfHeaderExtra, toRead) == toRead);

	//Read in program header sections
	for( int header_index = 0; header_index < m_elfHeader.e_phnum; ++header_index )
	{
		int start = m_elfHeader.e_phoff + header_index * m_elfHeader.e_phentsize;
		UVDElfProgramHeaderEntry *pSection = NULL;

		toRead = uvd_min(sizeof(Elf32_Phdr), m_elfHeader.e_phentsize);

		pSection = new UVDElfProgramHeaderEntry();
		uv_assert_ret(pSection);
		
		//Create a reference to the data
		UVDDataChunk *dataChunk = NULL;
		dataChunk = new UVDDataChunk();
		uv_assert_ret(dataChunk);
		pSection->setHeaderData(dataChunk);
		uv_assert_err_ret(dataChunk->init(m_data, start, start + toRead));
	
		//Data will be at uvd_max sizeof(program_header) as per above uvd_min() statement
		dataChunk->read(0, (char *)&pSection->m_programHeader, sizeof(pSection->m_programHeader));

		m_programHeaderEntries.push_back(pSection);
	}

	//Read in section header sections
	for( int header_index = 0; header_index < m_elfHeader.e_phnum; ++header_index )
	{
		int start = m_elfHeader.e_phoff + header_index * m_elfHeader.e_phentsize;
		UVDElfSectionHeaderEntry *pSection = NULL;

		toRead = uvd_min(sizeof(Elf32_Shdr), m_elfHeader.e_shentsize);

		pSection = new UVDElfSectionHeaderEntry();
		uv_assert_ret(pSection);
		
		//Create a reference to the data
		UVDDataChunk *dataChunk = NULL;
		dataChunk = new UVDDataChunk();
		uv_assert_ret(dataChunk);
		pSection->setHeaderData(dataChunk);
		uv_assert_err_ret(dataChunk->init(m_data, start, start + toRead));
		
		//Data will be at uvd_max sizeof(section_header) as per above uvd_min() statement
		dataChunk->read(0, (char *)&pSection->m_sectionHeader, sizeof(pSection->m_sectionHeader));
	
		m_sectionHeaderEntries.push_back(pSection);
	}

	return UV_ERR_OK;
}

uv_err_t UVDElf::init()
{
	/*
	Setup default sections
	We could discard these later if needed
	*/
	UVDElfSectionHeaderEntry *nullSection = NULL;
	UVDElfSectionHeaderEntry *stringTableSection = NULL;
	//XXX: should add SHT_SYMTAB entry?

	//Fill in some defaults
	uv_assert_err_ret(initHeader());

	//Add the null section
	//According to TIS, only SHT_NULL is defined, other fields are undefined
	nullSection = new UVDElfSectionHeaderEntry();
	uv_assert_ret(nullSection);
	nullSection->setType(SHT_NULL);
	uv_assert_err_ret(addSectionHeaderSection(nullSection));

	//Add string table section
	stringTableSection = new UVDElfStringTableSectionHeaderEntry();
	uv_assert_ret(stringTableSection);
	stringTableSection->setType(SHT_STRTAB);
	uv_assert_err_ret(addSectionHeaderSection(stringTableSection));
	//Register the index (only SHT_NULL was before this)
	m_elfHeader.e_shstrndx = 1;

	//Null string in string table must be first element
	uv_assert_err_ret(addString(""));	

	return UV_ERR_OK;
}

uv_err_t UVDElf::initHeader()
{
	//Header
	//Magic
	m_elfHeader.e_ident[EI_MAG0] = ELFMAG0;
	m_elfHeader.e_ident[EI_MAG1] = ELFMAG1;
	m_elfHeader.e_ident[EI_MAG2] = ELFMAG2;
	m_elfHeader.e_ident[EI_MAG3] = ELFMAG3;
	//Data class
	m_elfHeader.e_ident[EI_CLASS] = ELFCLASS32;
	//Data encoding...arbitrarily set to little endian since thats what GCC does
	m_elfHeader.e_ident[EI_DATA] = ELFDATA2LSB;
	//Must be set to this value
	m_elfHeader.e_ident[EI_VERSION] = EV_CURRENT;
	//Reserved, must be set to 0
	for( int i = EI_NIDENT; i < EI_PAD; ++i )
	{
		m_elfHeader.e_ident[i] = 0;
	}

	//Relocatable object
	m_elfHeader.e_type = ET_REL;
	//Default to x86...just because
	m_elfHeader.e_machine = EM_386;
	//Same as e_ident[EI_VERSION]
	m_elfHeader.e_version = EV_CURRENT;
	//Assume non-executable, would have to be filled in later anyway
	m_elfHeader.e_entry = 0;
	//This is relocatable
	m_elfHeader.e_phoff = 0;
	//This is relocatable
	m_elfHeader.e_shoff = 0;
	//Assume no processor flags
	m_elfHeader.e_flags = 0;
	//No special padding
	m_elfHeader.e_ehsize = sizeof(m_elfHeader);
	//Default size
	m_elfHeader.e_phentsize = sizeof(Elf32_Phdr);
	//This will be dynamically adjusted, start at 0
	m_elfHeader.e_phnum = 0;
	//Default size
	m_elfHeader.e_shentsize = sizeof(Elf32_Shdr);
	//This will be dynamically adjusted, start at 0
	m_elfHeader.e_shnum = 0;
	//Relocatable
	m_elfHeader.e_shstrndx = 0;
	
	return UV_ERR_OK;
}

uv_err_t UVDElf::getUVDElf(UVDElf **elfOut)
{
	UVDElf *elf = NULL;
	
	elf = new UVDElf();
	uv_assert_ret(elf);
	
	//Fill in some basic data into the ELF header
	uv_assert_err_ret(elf->init());
	
	uv_assert_ret(elfOut);
	*elfOut = elf;

	return UV_ERR_OK;
}

uv_err_t UVDElf::getStringTableSectionHeaderEntry(UVDElfStringTableSectionHeaderEntry **sectionOut)
{
	UVDElfStringTableSectionHeaderEntry *section = NULL;
	unsigned int stringTableSectionHeaderIndex = 0;
	
	//Index
	stringTableSectionHeaderIndex = m_elfHeader.e_shstrndx;
	//It should not be 0 (which is defined to be SHT_NULL), a good error check
	uv_assert_ret(stringTableSectionHeaderIndex != 0);
	//Make sure index is valid
	uv_assert_ret(stringTableSectionHeaderIndex < m_sectionHeaderEntries.size());
	
	//Grab it
	section = dynamic_cast<UVDElfStringTableSectionHeaderEntry *>(m_sectionHeaderEntries[stringTableSectionHeaderIndex]);
	uv_assert_ret(section);
	
	//Assign it
	uv_assert_err_ret(sectionOut);
	*sectionOut = section;
	
	return UV_ERR_OK;
}

UVDElfStringTableSectionHeaderEntry::UVDElfStringTableSectionHeaderEntry()
{
}

UVDElfStringTableSectionHeaderEntry::~UVDElfStringTableSectionHeaderEntry()
{
}

void UVDElfStringTableSectionHeaderEntry::addString(const std::string &sIn, unsigned int *index)
{
	for( unsigned int i = 0; i < m_stringTable.size(); ++i )
	{
		const std::string &sCur = m_stringTable[i];
		//Already in there?
		if( sCur == sIn )
		{
			//Its present, return it
			if( index )
			{
				*index = i;
			}
			return;
		}
	}
	//No found, add it
	if( index )
	{
		*index = m_stringTable.size();
	}
	m_stringTable.push_back(sIn);
}

uv_err_t UVDElfStringTableSectionHeaderEntry::ensureCurrentStringTableData()
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
	unsigned int dataSize = 0;
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
	unsigned int offset = 0;
	for( std::vector<std::string>::iterator iter = m_stringTable.begin(); iter != m_stringTable.end(); ++iter )
	{
		std::string s = *iter;
		//Include null space
		unsigned int bufferSize = s.size() + 1;

		uv_assert_err_ret(m_fileData->writeData(offset, s.c_str(), bufferSize));		
		offset += bufferSize;
	}
	
	return UV_ERR_OK;
}

uv_err_t UVDElfStringTableSectionHeaderEntry::getFileData(UVDData **dataOut)
{
	//Make sure our file data is current
	uv_assert_err_ret(ensureCurrentStringTableData());
	//String table file data is required
	uv_assert_ret(m_fileData);

	//Done, assign and return
	uv_assert_ret(dataOut);
	*dataOut = m_fileData;
	return UV_ERR_OK;
}

uv_err_t UVDElf::addString(const std::string &s, unsigned int *index)
{
	UVDElfStringTableSectionHeaderEntry *stringTableEntry = NULL;

	//See if the string exists
	uv_assert_err_ret(getStringTableSectionHeaderEntry(&stringTableEntry));
	uv_assert_ret(stringTableEntry);
	//And call its string managment
	stringTableEntry->addString(s, index);
	
	return UV_ERR_OK;
}

uv_err_t UVDElf::getStringRelocatableElement(const std::string &s, UVDRelocatableElement **relocatableOut, UVDRelocationManager *relocationManager)
{
	UVDSelfLocatingRelocatableElement *relocatable = NULL;
	UVDData *stringTableData = NULL;
	unsigned int stringTableIndex = 0;

	//Make sure this string is registered
	uv_assert_err_ret(addString(s, &stringTableIndex));
	
	UVDElfStringTableSectionHeaderEntry *stringTableEntry = NULL;
	//Find the section header for the string table (specified as part of the elf header)
	uv_assert_err_ret(getStringTableSectionHeaderEntry(&stringTableEntry));
	uv_assert_ret(stringTableEntry);
	//And get the file data associated with it
	uv_assert_err_ret(stringTableEntry->getFileData(&stringTableData));
	uv_assert_ret(stringTableData);
	
	relocatable = new UVDSelfLocatingRelocatableElement(relocationManager, stringTableData, stringTableIndex);
	uv_assert_ret(relocatable);
	uv_assert_ret(relocatableOut);
	*relocatableOut = relocatable;
	
	return UV_ERR_OK;
}

int UVDElf::getProgramHeaderTableSize()
{
	return m_elfHeader.e_phentsize * m_elfHeader.e_phnum;
}

int UVDElf::getSectionHeaderTableSize()
{
	return m_elfHeader.e_shentsize * m_elfHeader.e_shnum;
}

uv_err_t UVDElf::constructBinary(UVDData **dataOut)
{
	uv_assert_ret(dataOut);
	//Do we alreayd have a data representation?
	//Might be cached or previously loaded
	if( m_data )
	{
		*dataOut = m_data;
		return UV_ERR_OK;
	}
		
	UVDRelocationManager elfRelocationManager;
	//Since we construct the section header entries before placing supporting data, these must be stored in a temporary location
	std::vector<UVDRelocatableData *> headerSupportingData;
	
	//Header
	UVDDataMemory *elfHeaderData = NULL;
	UVDRelocatableData *elfHeaderRelocatable = NULL;
	
	//Header
	elfHeaderData = new UVDDataMemory((const char *)&m_elfHeader, sizeof(m_elfHeader));
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
		uv_assert_err_ret(getStringRelocatableElement(entry->m_sName, &nameRelocatable, &elfRelocationManager));
		//sh_name is first element, see elf.h
		unsigned int nameOffset = 0;
		unsigned int nameSize = 4 /*sizeof(Elf32_Shdr.sh_name)*/;
		//Create a fixup to that item at given offset within the header chunk
		nameFixup = new UVDRelocationFixup(nameRelocatable, nameOffset, nameSize);
		uv_assert_ret(nameFixup);
		//Register that offset to the header chunk
		headerRelocatable->addFixup(nameFixup);
		
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

uv_err_t UVDElf::getArchitecture(int *archOut)
{
	uv_assert_ret(archOut);
	//More or less ignore for now
	*archOut = m_elfHeader.e_machine;
	return UV_ERR_OK;
}

void UVDElf::setArchitecture(int archIn)
{
	m_elfHeader.e_machine = archIn;
}

uv_err_t UVDElf::addProgramHeaderSection(UVDElfProgramHeaderEntry *section)
{
	m_programHeaderEntries.push_back(section);	
	return UV_ERR_OK;
}

uv_err_t UVDElf::addSectionHeaderSection(UVDElfSectionHeaderEntry *section)
{
	m_sectionHeaderEntries.push_back(section);
	return UV_ERR_OK;
}

uv_err_t UVDElf::getSectionHeaderByName(const std::string &sName, UVDElfSectionHeaderEntry **section)
{
	uv_assert_ret(section);
	
	for( unsigned int i = 0; i < m_sectionHeaderEntries.size(); ++i )
	{
		UVDElfSectionHeaderEntry *pCurSection = m_sectionHeaderEntries[i];
		std::string sCurName;
		
		uv_assert_ret(pCurSection);
		pCurSection->getName(sCurName);
		//Found it?
		if( sCurName == sName )
		{
			*section = pCurSection;
			return UV_ERR_OK;
		}
	}
	
	//Not a totally abnormal error, don't log
	return UV_ERR_NOTFOUND;
}
