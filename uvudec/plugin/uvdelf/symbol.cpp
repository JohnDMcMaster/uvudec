/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

/*
Looking at symbols in a simple hello file:
hello_c.o:     file format elf32-i386

SYMBOL TABLE:
00000000 l    df *ABS*  00000000 hello_c.c
00000000 l    d  .text  00000000 .text
00000000 l    d  .data  00000000 .data
00000000 l    d  .bss   00000000 .bss
00000000 l    d  .rodata        00000000 .rodata
00000000 l    d  .note.GNU-stack        00000000 .note.GNU-stack
00000000 l    d  .comment       00000000 .comment
00000000 g     F .text  0000002b main
00000000         *UND*  00000000 puts

These seem somewhat useless
What does binutils actually require?
.text probably, maybe .data, .bss, .rodata
00000000 l    d  .text  00000000 .text
00000000 l    d  .data  00000000 .data
00000000 l    d  .bss   00000000 .bss
00000000 l    d  .rodata        00000000 .rodata
0000000 l    d  .note.GNU-stack        00000000 .note.GNU-stack
000000000 l    d  .comment       00000000 .comment
0Apparantly Sun tools like to add .comment and thats why thats there


Symbol notes, taken from uvelf's main.o


Specify the file name
symbol @ index 0x00000010
	st_name (symbol name): main.c (0x00000001)
	st_value: 0x00000000
	st_size: 0x00000000
	st_info: 0x04
	    bind: STB_LOCAL (0x0)
	    type: STT_FILE (name is file name) (0x4)
	st_other: 0x00
	st_shndx: 0x0000FFF1 (SHN_ABS (absolute symbol))

A sample section link entry...I'm not entirely clear the purpose of these
since each individual symbol defines its relavent section
symbol @ index 0x00000020
	st_name (symbol name): <NULL> (0x00000000)
	st_value: 0x00000000
	st_size: 0x00000000
	st_info: 0x03
	    bind: STB_LOCAL (0x0)
	    type: STT_SECTION associated with a section) (0x3)
	st_other: 0x00
	st_shndx: 0x00000001 (.text)

An undefined symbol
symbol @ index 0x00000190
	st_name (symbol name): printf (0x0000003A)
	st_value: 0x00000000
	st_size: 0x00000000
	st_info: 0x10
	    bind: STB_GLOBAL (0x1)
	    type: STT_NOTYPE (type is unspecified) (0x0)
	st_other: 0x00
	st_shndx: 0x00000000 (SHN_UNDEF (undefined section))

A defined function symbol
symbol @ index 0x000001A0
	st_name (symbol name): main (0x00000041)
	st_value: 0x00000070
	st_size: 0x00000395
	st_info: 0x12
	    bind: STB_GLOBAL (0x1)
	    type: STT_FUNC (a code object) (0x2)
	st_other: 0x70
	st_shndx: 0x00000001 (.text)


[mcmaster@gespenst uv_elf]$ objdump --syms main.o 

main.o:     file format elf32-i386

SYMBOL TABLE:
00000000 l    df *ABS*  00000000 main.c
00000000 l    d  .text  00000000 .text
00000000 l    d  .data  00000000 .data
00000000 l    d  .bss   00000000 .bss
00000000 l    d  .debug_abbrev  00000000 .debug_abbrev
...
00000000         *UND*  00000000 printf
00000070 g     F .text  00000395 main
...




A defined global var
symbol @ index 0x000001C0
	st_name (symbol name): g_string_table_indexes (0x000000B6)
	st_value: 0x00000004
	st_size: 0x00000010
	st_info: 0x11
	    bind: STB_GLOBAL (0x1)
	    type: STT_OBJECT (a data object) (0x1)
	st_other: 0x04
	st_shndx: 0x0000FFF2 (SHN_COMMON (common symbol))

00000010       O *COM*  00000004 g_string_table_indexes
*/

#include "uvdelf/object.h"
#include "uvdelf/relocation.h"
#include "uvd/relocation/relocation.h"
#include "uvd/util/util.h"
#include <string>
#include <stdio.h>
#include <string.h>

