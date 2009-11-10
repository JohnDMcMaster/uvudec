#include "uvd_elf.h"
#include <string>

/*
UVDElfSymbol
*/

UVDElfSymbol::UVDElfSymbol()
{
	m_data = NULL;
}

UVDElfSymbol::~UVDElfSymbol()
{
}

void UVDElfSymbol::setSymbolName(const std::string &sName)
{
	m_sName = sName;
}

std::string UVDElfSymbol::getSymbolName()
{
	return m_sName;
}

uv_err_t UVDElfSymbol::getData(UVDData **data)
{
	uv_assert_ret(data);
	*data = m_data;
	return UV_ERR_OK;
}

void UVDElfSymbol::setData(UVDData *data)
{
	m_data = data;
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
		
		sSymbolName = symbol->getSymbolName();
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
	symbol->setSymbolName(sName);
	uv_assert_err_ret(addSymbol(symbol));
	
	uv_assert_ret(symbolOut);
	*symbolOut = symbol;
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
