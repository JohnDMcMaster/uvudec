/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
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

#include "uvd_elf.h"
#include "uvd_relocation.h"
#include "uvd_util.h"
#include <string>

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
void UVDElfSymbol::setSymbolName(const std::string &sName)
{
	m_sName = sName;
}

std::string UVDElfSymbol::getSymbolName()
{
	return m_sName;
}
*/

uv_err_t UVDElfSymbol::updateForWrite()
{
	return UV_ERR_OK;
}

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
	
	//Update to the appropriete relocatable type
	//Needed because symbols can change type depending on whether they have data or not
	//really though, this needs to be moved to grand update before writting
	updateType();
	
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
printf("adding relocation 0x%.8X\n", (unsigned int)relocation);
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

/*
UVDElfNullSymbol
*/

UVDElfNullSymbol::UVDElfNullSymbol()
{
}

UVDElfNullSymbol::~UVDElfNullSymbol()
{
}

uv_err_t UVDElfNullSymbol::addSymbolNameRelocation()
{
	return UV_ERR_OK;
}

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
	
	uv_assert_err_ret(UVDElfNullSymbol::getUVDElfNullSymbol(&symbol));
	uv_assert_ret(symbol);	
	uv_assert_err_ret(addSymbol(symbol));

	return UV_ERR_OK;
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

uv_err_t UVDElfSymbolSectionHeaderEntry::prepareSymbol(UVDElfSymbol *symbol)
{
	uv_assert_err_ret(symbol->init());
	//So it can get string table indexes and such
	symbol->m_symbolSectionHeader = this;
	//Add it to our table list
	uv_assert_err_ret(addSymbol(symbol));
	
	return UV_ERR_OK;
}

uv_err_t UVDElfSymbolSectionHeaderEntry::addSectionSymbol(const std::string &section)
{
	UVDElfSymbol *symbol = NULL;

	uv_assert_err_ret(getSectionSymbol(section, &symbol));
	uv_assert_err_ret(addSymbol(symbol));
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
	symbol->setBinding(STB_GLOBAL);

	uv_assert_ret(m_elf);
	uv_assert_err_ret(m_elf->getSectionHeaderByName(section, &symbol->m_relevantSectionHeader));

	uv_assert_ret(symbolOut);
	*symbolOut = symbol;

	return UV_ERR_OK;
}

uv_err_t UVDElfSymbolSectionHeaderEntry::getFunctionSymbol(const std::string &sName, UVDElfSymbol **symbolOut)
{
	//Return if it already exists
	if( UV_SUCCEEDED(findSymbol(sName, symbolOut)) )
	{
		return UV_ERR_OK;
	}
	
	//Otherwise create it
	UVDElfSymbol *symbol = NULL;
	
	symbol = new UVDElfFunctionSymbol();
	uv_assert_ret(symbol);
	
	uv_assert_err_ret(prepareSymbol(symbol));

	symbol->setName(sName);
	
	//Assume undefined by default
	symbol->setType(STT_NOTYPE);
	symbol->setBinding(STB_GLOBAL);

	uv_assert_ret(symbolOut);
	*symbolOut = symbol;
	return UV_ERR_OK;
}