#if 1
#define printf_elf_symbol_debug(...)
#define ELF_SYMBOL_DEBUG(x)
#else
#define printf_elf_symbol_debug(format, ...)		do{ printf("ELF symbol: " format, ## __VA_ARGS__); fflush(stdout); } while(0)
#define ELF_SYMBOL_DEBUG(x)		x
#endif

/*
UVDElf related funcs
*/

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

/*
uv_err_t UVDElf::getRelocationSectionHeaderEntry(UVDElfRelocationSectionHeaderEntry **sectionOut);	
{
	UVDElfSectionHeaderEntry *sectionRaw = NULL;
	UVDElfRelocationSectionHeaderEntry *section = NULL;
	
	uv_assert_err_ret(getSectionHeaderByName(UVD_ELF_SECTION_SYMBOL_TABLE, &sectionRaw));
	uv_assert_ret(sectionRaw);
	section = dynamic_cast<UVDElfSymbolSectionHeaderEntry *>(sectionRaw);
	uv_assert_ret(section);

	uv_assert_ret(sectionOut);
	*sectionOut = section;
	return UV_ERR_OK;
}
*/

/*
UVDElfSymbol
*/

UVDElfSymbol::UVDElfSymbol()
{
	m_symbolSectionHeader = NULL;
	m_relevantSectionHeader = NULL;
	memset(&m_symbol, 0, sizeof(Elf32_Sym));
}

UVDElfSymbol::~UVDElfSymbol()
{
}

/*
void UVDElfSymbol::setSymbolName(const std::string &name)
{
	m_name = name;
}

std::string UVDElfSymbol::getSymbolName()
{
	return m_name;
}
*/

uv_err_t UVDElfSymbol::updateForWrite()
{
	UVDElf *elf = NULL;
	uv_assert_ret(m_symbolSectionHeader);
	elf = m_symbolSectionHeader->m_elf;
	uv_assert_ret(elf);

	printf_elf_symbol_debug("base symbol write update\n");

	//Make sure name is in string table
	uv_assert_err_ret(elf->addSymbolString(m_sName));
	
	return UV_ERR_OK;
}

/*
uv_err_t UVDElfSymbol::constructForWrite()
{
	return UV_ERR_OK;
}
*/

uv_err_t UVDElfSymbol::applyRelocationsForWrite()
{
	//Types tend to change depending on whether or not data was set
	uv_assert_err_ret(updateType());

	/*
	Update st_shndx if we linked to a section, or SHN_UNDEF otherwise
	*/
	if( m_relevantSectionHeader )
	{
		uint32_t index = 0;

		//Specifies the section to link to
		uv_assert_err_ret(m_symbolSectionHeader->m_elf->getSectionHeaderIndex(m_relevantSectionHeader, &index));
		m_symbol.st_shndx = index;
	}

	{
		UVDData *data = NULL;

		uv_assert_err_ret(getData(&data));
		//Defined
		if( data )
		{
			//FIXME: assume single symbol per object file for now
			uint32_t offset = 0;
			uint32_t size = 0;
			
			uv_assert_err_ret(m_symbolSectionHeader->getSymbolSectionOffset(this, &offset));
			uv_assert_err_ret(data->size(&size));
			
			m_symbol.st_value = offset;
			m_symbol.st_size = size;
		}
	}

	//uv_err_t UVDElfSymbol::addSymbolNameRelocation()
	{
		std::string name;
		uint32_t offset = 0;

		uv_assert_err_ret(getName(name));
		
		//The value
		uv_assert_ret(m_symbolSectionHeader);
		uv_assert_err_ret(m_symbolSectionHeader->getSymbolStringIndex(name, &offset));	
		
		ELF_SYMBOL_DEBUG
		({
			UVDData *data = NULL;
			m_relocatableData.getRelocatableData(&data);
			printf_elf_symbol_debug("name: %s, %s, symbol string table index: 0x%.8X, relocatable data: 0x%.8X, data: 0x%.8X\n", name.c_str(), m_sName.c_str(), offset, (unsigned int)&m_relocatableData, (unsigned int)data);
		});
		
		m_symbol.st_name = offset;

	}
	
	ELF_SYMBOL_DEBUG(hexdump((char *)&m_symbol, sizeof(m_symbol)));

	return UV_ERR_OK;
}

