/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
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

#include "uvdelf/object.h"
#include "uvdelf/relocation.h"
#include "uvd/data/data.h"
#include "uvd/relocation/relocation.h"
#include "uvd/util/types.h"
#include "uvd/util/util.h"
#include <elf.h>
#include <vector>
#include <string>

#if 0
/*
start UVDElfStringTableElement
*/

class UVDElfStringTableElement : public UVDRelocatableElement
{
public:
	UVDElfStringTableElement();
	UVDElfStringTableElement(UVDElf *elf, const std::string &sSection, const std::string &s);
	~UVDElfStringTableElement();

	virtual uv_err_t updateDynamicValue();
	
public:
	//The elf object so we can ID tables and such
	UVDElf *m_elf;
	//The string table we are operating on
	std::string m_sSection;
	//The string we are looking for
	std::string m_s;
};

UVDElfStringTableElement::UVDElfStringTableElement()
{
	m_elf = NULL;
}

UVDElfStringTableElement::UVDElfStringTableElement(UVDElf *elf, const std::string &sSection, const std::string &s)
{
	m_elf = elf;
	m_sSection = sSection;
	m_s = s;
}

UVDElfStringTableElement::~UVDElfStringTableElement()
{
}

uv_err_t UVDElfStringTableElement::updateDynamicValue()
{
	uint32_t stringTableIndex = 0;

	uv_assert_ret(m_elf);

	//Find the string table index (offset)
	uv_assert_err_ret(m_elf->addStringCore(m_sSection, m_s, &stringTableIndex));
	setDynamicValue(stringTableIndex);

	return UV_ERR_OK;
}

/*
end UVDElfStringTableElement
*/
#endif

#if 0
uv_err_t UVDElf::addRelocatableData(UVDRelocatableData *relocatableData,
		const std::string &rawDataSymbolName)
{
	return UV_DEBUG(addRelocatableDataCore(relocatableData, rawDataSymbolName, UVD_ELF_SECTION_EXECUTABLE, ".rel.text"));
}
#endif

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
		uv_assert_ret(fixup);
		uv_assert_err_ret(addRelocation(fixup));
	}
	
	printf_debug("setting up defined sym\n");
	//Now add on our data itself as a (defined) symbol
	UVDElfSymbol *symbol = NULL;
	//Note that if this was previously added (recursive call, etc), it should not be duplicated
	uv_assert_err_ret(getFunctionSymbol(rawDataSymbolName, &symbol));
	uv_assert_ret(symbol);
	printf_debug("the defined symbol: 0x%.8X\n", (unsigned int)symbol);
	//Get a copy of the symbol's data and save it
	uv_assert_err_ret(relocatableData->getRelocatableData(&rawData));
	uv_assert_ret(rawData);
	uv_assert_err_ret(symbol->setData(rawData));
	
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

uv_err_t UVDElf::getVariableSymbol(const std::string &name, UVDElfSymbol **symbolOut)
{
	UVDElfSymbolSectionHeaderEntry *section = NULL;
	
	uv_assert_err_ret(getSymbolTableSectionHeaderEntry(&section));
	uv_assert_ret(section);
	return UV_DEBUG(section->getVariableSymbol(name, symbolOut));	
}

uv_err_t UVDElf::getFunctionSymbol(const std::string &name, UVDElfSymbol **symbolOut)
{
	UVDElfSymbolSectionHeaderEntry *section = NULL;
	
	uv_assert_err_ret(getSymbolTableSectionHeaderEntry(&section));
	uv_assert_ret(section);
	return UV_DEBUG(section->getFunctionSymbol(name, symbolOut));	
}

