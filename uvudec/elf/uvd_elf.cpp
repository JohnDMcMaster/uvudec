/*
Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#if 0



TODO: figure out why use one over the other
/* Relocation table entry without addend (in section of type SHT_REL).  */
Elf32_Rel
/* Relocation table entry with addend (in section of type SHT_RELA).  */
Elf32_Rela





To make relocatable ELF file
1) Add code in .text

2) Add entries .rel.text:
typedef struct
{
  Elf32_Addr	r_offset;		/* Address */
  Elf32_Word	r_info;			/* Relocation type and symbol index */
} Elf32_Rel;
Also must set sh_info (symbol table section) and sh_link (section to modify relocations on)

3) Add symbol table in .symtab:
typedef struct
{
  Elf32_Word	st_name;		/* Symbol name (string tbl index) */
  Elf32_Addr	st_value;		/* Symbol value */
  Elf32_Word	st_size;		/* Symbol size */
  unsigned char	st_info;		/* Symbol type and binding */
  unsigned char	st_other;		/* Symbol visibility */
  Elf32_Section	st_shndx;		/* Section index */
} Elf32_Sym;



#endif

#include "uvd_elf.h"
#include "uvd_data.h"
#include "uvd_types.h"
#include "uvd_util.h"
#include <elf.h>
#include <vector>
#include <string>

uv_err_t UVDElf::addRelocatableData(UVDRelocatableData *relocatableData,
		const std::string &rawDataSymbolName)
{
	return UV_DEBUG(addRelocatableDataCore(relocatableData, rawDataSymbolName, ".text", ".rel.text"));
}

uv_err_t UVDElf::addRelocatableDataCore(UVDRelocatableData *relocatableData,
		const std::string &rawDataSymbolName,
		const std::string &sDataSection, const std::string &sRelocationSection)
{
	//The chunk of data we apply relocations to
	UVDData *rawData = NULL;

	uv_assert_ret(relocatableData);

	relocatableData->getRelocatableData(&rawData);
	uv_assert_ret(rawData);

	//Convert UVD relocations to ELF relocations
	for( std::set<UVDRelocationFixup *>::const_iterator iter = relocatableData->m_fixups.begin(); 
			iter != relocatableData->m_fixups.end(); ++iter )
	{
		UVDRelocationFixup *fixup = *iter;
		UVDRelocatableElement *originalSymbol = NULL;
		//Fixup into the ELF file
		//UVDSelfLocatingRelocatableElement *relocationElement = NULL;
		UVDElfSymbol *symbol = NULL;
		UVDElfRelocation *elfRelocation = NULL;
		
		uv_assert_ret(fixup);
		originalSymbol = fixup->m_symbol;
		uv_assert_ret(originalSymbol);
		
		uv_assert_err_ret(getSymbol(originalSymbol->getName(), &symbol));
		uv_assert_ret(symbol);
		
		elfRelocation = new UVDElfRelocation();
		uv_assert_ret(elfRelocation);
		//All relocations analyzed for now are just recorded as offset from function start
		elfRelocation->setSymbol(symbol);
		//Convert UVD relocation to ELF relocation
		uv_assert_err_ret(elfRelocation->setupRelocations(fixup));
	}
	
	//Now add on our data itself as a symbol
	UVDElfSymbol *symbol = NULL;
	symbol = new UVDElfSymbol();
	uv_assert_ret(symbol);
	symbol->setSymbolName(rawDataSymbolName);
	//Get a copy of the symbol's data and save it
	uv_assert_err_ret(relocatableData->getRelocatableData(&rawData));
	symbol->setData(rawData);
	uv_assert_err_ret(addSymbol(symbol));
	
	return UV_DEBUG(UV_ERR_OK);
}

uv_err_t UVDElf::addSymbol(UVDElfSymbol *symbol)
{
	return UV_DEBUG(UV_ERR_GENERAL);
}