uv_err_t UVDElfSymbol::getData(UVDData **data)
{
	uint32_t isEmpty = 0;

	//CHECKME: what if its a zero sized symbol?  Do those exist?	
	uv_assert_err_ret(m_relocatableData.isEmpty(&isEmpty));

	//Undefined symbols will not have associated data
	if( isEmpty )
	{
		uv_assert_ret(data);
		*data = NULL;
	}
	else
	{
		//...and it would be an error to get the default reocatable data if it didn't exist
		//We usually want to return the relocated version
		uv_assert_err_ret(m_relocatableData.getDefaultRelocatableData(data));
	}
	return UV_ERR_OK;
}

uv_err_t UVDElfSymbol::setData(UVDData *data)
{
	/*
	{
		std::string name;
		getName(name);
		printf_elf_symbol_debug("Setting symbol(0x%.8X, %s) data to 0x%.8X\n", (unsigned int)this, name.c_str(), (unsigned int)data);
	}
	*/
	uv_assert_err_ret(m_relocatableData.setData(data));
	
	//Update to the appropriete relocatable type
	//Needed because symbols can change type depending on whether they have data or not
	//really though, this needs to be moved to grand update before writting
	//updateType();
	
	return UV_ERR_OK;
}

uv_err_t UVDElfSymbol::setBinding(uint32_t binding)
{
	m_symbol.st_info = ELF32_ST_INFO(binding, ELF32_ST_TYPE(m_symbol.st_info));
	return UV_ERR_OK;
}

uv_err_t UVDElfSymbol::setType(uint32_t type)
{
	m_symbol.st_info = ELF32_ST_INFO(ELF32_ST_BIND(m_symbol.st_info), type);
	return UV_ERR_OK;
}

uv_err_t UVDElfSymbol::addRelocation(UVDElfRelocation *relocation)
{
	printf_elf_symbol_debug("adding relocation 0x%.8X\n", (unsigned int)relocation);
	uv_assert_ret(relocation);

	//Assume all relocations are against .text for now
	{
		UVDElf *elf = NULL;
		UVDElfTextSectionHeaderEntry *relevantSectionHeader = NULL;
		UVDElfRelocationSectionHeaderEntry *relocationSectionHeader = NULL;
		
		uv_assert_ret(m_symbolSectionHeader);
		elf = m_symbolSectionHeader->m_elf;
		uv_assert_ret(elf);
		
		uv_assert_err_ret(elf->getTextSectionHeaderEntry(&relevantSectionHeader));
		uv_assert_ret(relevantSectionHeader);

		//Now get the rel table
		uv_assert_err_ret(relevantSectionHeader->getRelocatableSection(&relocationSectionHeader));
		uv_assert_ret(relocationSectionHeader);
		uv_assert_err_ret(relocationSectionHeader->addRelocation(relocation));
	}

	m_relocatableData.addFixup(relocation);
	
	/*	
	//If the relocation was relative to a section,
	//we must get its relocation table and register it
	//uv_assert_ret(m_relevantSectionHeader);
	if( m_relevantSectionHeader )
	{
		UVDElfRelocationSectionHeaderEntry *relocationSection = NULL;
		
		//Get the relocation table then
		uv_assert_err_ret(m_relevantSectionHeader->useRelocatableSection());
		uv_assert_err_ret(m_relevantSectionHeader->getRelocatableSection(&relocationSection));
		uv_assert_ret(relocationSection);
		relocationSection->addRelocation(relocation);
	}
	*/
	
	return UV_ERR_OK;
}


uv_err_t UVDElfSymbol::getRelocation(UVDElfRelocation **relocationOut)
{
	UVDElfRelocation *relocation = NULL;
	
	relocation = new UVDElfSymbolRelocation();
	uv_assert_ret(relocation);
	
	relocation->setSymbol(this);
	
	uv_assert_ret(relocationOut);
	*relocationOut = relocation;
	
	return UV_ERR_OK;
}

