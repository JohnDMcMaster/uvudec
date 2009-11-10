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

	uv_assert_err_ret(relocatableData->getRelocatableData(&rawData));
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
	UVDElfSymbolSectionHeaderEntry *section = NULL;
	
	uv_assert_err_ret(getSymbolTableSectionHeaderEntry(&section));
	uv_assert_ret(section);
	return UV_DEBUG(section->addSymbol(symbol));	
}

uv_err_t UVDElf::findSymbol(const std::string &sName, UVDElfSymbol **symbol)
{
	UVDElfSymbolSectionHeaderEntry *section = NULL;
	
	uv_assert_err_ret(getSymbolTableSectionHeaderEntry(&section));
	uv_assert_ret(section);
	return UV_DEBUG(section->findSymbol(sName, symbol));	
}

uv_err_t UVDElf::getSymbol(const std::string &sName, UVDElfSymbol **symbolOut)
{
	UVDElfSymbolSectionHeaderEntry *section = NULL;
	
	uv_assert_err_ret(getSymbolTableSectionHeaderEntry(&section));
	uv_assert_ret(section);
	return UV_DEBUG(section->getSymbol(sName, symbolOut));	
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

uv_err_t UVDElf::addSectionHeaderString(const std::string &s, unsigned int *index)
{
	return UV_DEBUG(addStringCore(UVD_ELF_SECTION_SECTION_STRING_TABLE, s, index));
}

uv_err_t UVDElf::addSymbolString(const std::string &s, unsigned int *index)
{
	return UV_DEBUG(addStringCore(UVD_ELF_SECTION_SYMBOL_STRING_TABLE, s, index));
}

uv_err_t UVDElf::addStringCore(const std::string &sSection, const std::string &s, unsigned int *index)
{
	UVDElfSectionHeaderEntry *sectionHeaderEntry = NULL;
	UVDElfStringTableSectionHeaderEntry *stringTableEntry = NULL;

	//Get the table
	uv_assert_err_ret(getSectionHeaderByName(sSection, &sectionHeaderEntry));
	uv_assert_ret(sectionHeaderEntry);
	stringTableEntry = dynamic_cast<UVDElfStringTableSectionHeaderEntry *>(sectionHeaderEntry);
	uv_assert_ret(stringTableEntry);
	//And call its string managment
	stringTableEntry->addString(s, index);
	
	return UV_ERR_OK;
}

uv_err_t UVDElf::getSectionHeaderStringRelocatableElement(const std::string &s, UVDRelocatableElement **relocatableOut,
		UVDRelocationManager *relocationManager)
{
	return UV_DEBUG(getStringRelocatableElementCore(UVD_ELF_SECTION_SECTION_STRING_TABLE, s, relocatableOut, relocationManager));
}

uv_err_t UVDElf::getSymbolStringRelocatableElement(const std::string &s, UVDRelocatableElement **relocatableOut,
		UVDRelocationManager *relocationManager)
{
	return UV_DEBUG(getStringRelocatableElementCore(UVD_ELF_SECTION_SYMBOL_STRING_TABLE, s, relocatableOut, relocationManager));
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
printf("Section: <%s> vs needed <%s>\n", sCurName.c_str(), sName.c_str());
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