uv_err_t UVDElfSymbolSectionHeaderEntry::getVariableSymbol(const std::string &sName, UVDElfSymbol **symbolOut)
{
	//Return if it already exists
	if( UV_SUCCEEDED(findSymbol(sName, symbolOut)) )
	{
		return UV_ERR_OK;
	}
	
	//Otherwise create it
	UVDElfSymbol *symbol = NULL;
	
	symbol = new UVDElfVariableSymbol();
	uv_assert_ret(symbol);
	
	uv_assert_err_ret(prepareSymbol(symbol));

	symbol->setName(sName);
	
	//Assume undefined by default
	symbol->setType(STT_NOTYPE);
	symbol->setBinding(STB_GLOBAL);

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
printf(".symtab update core\n");
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

	//We are rebuilding this table
	relocatableData->m_relocatableDatas.clear();

printf("Symbols: %d\n", m_symbols.size());
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

uv_err_t UVDElfSymbolSectionHeaderEntry::syncDataAfterUpdate()
{
	//Data is auto syncd because of UVDMultiRelocatableData and cannot be set
	return UV_ERR_OK;
}

uv_err_t UVDElfSymbolSectionHeaderEntry::getSymbolStringRelocatableElement(const std::string &s, UVDRelocatableElement **relocatable)
{
	uv_assert_ret(m_elf);
	uv_assert_err_ret(m_elf->getSymbolStringRelocatableElement(s, relocatable));
	return UV_ERR_OK;
}

uv_err_t UVDElfSymbolSectionHeaderEntry::updateForWrite()
{
	uv_assert_err_ret(UVDElfSectionHeaderEntry::updateForWrite());

printf("symbol table update for write, entries: %d\n", m_symbols.size());
	for( std::vector<UVDElfSymbol *>::iterator iter = m_symbols.begin();
			iter != m_symbols.end(); ++iter )
	{
		UVDElfSymbol *symbol = *iter;
		
		uv_assert_ret(symbol);
		uv_assert_err_ret(symbol->updateForWrite());
	}
	return UV_ERR_OK;
}

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

uv_err_t UVDElfSymbol::init()
{
	//uv_assert_err_ret(initSymbolNameRelocation());
	return UV_ERR_OK;
}

uv_err_t UVDElfSymbol::addSymbolNameRelocation()
{
	std::string name;
	UVDRelocatableElement *relocatable = NULL;
	UVDRelocationFixup *offsetFixup = NULL;

	uv_assert_err_ret(getName(name));
	//The value
	uv_assert_ret(m_symbolSectionHeader);
	uv_assert_err_ret(m_symbolSectionHeader->getSymbolStringRelocatableElement(name, &relocatable));
	//Where
	offsetFixup = new UVDRelocationFixup(relocatable,
			OFFSET_OF(Elf32_Sym, st_name), sizeof(m_symbol.st_name));
	uv_assert_ret(offsetFixup);
	
	m_headerEntryRelocatableData.addFixup(offsetFixup);

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
	
	uv_assert_err_ret(addSymbolNameRelocation());
	
	uv_assert_err_ret(UVDDataMemory::getUVDDataMemoryByTransfer(&headerDataMemory,
			(char *)&m_symbol, sizeof(Elf32_Sym), false));
	uv_assert_ret(headerDataMemory);
	uv_assert_err_ret(m_headerEntryRelocatableData.setData(headerDataMemory));

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

uv_err_t UVDElfSymbol::updateRelocations()
{
	//This is more of a construction thing, should it be called again here?
	//Doesn't hurt (except minor speed), leave for now
	uv_assert_err_ret(updateType());
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
	
printf("updating type on function symbol: 0x%.8X\n", (unsigned int)this);

	uv_assert_err_ret(getData(&data));

	//Defined
	if( data )
	{
		UVDElf *elf = NULL;
		UVDElfTextSectionHeaderEntry *relevantSectionHeader = NULL;
		
		uv_assert_ret(m_symbolSectionHeader);
		elf = m_symbolSectionHeader->m_elf;
		uv_assert_ret(elf);
		
		//Function code
		setType(STT_FUNC);
		//Link it to the .text section
		uv_assert_err_ret(elf->getTextSectionHeaderEntry(&relevantSectionHeader));
		uv_assert_ret(relevantSectionHeader);
		m_relevantSectionHeader = relevantSectionHeader;
printf("linked to text section\n");
	}
	//Undefined
	else
	{
		setType(STT_NOTYPE);
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
printf("section symbol write update\n");

	//We must update get the section index
	//ELF32_ST_TYPE(ELF32_ST_BIND(m_symbol.st_info) == STT_SECTION
	uint32_t index = 0;
	UVDElf *elf = NULL;

	uv_assert_ret(m_symbolSectionHeader);
	elf = m_symbolSectionHeader->m_elf;
	uv_assert_ret(elf);

	//Must specify the section to link to
	uv_assert_ret(m_relevantSectionHeader);
	uv_assert_err_ret(elf->getSectionHeaderIndex(m_relevantSectionHeader, &index));
	m_symbol.st_shndx = index;

	return UV_ERR_OK;
}