uv_err_t UVDElfSymbol::init()
{
	//uv_assert_err_ret(initSymbolNameRelocation());
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
	
	/*
	uv_assert_err_ret(m_headerEntryRelocatableData.getData((UVDData **)&headerDataMemory));
	//Is this already coherent?
	//Current code only calls this once, so this should actually not be called for now
	or maybe this would just crash because no data
	if( headerDataMemory )
	{
		
	}
	*/
	
	//uv_assert_err_ret(addSymbolNameRelocation());
	
	uv_assert_err_ret(UVDDataMemory::getUVDDataMemoryByTransfer(&headerDataMemory,
			(char *)&m_symbol, sizeof(Elf32_Sym), false));
	uv_assert_ret(headerDataMemory);
	printf_elf_symbol_debug("created symbol data: 0x%.8X\n", (unsigned int)headerDataMemory);
	uv_assert_err_ret(m_headerEntryRelocatableData.transferData(headerDataMemory, true));

	//Add relocations
		
	/*
	st_value, (st_size?) is determined during linking
	Since our program was already linked, we could bind st_value to the val it took during linking
	Maybe add a switch for this?
	*/
	
	//FIXME
	//There will be some local non-function symbols later (jumps), but assume all global funcs for now
	//These are now updated as needed
	//m_symbol.st_info = ELF32_ST_INFO(STB_GLOBAL, STT_FUNC);
	
	//st_other is undefined according to TIS spec
	
	//Absolute symbol: defined in this file
	//m_symbol.st_shndx = SHN_ABS;
	
	uv_assert_ret(symbolEntryRelocatableOut);
	*symbolEntryRelocatableOut = &m_headerEntryRelocatableData;
	
	return UV_ERR_OK;
}

/*
uv_err_t UVDElfSymbol::updateRelocations()
{
	//This is more of a construction thing, should it be called again here?
	//Doesn't hurt (except minor speed), leave for now
	uv_assert_err_ret(updateType());
	return UV_ERR_OK;
}
*/

/*
UVDElfNullSymbol
*/

UVDElfNullSymbol::UVDElfNullSymbol()
{
}

UVDElfNullSymbol::~UVDElfNullSymbol()
{
}

/*
uv_err_t UVDElfNullSymbol::addSymbolNameRelocation()
{
	return UV_ERR_OK;
}
*/

uv_err_t UVDElfNullSymbol::getUVDElfNullSymbol(UVDElfNullSymbol **symbolOut)
{
	/*
	symbol @ index 0x00000000
		st_name (symbol name): <NULL> (0x00000000)
		st_value: 0x00000000
		st_size: 0x00000000
		st_info: 0x00
		    bind: STB_LOCAL (0x0)
		    type: STT_NOTYPE (type is unspecified) (0x0)
		st_other: 0x00
		st_shndx: 0x00000000 (SHN_UNDEF (undefined section))
	*/

	UVDElfNullSymbol *symbol = NULL;
	
	//FIXME: WHy?
	//Don't touch anything, it should be completely zero'd
	symbol = new UVDElfNullSymbol();
	uv_assert_ret(symbol);
	
	uv_assert_ret(symbolOut);
	*symbolOut = symbol;
	
	return UV_ERR_OK;
}

uv_err_t UVDElfNullSymbol::updateType()
{
	return UV_ERR_OK;
}

/*
UVDElfVariableSymbol
*/

UVDElfVariableSymbol::UVDElfVariableSymbol()
{
}

UVDElfVariableSymbol::~UVDElfVariableSymbol()
{
}

uv_err_t UVDElfVariableSymbol::updateType()
{
	UVDData *data = NULL;
	
	uv_assert_err_ret(getData(&data));

	//Defined
	if( data )
	{
		//something requiring storage (varaible, array, etc)
		setType(STT_OBJECT);
	}
	//Undefined
	else
	{
		setType(STT_NOTYPE);
	}
	return UV_ERR_OK;
}

/*
UVDElfFunctionSymbol
*/

UVDElfFunctionSymbol::UVDElfFunctionSymbol()
{
}

UVDElfFunctionSymbol::~UVDElfFunctionSymbol()
{
}

uv_err_t UVDElfFunctionSymbol::updateType()
{
	UVDData *data = NULL;

	//FIXME: updateType() is a legacy func, call full update
	printf_elf_symbol_debug("updating type on function symbol: 0x%.8X\n", (unsigned int)this);

	uv_assert_err_ret(getData(&data));

	//Defined
	if( data )
	{
		//Function code
		setType(STT_FUNC);
	}
	//Undefined
	else
	{
		setType(STT_NOTYPE);
	}
	
	return UV_ERR_OK;
}

