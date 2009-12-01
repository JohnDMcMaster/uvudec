/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#include "uvd_binary_symbol.h"
#include "uvd_binary_function.h"

/*
UVDBinarySymbol
*/

UVDBinarySymbol::UVDBinarySymbol()
{
	m_data = NULL;
}

UVDBinarySymbol::~UVDBinarySymbol()
{
}

void setName(const std::string &name)
{
}

/*
UVDBinarySymbolManager
*/

UVDBinarySymbolManager::UVDBinarySymbolManager()
{
}

UVDBinarySymbolManager::~UVDBinarySymbolManager()
{
}

uv_err_t UVDBinarySymbolManager::findSymbol(std::string &name, UVDBinarySymbol **symbolIn)
{
	std::map<std::string, UVDBinarySymbol *>::iterator iter = m_symbols.find(name);
	UVDBinarySymbol *symbol = NULL;
	
	if( iter == m_symbols.end() )
	{
		uv_assert_ret(symbolIn);
		*symbolIn = NULL;
		return UV_ERR_NOTFOUND;
	}
	
	symbol = (*iter).second;
	uv_assert_ret(symbol);
	
	uv_assert_ret(symbolIn);
	*symbolIn = symbol;
	
	return UV_ERR_OK;
}

uv_err_t UVDBinarySymbolManager::addSymbol(UVDBinarySymbol *symbol)
{
	uv_assert_ret(symbol);
	m_symbols[symbol->m_name] = symbol;
	return UV_ERR_OK;
}