uv_err_t UVDElf::getSectionHeaderStringTableEntry(UVDElfStringTableSectionHeaderEntry **sectionOut)
{
	UVDElfStringTableSectionHeaderEntry *section = NULL;
	//unsigned int stringTableSectionHeaderIndex = 0;
	UVDElfSectionHeaderEntry *sectionUncasted = NULL;
	
	/*
	//Index
	stringTableSectionHeaderIndex = m_elfHeader.e_shstrndx;
	//It should not be 0 (which is defined to be SHT_NULL), a good error check
	uv_assert_ret(stringTableSectionHeaderIndex != 0);
	//Make sure index is valid
	uv_assert_ret(stringTableSectionHeaderIndex < m_sectionHeaderEntries.size());
	sectionUncasted = m_sectionHeaderEntries[stringTableSectionHeaderIndex];
	*/
	uv_assert_err_ret(getSectionHeaderByName(UVD_ELF_SECTION_SECTION_STRING_TABLE, &sectionUncasted));
	
	//Grab it
	section = dynamic_cast<UVDElfStringTableSectionHeaderEntry *>(sectionUncasted);
	uv_assert_ret(section);
	
	//Assign it
	uv_assert_err_ret(sectionOut);
	*sectionOut = section;
	
	return UV_ERR_OK;
}

uv_err_t UVDElf::addSectionHeaderString(const std::string &s)
{
	return UV_DEBUG(addStringCore(UVD_ELF_SECTION_SECTION_STRING_TABLE, s));
}

uv_err_t UVDElf::addSymbolString(const std::string &s)
{
	return UV_DEBUG(addStringCore(UVD_ELF_SECTION_SYMBOL_STRING_TABLE, s));
}

uv_err_t UVDElf::addStringCore(const std::string &sSection, const std::string &s)
{
	UVDElfSectionHeaderEntry *sectionHeaderEntry = NULL;
	UVDElfStringTableSectionHeaderEntry *stringTableEntry = NULL;

	//Get the table
	uv_assert_err_ret(getSectionHeaderByName(sSection, &sectionHeaderEntry));
	uv_assert_ret(sectionHeaderEntry);
	stringTableEntry = dynamic_cast<UVDElfStringTableSectionHeaderEntry *>(sectionHeaderEntry);
	uv_assert_ret(stringTableEntry);
	//And call its string managment
	stringTableEntry->addString(s);
	
	return UV_ERR_OK;
}

/*
uv_err_t UVDElf::getSectionHeaderStringRelocatableElement(const std::string &s, UVDRelocatableElement **relocatableOut,
		UVDRelocationManager *relocationManager)
{
	return UV_DEBUG(getStringRelocatableElementCore(UVD_ELF_SECTION_SECTION_STRING_TABLE, s, relocatableOut, relocationManager));
}

uv_err_t UVDElf::getSymbolStringRelocatableElement(const std::string &s, UVDRelocatableElement **relocatableOut)
{
	return UV_DEBUG(getStringRelocatableElementCore(UVD_ELF_SECTION_SYMBOL_STRING_TABLE, s, relocatableOut, NULL));
}

uv_err_t UVDElf::getStringRelocatableElementCore(const std::string &sSection, const std::string &s,
		UVDRelocatableElement **relocatableOut, UVDRelocationManager *relocationManager)
{
	UVDRelocatableElement *relocatable = NULL;
	UVDData *stringTableData = NULL;
	unsigned int stringTableIndex = 0;
	UVDElfSectionHeaderEntry *sectionHeaderEntry = NULL;
	UVDElfStringTableSectionHeaderEntry *stringTableEntry = NULL;

	//Make sure this string is registered
	//what if a string is removed?  Don't think it will be done soon, but "what if"
	//It will get added at the end anyway, may not be necessary
	uv_assert_err_ret(addStringCore(sSection, s, &stringTableIndex));
	//printf_debug("Index of string <%s> in table %s: %d\n", s.c_str(), sSection.c_str(), stringTableIndex);

	//Find the section header for the string table (specified as part of the elf header)
	uv_assert_err_ret(getSectionHeaderByName(sSection, &sectionHeaderEntry));
	uv_assert_ret(sectionHeaderEntry);
	stringTableEntry = dynamic_cast<UVDElfStringTableSectionHeaderEntry *>(sectionHeaderEntry);
	//And get the file data associated with it
	uv_assert_err_ret(stringTableEntry->getFileData(&stringTableData));
	uv_assert_ret(stringTableData);
	
	//FIXME
	//This is the wrong type of relocatable
	//Needs to be specialised to do string table indexing
	//Needs a string table and a string
	//relocatable = new UVDSelfLocatingRelocatableElement(relocationManager, stringTableData, stringTableIndex);
	relocatable = new UVDElfStringTableElement(this, sSection, s);
	uv_assert_ret(relocatable);
	uv_assert_ret(relocatableOut);
	*relocatableOut = relocatable;
	
	return UV_ERR_OK;
}
*/