uv_err_t UVDElfFunctionSymbol::updateForWrite()
{
	UVDData *data = NULL;

	uv_assert_err_ret(UVDElfSymbol::updateForWrite());

	printf_elf_symbol_debug("function symbol write update\n");

	//FIXME: was there a good reason using this before?
	//Causes errors if it doesn't exist becausing trying to apply relocations against non-existent data
	uv_assert_err_ret(getData(&data));

	//Defined
	//This must be done before parent update to have the section defined so it can be st_shndx'd
	if( data )
	{
		UVDElf *elf = NULL;
		UVDElfTextSectionHeaderEntry *relevantSectionHeader = NULL;
		
		uv_assert_ret(m_symbolSectionHeader);
		elf = m_symbolSectionHeader->m_elf;
		uv_assert_ret(elf);
		
		//Link it to the .text section
		uv_assert_err_ret(elf->getTextSectionHeaderEntry(&relevantSectionHeader));
		uv_assert_ret(relevantSectionHeader);
		m_relevantSectionHeader = relevantSectionHeader;
		printf_elf_symbol_debug("linked to text section\n");
	}
	//Undefined
	else
	{
		m_relevantSectionHeader = NULL;
	}

	return UV_ERR_OK;
}

/*
UVDElfSectionSymbol
*/

UVDElfSectionSymbol::UVDElfSectionSymbol()
{
}

UVDElfSectionSymbol::~UVDElfSectionSymbol()
{
}

uv_err_t UVDElfSectionSymbol::updateType()
{
	setType(STT_SECTION);
	return UV_ERR_OK;
}

uv_err_t UVDElfSectionSymbol::updateForWrite()
{
	printf_elf_symbol_debug("section symbol <%s> write update\n", m_sName.c_str());

	//We must update get the section index
	//ELF32_ST_TYPE(ELF32_ST_BIND(m_symbol.st_info) == STT_SECTION
	UVDElf *elf = NULL;
	
	uv_assert_err_ret(UVDElfSymbol::updateForWrite());

	uv_assert_ret(m_symbolSectionHeader);
	elf = m_symbolSectionHeader->m_elf;
	uv_assert_ret(elf);

	//Must specify the section to link to
	uv_assert_ret(m_relevantSectionHeader);
	//This isn't updated until relocations
	//uv_assert_ret(m_symbol.st_shndx);
	//uv_assert_err_ret(elf->getSectionHeaderIndex(m_relevantSectionHeader, &index));
	//m_symbol.st_shndx = index;

	return UV_ERR_OK;
}

/*
UVDElfFilenameSymbol
*/

UVDElfFilenameSymbol::UVDElfFilenameSymbol()
{
}

UVDElfFilenameSymbol::~UVDElfFilenameSymbol()
{
}

uv_err_t UVDElfFilenameSymbol::updateType()
{
	setType(STT_FILE);
	return UV_ERR_OK;
}

uv_err_t UVDElfFilenameSymbol::updateForWrite()
{
	printf_elf_symbol_debug("filename symbol write update\n");
	
	uv_assert_err_ret(UVDElfSymbol::updateForWrite());

	//Specified
	setBinding(STB_LOCAL);
	m_symbol.st_shndx = SHN_ABS;

	return UV_ERR_OK;
}

/*
uv_err_t UVDElfFilenameSymbol::applyRelocationsForWrite()
{
	printf_elf_symbol_debug("filename symbol apply relocations\n");
	
	uv_assert_err_ret(UVDElfSymbol::applyRelocationsForWrite());

	return UV_ERR_OK;
}
*/

/*
UVDElfSymbolSectionHeaderEntry
*/

UVDElfSymbolSectionHeaderEntry::UVDElfSymbolSectionHeaderEntry()
{
}

UVDElfSymbolSectionHeaderEntry::~UVDElfSymbolSectionHeaderEntry()
{
}

