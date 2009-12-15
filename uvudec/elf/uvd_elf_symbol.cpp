/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#include "uvd_elf.h"
#include "uvd_util.h"
#include <string>

/*
UVDElfSymbol
*/

UVDElfSymbol::UVDElfSymbol()
{
	m_sectionHeader = NULL;
	memset(&m_symbol, 0, sizeof(Elf32_Sym));
}

UVDElfSymbol::~UVDElfSymbol()
{
}

/*
void UVDElfSymbol::setSymbolName(const std::string &sName)
{
	m_sName = sName;
}

std::string UVDElfSymbol::getSymbolName()
{
	return m_sName;
}
*/

uv_err_t UVDElfSymbol::getData(UVDData **data)
{
	//uv_assert_ret(m_relocatableData);
	//We usually want to return the relocated version
	uv_assert_err_ret(m_relocatableData.getDefaultRelocatableData(data));
	return UV_ERR_OK;
}

uv_err_t UVDElfSymbol::setData(UVDData *data)
{
	/*
	{
		std::string sName;
		getName(sName);
		printf("Setting symbol(0x%.8X, %s) data to 0x%.8X\n", (unsigned int)this, sName.c_str(), (unsigned int)data);
	}
	*/
	uv_assert_err_ret(m_relocatableData.setData(data));
	return UV_ERR_OK;
}

/*
UVDElfSymbolSectionHeaderEntry
*/

UVDElfSymbolSectionHeaderEntry::UVDElfSymbolSectionHeaderEntry()
{
}

UVDElfSymbolSectionHeaderEntry::~UVDElfSymbolSectionHeaderEntry()
{
}

uv_err_t UVDElfSymbolSectionHeaderEntry::initRelocatableData()
{
	//So that we can construct the symbol table in relocatable units
	m_fileRelocatableData = new UVDMultiRelocatableData();
	uv_assert_ret(m_fileRelocatableData);
printf("relocatable section file relocatable: 0x%.8X)\n", (unsigned int)m_fileRelocatableData);
	return UV_ERR_OK;
}

uv_err_t UVDElfSymbolSectionHeaderEntry::addSymbol(UVDElfSymbol *symbol)
{
	uv_assert_ret(symbol);
	//Don't do this, there are external symbols without data
	//uv_assert_ret(symbol->m_relocatableData.m_data);
	m_symbols.push_back(symbol);
	return UV_ERR_OK;
}

uv_err_t UVDElfSymbolSectionHeaderEntry::findSymbol(const std::string &sName, UVDElfSymbol **symbolOut)
{
	//Loop until we find a matching name
	for( std::vector<UVDElfSymbol *>::iterator iter = m_symbols.begin();
			iter != m_symbols.end(); ++iter )
	{
		UVDElfSymbol *symbol = *iter;
		std::string sSymbolName;
		
		uv_assert_ret(symbol);
		
		uv_assert_err_ret(symbol->getName(sSymbolName));
		if( sSymbolName == sName )
		{
			uv_assert_ret(symbolOut);
			*symbolOut = symbol;
			return UV_ERR_OK;
		}
	}
	return UV_ERR_NOTFOUND;
}

uv_err_t UVDElfSymbolSectionHeaderEntry::getSymbol(const std::string &sName, UVDElfSymbol **symbolOut)
{
	//Return if it already exists
	if( UV_SUCCEEDED(findSymbol(sName, symbolOut)) )
	{
		return UV_ERR_OK;
	}
	
	//Otherwise create it
	UVDElfSymbol *symbol = NULL;
	
	symbol = new UVDElfSymbol();
	uv_assert_ret(symbol);
	//So it can get string table indexes and such
	symbol->m_sectionHeader = this;
	symbol->setName(sName);
	//Add it to our table list
	uv_assert_err_ret(addSymbol(symbol));
	
	uv_assert_ret(symbolOut);
	*symbolOut = symbol;
	return UV_ERR_OK;
}

uv_err_t UVDElfSymbolSectionHeaderEntry::getSymbolIndex(const UVDElfSymbol *symbool, uint32_t *index)
{
	for( std::vector<UVDElfSymbol *>::size_type i = 0; i < m_symbols.size(); ++i )
	{
		//The memory address are expected to be equivilent since should be using getSymbol() for uniq symbol objects (per ELF object)
		if( symbool == m_symbols[i] )
		{
			uv_assert_ret(index);
			*index = i;
			return UV_ERR_OK;
		}
	}
	//Not found
	return UV_DEBUG(UV_ERR_GENERAL);
}

uv_err_t UVDElfSymbolSectionHeaderEntry::updateDataCore()
{
	/*
	.symtab
	This is the symbol names along with basic attributes
	
	.rel.text
	relocations for the text segment
	
	typedef struct {
	Elf32_Word st_name;
	Elf32_Addr st_value;
	Elf32_Word st_size;
	unsigned char st_info;
	unsigned char st_other;
	Elf32_Half st_shndx;
	} Elf32_Sym;
	*/
	//Bad symbol table?
	//uv_assert_ret(m_fileData); 
	UVDMultiRelocatableData *relocatableData = NULL;
	
	relocatableData = dynamic_cast<UVDMultiRelocatableData *>(m_fileRelocatableData);
	uv_assert_ret(relocatableData);
	
	/*
	this approach was abandoned in place of a more flexible approach
	
	UVDDataMemory *dataMemory = NULL;
	//Figure out our m_fileData size
	//Total size of the table = elements * element size
	uint32_t tableSize = 0;

	tableSize = sizeof(Elf32_Sym) * tableSize;
	if( m_fileData )
	{
		dataMemory = dynamic_cast<UVDDataMemory *>(m_fileData);
	}
	else
	{
		dataMemory = new UVDDataMemory();
	}
	uv_assert_ret(dataMemory);
	uv_assert_err_ret(dataMemory->realloc(tableSize));
	*/
	
	//We are rebuilding this table
	relocatableData->m_relocatableDatas.clear();

	//Don't use an iterator (that might put in undefined order)
	//It must be well ordered since Elf32_Rel.r_info specifies a symbol table index
	for( std::vector<UVDElfSymbol *>::size_type i = 0; i < m_symbols.size(); ++i )
	{
		UVDElfSymbol *symbol = m_symbols[i];
		UVDRelocatableData *symbolEntryRelocatable = NULL;
		
		uv_assert_ret(symbol);
		uv_assert_err_ret(symbol->getHeaderEntryRelocatable(&symbolEntryRelocatable));
		uv_assert_ret(symbolEntryRelocatable);
		relocatableData->m_relocatableDatas.push_back(symbolEntryRelocatable);
	}
	
	return UV_ERR_OK;
}