uv_err_t UVDElf::findSymbol(const std::string &sName, UVDElfSymbol **symbol)
{
	return UV_DEBUG(UV_ERR_GENERAL);
}

uv_err_t UVDElf::getSymbol(const std::string &sName, UVDElfSymbol **symbolOut)
{
	UVDElfSymbol *symbol = NULL;

	//If it already exists, return it
	if( UV_SUCCEEDED(findSymbol(sName, symbolOut)) )
	{
		return UV_ERR_OK;
	}
	
	//Otherwise we must create it
	
	symbol = new UVDElfSymbol();
	uv_assert_ret(symbol);
	
	symbol->setSymbolName(sName);
	uv_assert_err_ret(addSymbol(symbol));

	uv_assert_ret(symbolOut);
	*symbolOut = symbol;
	return UV_ERR_OK;
}

uv_err_t UVDElf::getSectionHeaderStringTableEntry(UVDElfStringTableSectionHeaderEntry **sectionOut)
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

uv_err_t UVDElf::addSectionHeaderString(const std::string &s, unsigned int *index)
{
	return UV_DEBUG(addStringCore(".shstrtab", s, index));
}

uv_err_t UVDElf::addSymbolString(const std::string &s, unsigned int *index)
{
	return UV_DEBUG(addStringCore(".strtab", s, index));
}

uv_err_t UVDElf::addStringCore(const std::string &sSection, const std::string &s, unsigned int *index)
{
	UVDElfSectionHeaderEntry *sectionHeaderEntry = NULL;
	UVDElfStringTableSectionHeaderEntry *stringTableEntry = NULL;

	//Get the table
	uv_assert_err_ret(getSectionHeaderByName(sSection, &sectionHeaderEntry));
	uv_assert_ret(stringTableEntry);
	stringTableEntry = dynamic_cast<UVDElfStringTableSectionHeaderEntry *>(sectionHeaderEntry);
	//And call its string managment
	stringTableEntry->addString(s, index);
	
	return UV_ERR_OK;
}

uv_err_t UVDElf::getSectionHeaderStringRelocatableElement(const std::string &s, UVDRelocatableElement **relocatableOut,
		UVDRelocationManager *relocationManager)
{
	return UV_DEBUG(getStringRelocatableElementCore(".shstrtab", s, relocatableOut, relocationManager));
}

uv_err_t UVDElf::getSymbolStringRelocatableElement(const std::string &s, UVDRelocatableElement **relocatableOut,
		UVDRelocationManager *relocationManager)
{
	return UV_DEBUG(getStringRelocatableElementCore(".strtab", s, relocatableOut, relocationManager));
}

uv_err_t UVDElf::getStringRelocatableElementCore(const std::string &sSection, const std::string &s,
		UVDRelocatableElement **relocatableOut, UVDRelocationManager *relocationManager)
{
	UVDSelfLocatingRelocatableElement *relocatable = NULL;
	UVDData *stringTableData = NULL;
	unsigned int stringTableIndex = 0;
	UVDElfSectionHeaderEntry *sectionHeaderEntry = NULL;
	UVDElfStringTableSectionHeaderEntry *stringTableEntry = NULL;

	//Make sure this string is registered
	uv_assert_err_ret(addStringCore(sSection, s, &stringTableIndex));
	
	//Find the section header for the string table (specified as part of the elf header)
	uv_assert_err_ret(getSectionHeaderByName(sSection, &sectionHeaderEntry));
	uv_assert_ret(stringTableEntry);
	stringTableEntry = dynamic_cast<UVDElfStringTableSectionHeaderEntry *>(sectionHeaderEntry);
	//And get the file data associated with it
	uv_assert_err_ret(stringTableEntry->getFileData(&stringTableData));
	uv_assert_ret(stringTableData);
	
	//FIXME
	//This is the wrong type of relocatable
	//Needs to be specialised to do string table indexing
	//Needs a string table and a string
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