uv_err_t UVDElfSymbolSectionHeaderEntry::init()
{
	uv_assert_err_ret(UVDElfSectionHeaderEntry::init());

	setType(SHT_SYMTAB);

	//ELF_SYMBOL_DEBUG(UVD_PRINT_STACK());
	
	//Add NULL symbol
	uv_assert_err_ret(addNullSymbol());
	//Add section symbols?
	//I'm not yet quite sure I fully understand their true purpose and if they are necessary
	/*
	uv_assert_err_ret(addSectionSymbol());
	*/
	m_sectionHeader.sh_entsize = sizeof(Elf32_Sym);
	
	return UV_ERR_OK;
}

uv_err_t UVDElfSymbolSectionHeaderEntry::addNullSymbol()
{
	UVDElfNullSymbol *symbol = NULL;

	printf_elf_symbol_debug("adding null symbol\n");
	
	uv_assert_err_ret(UVDElfNullSymbol::getUVDElfNullSymbol(&symbol));
	uv_assert_ret(symbol);	
	uv_assert_err_ret(prepareSymbolCore(symbol, FALSE));
	uv_assert_err_ret(addSymbolCore(symbol, m_symbols.begin()));

	return UV_ERR_OK;
}

uv_err_t UVDElfSymbolSectionHeaderEntry::initRelocatableData()
{
	//So that we can construct the symbol table in relocatable units
	m_fileRelocatableData = new UVDMultiRelocatableData();
	uv_assert_ret(m_fileRelocatableData);
	printf_elf_symbol_debug("relocatable section file relocatable: 0x%.8X)\n", (unsigned int)m_fileRelocatableData);
	return UV_ERR_OK;
}

uv_err_t UVDElfSymbolSectionHeaderEntry::addSymbol(UVDElfSymbol *symbol)
{
	return UV_DEBUG(addSymbolCore(symbol, m_symbols.end()));
}

uv_err_t UVDElfSymbolSectionHeaderEntry::addSymbolCore(UVDElfSymbol *symbol, const std::vector<UVDElfSymbol *>::iterator &iter)
{
	uv_assert_ret(symbol);
	//Don't do this, there are external symbols without data
	//uv_assert_ret(symbol->m_relocatableData.m_data);
	m_symbols.insert(iter, symbol);
	ELF_SYMBOL_DEBUG({
		std::string link;
		if( m_relevantSectionHeader )
		{
			link = m_relevantSectionHeader->m_name;
		}
		printf_elf_symbol_debug("adding symbol %s, link %s\n", symbol->m_sName.c_str(), link.c_str());
	});

	//ELF_SYMBOL_DEBUG(UVD_PRINT_STACK());
	return UV_ERR_OK;
}

uv_err_t UVDElfSymbolSectionHeaderEntry::findSymbol(const std::string &name, UVDElfSymbol **symbolOut)
{
	//Loop until we find a matching name
	for( std::vector<UVDElfSymbol *>::iterator iter = m_symbols.begin();
			iter != m_symbols.end(); ++iter )
	{
		UVDElfSymbol *symbol = *iter;
		std::string sSymbolName;
		
		uv_assert_ret(symbol);
		
		uv_assert_err_ret(symbol->getName(sSymbolName));
		if( sSymbolName == name )
		{
			uv_assert_ret(symbolOut);
			*symbolOut = symbol;
			return UV_ERR_OK;
		}
	}
	return UV_ERR_NOTFOUND;
}

uv_err_t UVDElfSymbolSectionHeaderEntry::prepareSymbol(UVDElfSymbol *symbol)
{
	return UV_DEBUG(prepareSymbolCore(symbol, TRUE));
}

uv_err_t UVDElfSymbolSectionHeaderEntry::prepareSymbolCore(UVDElfSymbol *symbol, uint32_t shouldAdd)
{
	uv_assert_err_ret(symbol->init());
	//So it can get string table indexes and such
	symbol->m_symbolSectionHeader = this;
	//Add it to our table list
	if( shouldAdd )
	{
		uv_assert_err_ret(addSymbol(symbol));
	}
	
	return UV_ERR_OK;
}

uv_err_t UVDElfSymbolSectionHeaderEntry::addSectionSymbol(const std::string &section)
{
	UVDElfSymbol *symbol = NULL;

	printf_elf_symbol_debug("adding section symbol: %s\n", section.c_str());

	uv_assert_err_ret(getSectionSymbol(section, &symbol));
	//uv_assert_err_ret(addSymbol(symbol));
	return UV_ERR_OK;
}