uv_err_t UVDElf::getSectionHeaderStringIndex(const std::string &s, uint32_t *index)
{
	return UV_DEBUG(getStringIndexCore(UVD_ELF_SECTION_SECTION_STRING_TABLE, s, index));
}

uv_err_t UVDElf::getSymbolStringIndex(const std::string &s, uint32_t *index)
{
	return UV_DEBUG(getStringIndexCore(UVD_ELF_SECTION_SYMBOL_STRING_TABLE, s, index));
}

uv_err_t UVDElf::getStringIndexCore(const std::string &sSection, const std::string &s, uint32_t *index)
{
	UVDElfSectionHeaderEntry *sectionHeaderEntry = NULL;
	UVDElfStringTableSectionHeaderEntry *stringTableEntry = NULL;

	//Find the section header for the string table (specified as part of the elf header)
	uv_assert_err_ret(getSectionHeaderByName(sSection, &sectionHeaderEntry));
	uv_assert_ret(sectionHeaderEntry);
	stringTableEntry = dynamic_cast<UVDElfStringTableSectionHeaderEntry *>(sectionHeaderEntry);
	uv_assert_ret(stringTableEntry);
		
	uv_assert_err_ret(stringTableEntry->getStringOffset(s, index));
		
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

int UVDElf::getNumberProgramHeaderTableEntries()
{
	return m_programHeaderEntries.size();
}

int UVDElf::getNumberSectionHeaderTableEntries()
{
	return m_sectionHeaderEntries.size();
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
	uv_assert_ret(section);
	section->m_elf = this;
	m_sectionHeaderEntries.push_back(section);
	return UV_ERR_OK;
}

uv_err_t UVDElf::getSectionHeaderIndex(const UVDElfSectionHeaderEntry *section, uint32_t *index)
{
	for( std::vector<UVDElfSectionHeaderEntry *>::size_type i = 0; i < m_sectionHeaderEntries.size(); ++i )
	{
		UVDElfSectionHeaderEntry *curSection = m_sectionHeaderEntries[i];

		uv_assert_ret(curSection);
		if( curSection == section )
		{
			uv_assert_ret(index);
			*index = i;
			return UV_ERR_OK;
		}
	}
	return UV_DEBUG(UV_ERR_GENERAL);
}

uv_err_t UVDElf::getSectionHeaderIndexByName(const std::string &name, uint32_t *index)
{
	UVDElfSectionHeaderEntry *section = NULL;
	
	uv_assert_err_ret(getSectionHeaderByName(name, &section));
	uv_assert_err_ret(getSectionHeaderIndex(section, index));
	
	return UV_ERR_OK;
}

uv_err_t UVDElf::getSectionHeaderByName(const std::string &name, UVDElfSectionHeaderEntry **sectionOut)
{
	uv_assert_ret(sectionOut);
	
	for( unsigned int i = 0; i < m_sectionHeaderEntries.size(); ++i )
	{
		UVDElfSectionHeaderEntry *pCurSection = m_sectionHeaderEntries[i];
		std::string sCurName;

		uv_assert_ret(pCurSection);
		pCurSection->getName(sCurName);
		//printf_debug("Section: <%s> vs needed <%s>\n", sCurName.c_str(), name.c_str());
		//Found it?
		if( sCurName == name )
		{
			*sectionOut = pCurSection;
			return UV_ERR_OK;
		}
	}

	//printf_warn("Couldn't get section %s\n", name.c_str());	
	//Not a totally abnormal error, don't log
	return UV_ERR_NOTFOUND;
}

uv_err_t UVDElf::getUVDElfSectionHeaderEntry(const std::string &sSection, UVDElfSectionHeaderEntry **sectionHeaderOut)
{
	uv_assert_err_ret(UVDElfSectionHeaderEntry::getUVDElfSectionHeaderEntryCore(sSection, sectionHeaderOut));
	uv_assert_ret(sectionHeaderOut);
	uv_assert_ret(*sectionHeaderOut);
	(*sectionHeaderOut)->m_elf = this;
	
	return UV_ERR_OK;
}

uv_err_t UVDElf::addUVDElfSectionHeaderEntry(const std::string &sSection, UVDElfSectionHeaderEntry **sectionHeaderOut)
{
	UVDElfSectionHeaderEntry *sectionHeader = NULL;
	
	uv_assert_err_ret(getUVDElfSectionHeaderEntry(sSection, &sectionHeader));
	uv_assert_ret(sectionHeader);

	uv_assert_err_ret(addSectionHeaderSection(sectionHeader));
	
	if( sectionHeaderOut )
	{
		*sectionHeaderOut = sectionHeader;
	}
	
	return UV_ERR_OK;
}

uv_err_t UVDElf::getTextSectionHeaderEntry(UVDElfTextSectionHeaderEntry **sectionOut)
{
	UVDElfSectionHeaderEntry *sectionRaw = NULL;
	UVDElfTextSectionHeaderEntry *section = NULL;
	
	uv_assert_err_ret(getSectionHeaderByName(UVD_ELF_SECTION_EXECUTABLE, &sectionRaw));
	uv_assert_ret(sectionRaw);
	section = dynamic_cast<UVDElfTextSectionHeaderEntry *>(sectionRaw);
	uv_assert_ret(section);

	uv_assert_ret(sectionOut);
	*sectionOut = section;
	return UV_ERR_OK;
}

uv_err_t UVDElf::canLoad(const UVDData *data, const UVDRuntimeHints &hints, uvd_priority_t *confidence)
{
	Elf32_Ehdr elfHeader;

	if( UV_SUCCEEDED(data->readData(0, (char *)&elfHeader, sizeof(Elf32_Ehdr)))
			&& elfHeader.e_ident[EI_MAG0] == ELFMAG0
			&& elfHeader.e_ident[EI_MAG1] == ELFMAG1
			&& elfHeader.e_ident[EI_MAG2] == ELFMAG2
			&& elfHeader.e_ident[EI_MAG3] == ELFMAG3 )
	{
		*confidence = UVD_MATCH_ACCEPTABLE;
	}
	else
	{
		*confidence = UVD_MATCH_NONE;
	}
	return UV_ERR_OK;
}

uv_err_t UVDElf::tryLoad(UVDData *data, const UVDRuntimeHints &hints, UVDObject **out)
{
	UVDElf *object = NULL;
	
	object = new UVDElf();
	uv_assert_ret(object);
	uv_assert_err_ret(object->init(data));
	
	uv_assert_ret(out);
	*out = object;
	return UV_ERR_OK;
}

#if 0
uv_err_t UVDElf::addUVDRelocationFixup(UVDRelocationFixup *fixup)
{
	UVDRelocatableElement *originalSymbol = NULL;
	//Fixup into the ELF file
	//UVDSelfLocatingRelocatableElement *relocationElement = NULL;
	UVDElfSymbol *symbol = NULL;
	UVDElfRelocation *elfRelocation = NULL;
	
	uv_assert_ret(fixup);
	originalSymbol = fixup->m_symbol;
	uv_assert_ret(originalSymbol);
	
	std::string symbolName;
	uv_assert_err_ret(originalSymbol->getName(symbolName));
	uv_assert_ret(!symbolName.empty());
	uv_assert_err_ret(getFunctionSymbol(symbolName, &symbol));
	uv_assert_ret(symbol);
	
	elfRelocation = new UVDElfSymbolRelocation();
	uv_assert_ret(elfRelocation);
	//All relocations analyzed for now are just recorded as offset from function start
	elfRelocation->setSymbol(symbol);

	//Convert UVD relocation to ELF relocation
	{
		uint32_t offset = 0;
		
		//uv_assert_err_ret(elfRelocation->setupRelocations(fixup));
		uv_assert_err_ret(fixup->getOffset(&offset));
		uv_assert_err_ret(elfRelocation->setOffset(offset));
	
		uv_assert_err_ret(elfRelocation->setSizeBits(fixup->getSizeBits()));
	}

	elfRelocation->m_offset = fixup->m_offset;

	//And register it
	uv_assert_err_ret(symbol->addRelocation(elfRelocation));

	return UV_ERR_OK;
}
#endif

uv_err_t UVDElf::addRelocation(UVDRelocationFixup *analysisRelocation)
{
	//analysisRelocation param forms analysis place
	//Offsets are assumed for now to be for the single symbol we have in the file that these apply to
	//Analysis fixup value
	UVDRelocatableElement *analysisSymbol = NULL;
	//ELF fixup value
	UVDElfSymbol *elfSymbol = NULL;
	//ELF place to apply fixup
	UVDElfRelocation *elfRelocation = NULL;

	std::string symbolName;


	//Setup related
	
	uv_assert_ret(analysisRelocation);
	
	//Analysis symbol
	analysisSymbol = analysisRelocation->m_symbol;
	uv_assert_ret(analysisSymbol);

	//Symbol name
	uv_assert_err_ret(analysisSymbol->getName(symbolName));
	uv_assert_ret(!symbolName.empty());
	
	//ELF symbol
	uv_assert_err_ret(getFunctionSymbol(symbolName, &elfSymbol));
	uv_assert_ret(elfSymbol);

	//ELF relocation
	uv_assert_err_ret(elfSymbol->getRelocation(&elfRelocation));
	uv_assert_ret(elfRelocation);

	//Transfer the relocation
	//Assume are relative to the symbol (assumed function) for now
	
	//Size
	uv_assert_err_ret(elfRelocation->updateRelocationTypeByBits(analysisRelocation->getSizeBits()));
	//Where
	//elfRelocation->setSectionOffset(analysisRelocation->m_offset);
	elfRelocation->m_offset = analysisRelocation->m_offset;
	//Note value is the external symbol value

	//And register it
	uv_assert_err_ret(elfSymbol->addRelocation(elfRelocation));
	
	return UV_ERR_OK;
}

#include "uvd/assembly/function.h"

uv_err_t UVDElf::addFunction(UVDBinaryFunction *function)
{
	UVDElf *elf = NULL;
	std::string symbolName;
	
	uv_assert_ret(function->m_relocatableData);
	//uv_assert_ret(m_relocatableData->m_data);
	//Get a base representation
	//printf("symbol: %s, symbol relocations: %d\n", name.c_str(), m_relocatableData->m_fixups.size()); 
	uv_assert_err_ret(function->getSymbolName(symbolName));
	uv_assert_ret(!symbolName.empty());
	//uv_assert_err_ret(UVDElf::getFromRelocatableData(function->m_relocatableData, symbolName, &elf));


	uv_assert_err_ret(addRelocatableDataCore(function->m_relocatableData, symbolName, UVD_ELF_SECTION_EXECUTABLE, ".rel.text"));



	/*
	Add in the relocations
	For each relocation, register the symbol if it doesn't exist
	Then, add it as an instance of relocation for that symbol
	*/
	uv_assert_ret(function->m_relocatableData);
	for( std::set<UVDRelocationFixup *>::iterator iter = function->m_relocatableData->m_fixups.begin();
			iter != function->m_relocatableData->m_fixups.end(); ++iter )
	{
		UVDRelocationFixup *fixup = *iter;
		
		uv_assert_ret(fixup);
		uv_assert_ret(fixup->m_symbol);
		/*
		{
			std::string name;
			uv_assert_err_ret(fixup->m_symbol->getName(name));
			printf("Fixup name: %s\n", name.c_str());
			uv_assert_ret(!name.empty());
		}
		*/
		uv_assert_err_ret(addRelocation(fixup));
	}
	
	{
		std::string sourceFilename;
		std::string mangledSourceFilename;

		sourceFilename = m_data->getSource();
		uv_assert_ret(!sourceFilename.empty());
		//TODO: add option here for absolute path save
		mangledSourceFilename = uv_basename(sourceFilename);
		uv_assert_ret(!mangledSourceFilename.empty());
		uv_assert_err_ret(elf->setSourceFilename(mangledSourceFilename));
	}
	
	return UV_ERR_OK;
}

