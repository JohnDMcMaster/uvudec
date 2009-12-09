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
	m_relocatableData = NULL;
}

UVDBinarySymbol::~UVDBinarySymbol()
{
}

uv_err_t UVDBinarySymbol::init()
{
	m_relocatableData = new UVDRelocatableData();
	uv_assert_ret(m_relocatableData);
	
	return UV_ERR_OK;
}

void UVDBinarySymbol::setSymbolName(const std::string &name)
{
	m_symbolName = name;
}

uv_err_t UVDBinarySymbol::getSymbolName(std::string &name)
{
	name = m_symbolName;
	return UV_ERR_OK;
}

uv_err_t UVDBinarySymbol::addRelocation(uint32_t relocatableDataOffset, uint32_t relocatableDataSize)
{
	return UV_DEBUG(UV_ERR_GENERAL);
	/*
	UVDBinaryRelocation *relocation = NULL;
	
	uv_assert_err_ret(UVDBinaryRelocation::getUVDBinaryRelocation(&relocation,
			relocatableDataOffset, relocatableDataSize));
	uv_assert_ret(relocation);
	m_relocations.push_back(relocation);
	
	return UV_ERR_OK;
	*/
}

uv_err_t UVDBinarySymbol::getSymbolAddress(uint32_t *symbolAddress)
{
	return UV_DEBUG(m_symbolAddress.getDynamicValue(symbolAddress));
}

/*
uv_err_t UVDBinarySymbol::getOffset(uint32_t *offset)
{
	uv_assert_ret(offset);
	uv_assert_err_ret(m_symbolAddress.getDynamicValue(offset)).;
	return UV_ERR_OK;
}
*/

uv_err_t UVDBinarySymbol::addRelocations(const UVDBinarySymbol *otherSymbol)
{
	uv_assert_ret(m_relocatableData);	
	//We can simply copy the relocation list over
	uv_assert_ret(otherSymbol);
	uv_assert_ret(otherSymbol->m_relocatableData);
	//Copy for now..do add later if needed
	m_relocatableData->m_fixups = otherSymbol->m_relocatableData->m_fixups;

	return UV_ERR_OK;	
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
	uv_assert_ret(!symbol->m_symbolName.empty());
	m_symbols[symbol->m_symbolName] = symbol;
	return UV_ERR_OK;
}

uv_err_t UVDBinarySymbolManager::collectRelocations(UVDBinaryFunction *function)
{
	uint32_t functionAddress = 0;
	UVDBinaryFunctionInstance *functionInstance = NULL;
	
	uv_assert_ret(function);
	
	functionInstance = function->getFunctionInstance();
	uv_assert_ret(functionInstance);

	uv_assert_err_ret(functionInstance->getSymbolAddress(&functionAddress));

	for( std::map<std::string, UVDBinarySymbol *>::iterator iter = m_symbols.begin(); iter != m_symbols.end(); ++iter )
	{
		UVDBinarySymbol *binarySymbol = (*iter).second;
		uv_assert_ret(binarySymbol);
		uint32_t curSymbolAddress = 0;

		uv_assert_err_ret(binarySymbol->getSymbolAddress(&curSymbolAddress));
		
		//Is this the matching symbol we are looking for?
		if( functionAddress == curSymbolAddress )
		{
			uv_assert_err_ret(doCollectRelocations(function, binarySymbol));
			break;
		}
	}
	
	return UV_ERR_OK;
}

uv_err_t UVDBinarySymbolManager::doCollectRelocations(UVDBinaryFunction *function, UVDBinarySymbol *analysisSymbol)
{
	UVDBinaryFunctionInstance *functionInstance = NULL;
		
	functionInstance = function->getFunctionInstance();
	uv_assert_ret(functionInstance);

	uv_assert_err_ret(functionInstance->addRelocations(analysisSymbol));

	return UV_ERR_OK;	
}