uv_err_t UVDElfSymbolSectionHeaderEntry::getSectionSymbol(const std::string &section, UVDElfSymbol **symbolOut)
{
	/*
	A sample link entry...I'm not entirely clear the purpose of these
	since each individual symbol defines its relavent section
	symbol @ index 0x00000020
		st_name (symbol name): <NULL> (0x00000000)
		st_value: 0x00000000
		st_size: 0x00000000
		st_info: 0x03
		    bind: STB_LOCAL (0x0)
		    type: STT_SECTION associated with a section) (0x3)
		st_other: 0x00
		st_shndx: 0x00000001 (.text)
	*/

	UVDElfSymbol *symbol = NULL;
		
	symbol = new UVDElfSectionSymbol();
	uv_assert_ret(symbol);
	
	uv_assert_err_ret(prepareSymbol(symbol));

	//And now make the fixups to make it a section symbol
	symbol->setType(STT_SECTION);
	//This is recomended by TIS spec
	symbol->setBinding(STB_LOCAL);

	uv_assert_ret(!section.empty());
	uv_assert_ret(m_elf);
	uv_assert_err_ret(m_elf->getSectionHeaderByName(section, &symbol->m_relevantSectionHeader));

	uv_assert_ret(symbolOut);
	*symbolOut = symbol;

	return UV_ERR_OK;
}

uv_err_t UVDElfSymbolSectionHeaderEntry::getFunctionSymbol(const std::string &name, UVDElfSymbol **symbolOut)
{
	printf_elf_symbol_debug("Getting function symbol %s\n", name.c_str());
	//Return if it already exists
	if( UV_SUCCEEDED(findSymbol(name, symbolOut)) )
	{
		printf_elf_symbol_debug("pre defined %s\n", name.c_str());
		return UV_ERR_OK;
	}
	printf_elf_symbol_debug("NOT pre defined %s\n", name.c_str());
	
	//Otherwise create it
	UVDElfSymbol *symbol = NULL;
	
	symbol = new UVDElfFunctionSymbol();
	uv_assert_ret(symbol);
	
	uv_assert_err_ret(prepareSymbol(symbol));

	printf_elf_symbol_debug("setting function symbol name to %s\n", name.c_str());
	symbol->setName(name);
	printf_elf_symbol_debug("%s\n", symbol->m_sName.c_str());	
	
	//Assume undefined by default
	symbol->setType(STT_NOTYPE);
	symbol->setBinding(STB_GLOBAL);

	uv_assert_ret(symbolOut);
	*symbolOut = symbol;
	return UV_ERR_OK;
}

uv_err_t UVDElfSymbolSectionHeaderEntry::getVariableSymbol(const std::string &name, UVDElfSymbol **symbolOut)
{
	//Return if it already exists
	if( UV_SUCCEEDED(findSymbol(name, symbolOut)) )
	{
		return UV_ERR_OK;
	}
	
	//Otherwise create it
	UVDElfSymbol *symbol = NULL;
	
	symbol = new UVDElfVariableSymbol();
	uv_assert_ret(symbol);
	
	uv_assert_err_ret(prepareSymbol(symbol));

	symbol->setName(name);
	
	//Assume undefined by default
	symbol->setType(STT_NOTYPE);
	symbol->setBinding(STB_GLOBAL);

	uv_assert_ret(symbolOut);
	*symbolOut = symbol;
	return UV_ERR_OK;
}

