#include "uvd_elf.h"
#include <string>

/*
UVDElfSymbol
*/

UVDElfSymbol::UVDElfSymbol()
{
	//m_relocatableData = NULL;
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
	m_relocatableData.setData(data);
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
	symbol->setName(sName);
	uv_assert_err_ret(addSymbol(symbol));
	
	uv_assert_ret(symbolOut);
	*symbolOut = symbol;
	return UV_ERR_OK;
}

uv_err_t UVDElfSymbolSectionHeaderEntry::updateData()
{
	/*
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
