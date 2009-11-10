/*
Universal Decompiler (uvudec)
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


uv_err_t UVDElf::getFromRelocatableData(UVDRelocatableData *relocatableData,
		const std::string &symbolPrefix, UVDElf **out)
{
	return UV_DEBUG(getFromRelocatableDataCore(relocatableData, symbolPrefix, ".text", ".rel.text", out));
}

uv_err_t UVDElf::getFromRelocatableDataCore(UVDRelocatableData *relocatableData,
		const std::string &symbolPrefix,
		const std::string &sDataSection, const std::string &sRelocationSection,
		UVDElf **out)
{
	/*
	There is a fine difference in relocatable elements here
	The relocatable elment in is a UVD representation of arbitrary data
	Here we must convert this arbitrary form to an ELF structure which may be, for now, repr as a UVD relocatable
	*/
	
	UVDElf *elf = NULL;
	//Data gets placed in section headers
	UVDElfSectionHeaderEntry *sectionHeaderEntry = NULL;

	uv_assert_ret(relocatableData);
	
	//Figure out appropriete section header entry
	uv_assert_err_ret(UVDElfSectionHeaderEntry::getUVDElfSectionHeaderEntry(sDataSection, &sectionHeaderEntry));
	uv_assert_ret(sectionHeaderEntry);
	
	//Get a template ELF object
	uv_assert_err_ret(UVDElf::getUVDElf(&elf));
	uv_assert_ret(elf);
	uv_assert_err_ret(elf->addRelocatableDataCore(relocatableData, symbolPrefix, sDataSection, sRelocationSection));

	return UV_DEBUG(UV_ERR_OK);
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

uv_err_t UVDElf::init()
{
	/*
	Setup default sections
	We could discard these later if needed
	*/
	UVDElfSectionHeaderEntry *nullSection = NULL;
	UVDElfSectionHeaderEntry *sectionStringTableSection = NULL;
	UVDElfSectionHeaderEntry *symbolStringTableSection = NULL;
	UVDElfSectionHeaderEntry *symbolTableSection = NULL;
	UVDElfSectionHeaderEntry *executableSection = NULL;
	//XXX: should add SHT_SYMTAB entry?

	//Fill in some defaults
	uv_assert_err_ret(initHeader());

	//Add the null section
	//According to TIS, only SHT_NULL is defined, other fields are undefined
	nullSection = new UVDElfSectionHeaderEntry();
	uv_assert_ret(nullSection);
	nullSection->setType(SHT_NULL);
	uv_assert_err_ret(addSectionHeaderSection(nullSection));

printDebug();

	//Add section header string table section
	uv_assert_err_ret(UVDElfSectionHeaderEntry::getUVDElfSectionHeaderEntry(UVD_ELF_SECTION_SECTION_STRING_TABLE,
			&sectionStringTableSection));
	uv_assert_err_ret(addSectionHeaderSection(sectionStringTableSection));
	//Register the index (only SHT_NULL was before this)
	//We could alternativly do this as a UVDRelocatable
	m_elfHeader.e_shstrndx = 1;
	//Null string in string table must be first element
	uv_assert_err_ret(addSectionHeaderString(""));

	//Add symbol string table section
	uv_assert_err_ret(UVDElfSectionHeaderEntry::getUVDElfSectionHeaderEntry(UVD_ELF_SECTION_SYMBOL_STRING_TABLE,
			&symbolStringTableSection));
	uv_assert_err_ret(addSectionHeaderSection(symbolStringTableSection));
	//Null string in string table must be first element
	uv_assert_err_ret(addSymbolString(""));	

	//Add symbol table section
	uv_assert_err_ret(UVDElfSectionHeaderEntry::getUVDElfSectionHeaderEntry(UVD_ELF_SECTION_SYMBOL_TABLE,
			&symbolTableSection));
	uv_assert_err_ret(addSectionHeaderSection(symbolTableSection));

	//Add executable data section
	uv_assert_err_ret(UVDElfSectionHeaderEntry::getUVDElfSectionHeaderEntry(UVD_ELF_SECTION_EXECUTABLE,
			&executableSection));
	uv_assert_err_ret(addSectionHeaderSection(executableSection));

printDebug();

	return UV_ERR_OK;
}

void UVDElf::printDebug()
{
	printf("\nELF debug dump\n");
	for( unsigned int i = 0; i < m_sectionHeaderEntries.size(); ++i )
	{
		UVDElfSectionHeaderEntry *pCurSection = m_sectionHeaderEntries[i];
		if( !pCurSection )
		{
			printf("Bad section at index %d\n", i);
			continue;
		}
		
		std::string sCurName;
		pCurSection->getName(sCurName);
		printf("Sections[%d]: <%s>\n", i, sCurName.c_str());
	}
	printf("Dump completed\n");
}