uv_err_t UVDElfSymbolSectionHeaderEntry::getFilenameSymbol(const std::string &name, UVDElfSymbol **symbolOut)
{
	UVDElfSymbol *symbol = NULL;
	std::vector<UVDElfSymbol *>::iterator iter;
	
	symbol = new UVDElfFilenameSymbol();
	uv_assert_ret(symbol);
	
	uv_assert_err_ret(prepareSymbolCore(symbol, FALSE));

	symbol->setName(name);

	//We need at least the null symbol first
	iter = m_symbols.begin();
	uv_assert_ret(iter != m_symbols.end());
	++iter;
	uv_assert_err_ret(addSymbolCore(symbol, iter));
	
	symbol->setType(STT_FILE);
	symbol->setBinding(STB_LOCAL);

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

/*
uv_err_t UVDElfSymbolSectionHeaderEntry::syncDataAfterUpdate()
{
	//Data is auto syncd because of UVDMultiRelocatableData and cannot be set
	return UV_ERR_OK;
}
*/

/*
uv_err_t UVDElfSymbolSectionHeaderEntry::getSymbolStringRelocatableElement(const std::string &s, UVDRelocatableElement **relocatable)
{
	uv_assert_ret(m_elf);
	uv_assert_err_ret(m_elf->getSymbolStringRelocatableElement(s, relocatable));
	return UV_ERR_OK;
}
*/

uv_err_t UVDElfSymbolSectionHeaderEntry::getSymbolStringIndex(const std::string &s, uint32_t *index)
{
	uv_assert_ret(m_elf);
	uv_assert_err_ret(m_elf->getSymbolStringIndex(s, index));
	return UV_ERR_OK;
}

uv_err_t UVDElfSymbolSectionHeaderEntry::updateForWrite()
{
	/*
	Primary goal here is to push out string table references
	*/
	
	uv_assert_err_ret(UVDElfSectionHeaderEntry::updateForWrite());

	printf_elf_symbol_debug("symbol table update for write, entries: %d\n", m_symbols.size());
	for( std::vector<UVDElfSymbol *>::iterator iter = m_symbols.begin();
			iter != m_symbols.end(); ++iter )
	{
		UVDElfSymbol *symbol = *iter;
		
		uv_assert_ret(symbol);
		uv_assert_err_ret(symbol->updateForWrite());
	}
	
	return UV_ERR_OK;
}

uv_err_t UVDElfSymbolSectionHeaderEntry::constructForWrite()
{
	printf_elf_symbol_debug(".symtab constructForWrite\n");
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
	
	uv_assert_err_ret(UVDElfHeaderEntry::constructForWrite());

	relocatableData = dynamic_cast<UVDMultiRelocatableData *>(m_fileRelocatableData);
	uv_assert_ret(relocatableData);

	//We are rebuilding this table
	//CHECKME: see if this causes a memory leak
	relocatableData->m_relocatableDatas.clear();

	printf_elf_symbol_debug("Symbols: %d\n", m_symbols.size());
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
	
	uv_assert_err_ret(relocatableData->getRelocatableData(&m_fileData));
	
	return UV_ERR_OK;
}

uv_err_t UVDElfSymbolSectionHeaderEntry::applyRelocationsForWrite()
{
	printf_elf_symbol_debug(".symtab applyRelocationsForWrite\n");

	uv_assert_err_ret(UVDElfSectionHeaderEntry::applyRelocationsForWrite());

	for( std::vector<UVDElfSymbol *>::size_type i = 0; i < m_symbols.size(); ++i )
	{
		UVDElfSymbol *symbol = m_symbols[i];
		
		uv_assert_ret(symbol);
		uv_assert_err_ret(symbol->applyRelocationsForWrite());
	}	
	
	printf_elf_symbol_debug("symbol table after relocations\n");
	ELF_SYMBOL_DEBUG(m_fileRelocatableData->hexdump());
	//DEBUG_BREAK();

	return UV_ERR_OK;
}

uv_err_t UVDElf::setSourceFilename(const std::string &s)
{
	UVDElfSymbolSectionHeaderEntry *section = NULL;

	uv_assert_err_ret(getSymbolTableSectionHeaderEntry(&section));
	uv_assert_err_ret(section->setSourceFilename(s));

	return UV_ERR_OK;
}

uv_err_t UVDElfSymbolSectionHeaderEntry::setSourceFilename(const std::string &file)
{
	/*
	STT_FILE
	A file symbol has STB_LOCAL binding, its section index is SHN_ABS, and
	it precedes the other STB_LOCAL symbols for the file, if it is present.
	*/
	UVDElfSymbol *symbol = NULL;
	
	uv_assert_err_ret(getFilenameSymbol(file, &symbol));
	
	return UV_ERR_OK;
}

uv_err_t UVDElfSymbolSectionHeaderEntry::getSymbolSectionOffset(UVDElfSymbol *symbol, uint32_t *offsetOut)
{
	//FIXME: assume single symbol object files
	uv_assert_ret(offsetOut);
	*offsetOut = 0;
	
	return UV_ERR_OK;
}