uv_err_t UVDElfSymbolSectionHeaderEntry::getSymbolStringRelocatableElement(const std::string &s, UVDRelocatableElement **relocatable)
{
	uv_assert_ret(m_elf);
	uv_assert_err_ret(m_elf->getSymbolStringRelocatableElement(s, relocatable));
	return UV_ERR_OK;
}

uv_err_t UVDElf::getSymbolStringTableSectionHeaderEntry(UVDElfStringTableSectionHeaderEntry **sectionOut)
{
	UVDElfSectionHeaderEntry *sectionRaw = NULL;
	UVDElfStringTableSectionHeaderEntry *section = NULL;
	
	uv_assert_err_ret(getSectionHeaderByName(UVD_ELF_SECTION_SYMBOL_STRING_TABLE, &sectionRaw));
	uv_assert_ret(sectionRaw);
	section = dynamic_cast<UVDElfStringTableSectionHeaderEntry *>(sectionRaw);
	uv_assert_ret(section);

	uv_assert_ret(sectionOut);
	*sectionOut = section;
	return UV_ERR_OK;
}

uv_err_t UVDElf::getSymbolTableSectionHeaderEntry(UVDElfSymbolSectionHeaderEntry **sectionOut)
{
	UVDElfSectionHeaderEntry *sectionRaw = NULL;
	UVDElfSymbolSectionHeaderEntry *section = NULL;
	
	uv_assert_err_ret(getSectionHeaderByName(UVD_ELF_SECTION_SYMBOL_TABLE, &sectionRaw));
	uv_assert_ret(sectionRaw);
	section = dynamic_cast<UVDElfSymbolSectionHeaderEntry *>(sectionRaw);
	uv_assert_ret(section);

	uv_assert_ret(sectionOut);
	*sectionOut = section;
	return UV_ERR_OK;
}

uv_err_t UVDElfSymbol::getRelocation(UVDElfRelocation **relocationOut)
{
	UVDElfRelocation *relocation = NULL;
	
	relocation = new UVDElfRelocation();
	uv_assert_ret(relocation);
	
	relocation->setSymbol(this);
	
	uv_assert_ret(relocationOut);
	*relocationOut = relocation;
	
	return UV_ERR_OK;
}

uv_err_t UVDElfSymbol::getHeaderEntryRelocatable(UVDRelocatableData **symbolEntryRelocatableOut)
{
	//FIXME: make sure we only do this once
	/*
	typedef struct
	{
	  Elf32_Word	st_name;		// Symbol name (string tbl index)
	  Elf32_Addr	st_value;		// Symbol value
	  Elf32_Word	st_size;		// Symbol size
	  unsigned char	st_info;		// Symbol type and binding
	  unsigned char	st_other;		// Symbol visibility
	  Elf32_Section	st_shndx;		// Section index
	} Elf32_Sym;

	Assume symbols are not defined
	Thus, we don't know the value of:
		st_value (applied during final linking)
	*/
	UVDDataMemory *headerDataMemory = NULL;
	
	uv_assert_err_ret(UVDDataMemory::getUVDDataMemoryByTransfer(&headerDataMemory,
			(char *)&m_symbol, sizeof(Elf32_Sym), false));
	uv_assert_ret(headerDataMemory);
	uv_assert_err_ret(m_headerEntryRelocatableData.setData(headerDataMemory));

	//Add relocations
	
	//Symbol name
	{
		std::string name;
		UVDRelocatableElement *relocatable = NULL;
		UVDRelocationFixup *offsetFixup = NULL;

		uv_assert_err_ret(getName(name));
		//The value
		uv_assert_ret(m_sectionHeader);
		uv_assert_err_ret(m_sectionHeader->getSymbolStringRelocatableElement(name, &relocatable));
		//Where
		offsetFixup = new UVDRelocationFixup(relocatable,
				OFFSET_OF(Elf32_Sym, st_name), sizeof(m_symbol.st_name));
		uv_assert_ret(offsetFixup);
		
		m_headerEntryRelocatableData.addFixup(offsetFixup);
	}
	
	/*
	st_value, (st_size?) is determined during linking
	Since our program was already linked, we could bind st_value to the val it took during linking
	Maybe add a switch for this?
	*/
	
	//FIXME
	//There will be some local non-function symbols later (jumps), but assume all global funcs for now
	m_symbol.st_info = ELF32_ST_INFO(STB_GLOBAL, STT_FUNC);
	
	//st_other is undefined according to TIS spec
	
	//The section we associating this symbol with (probably .text?  Or is .rel.text?)
	//Absolute for now, not sure if this is right
	m_symbol.st_shndx = SHN_ABS;
	
	uv_assert_ret(symbolEntryRelocatableOut);
	*symbolEntryRelocatableOut = &m_headerEntryRelocatableData;
	
	return UV_ERR_OK;
}
