/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvdasm/config_symbol.h"
#include <string>

UVDSymbol::UVDSymbol()
{
	m_type = UVD_SYMBOL_UNKNOWN;
	m_mem = NULL;
	m_operator = NULL;
}

UVDSymbol::~UVDSymbol()
{
	deinit();
}

uv_err_t UVDSymbol::init()
{
	return UV_ERR_OK;
}

uv_err_t UVDSymbol::deinit()
{
	switch( m_type )
	{
	case UVD_SYMBOL_MEM:
		delete m_mem;
		m_mem = NULL;
	case UVD_SYMBOL_OPERATOR:
		delete m_operator;
		m_operator = NULL;
	};
	m_type = UVD_SYMBOL_UNKNOWN;

	return UV_ERR_OK;
}

std::string symbolTypeString(int type)
{
	switch( type )
	{
	case UVD_SYMBOL_MEM:
		return "MEMORY";
	case UVD_SYMBOL_OPERATOR:
		return "OPERATOR";
	default:
		return "<UNKNOWN>";
	};
}

std::string UVDSymbol::toString()
{
	std::string ret = m_key + " (" + symbolTypeString(m_type) + ") = ";
	switch( m_type )
	{
	case UVD_SYMBOL_MEM:
		if( m_mem != NULL )
		{
			ret += m_mem->m_name;
		}
		break;
	case UVD_SYMBOL_OPERATOR:
		if( m_operator != NULL )
		{
			ret += m_operator->m_operator;
		}
	}
	return ret;
}

uv_err_t UVDSymbol::getValue(UVDAddressSpace **memoryShared)
{
	uv_assert_ret(memoryShared);
	uv_assert_ret(m_type == UVD_SYMBOL_MEM);
	*memoryShared = m_mem;
	return UV_ERR_OK;
}

uv_err_t UVDSymbol::setValue(UVDAddressSpace *memoryShared)
{
	uv_assert_ret(memoryShared);
	m_mem = memoryShared;
	m_type = UVD_SYMBOL_MEM;
	return UV_ERR_OK;
}

uv_err_t UVDSymbol::getValue(UVDOperator **oper)
{
	uv_assert_ret(oper);
	uv_assert_ret(m_type == UVD_SYMBOL_OPERATOR);
	*oper = m_operator;
	return UV_ERR_OK;
}

uv_err_t UVDSymbol::setValue(UVDOperator *oper)
{
	uv_assert_ret(oper);
	m_operator = oper;
	m_type = UVD_SYMBOL_OPERATOR;
	return UV_ERR_OK;
}

UVDSymbolMap::UVDSymbolMap()
{
}

UVDSymbolMap::~UVDSymbolMap()
{
	for( SymbolMapMap::iterator iter = m_map.begin(); iter != m_map.end(); ++iter )
	{
		delete (*iter).second;
	}
	m_map.clear();
}

uv_err_t UVDSymbolMap::init()
{
	UVDOperator *addOperator = NULL;
	UVDOperator *subtractOperator = NULL;
	
	uv_assert_err_ret(UVDOperator::getOperator(UVD_OPERATOR_ADD, &addOperator));
	uv_assert_err_ret(setSym("ADD", addOperator));

	uv_assert_err_ret(UVDOperator::getOperator(UVD_OPERATOR_SUBTRACT, &subtractOperator));
	uv_assert_err_ret(setSym("SUBTRACT", subtractOperator));

	return UV_ERR_OK;
}

uv_err_t UVDSymbolMap::setSym(const std::string &key, UVDSymbol *sym, UVDSymbol **old)
{
	//Disassalow NULL symbols
	uv_assert_ret(sym);

	m_map[key] = sym;
	return UV_ERR_OK;
}

uv_err_t UVDSymbolMap::setSym(const std::string &key, UVDAddressSpace *mem_sym)
{
	UVDSymbol *sym = NULL;
	
	sym = new UVDSymbol();
	uv_assert_ret(sym);
	sym->m_mem = mem_sym;
	sym->m_type = UVD_SYMBOL_MEM;

	return UV_DEBUG(setSym(key, sym, NULL));
}

uv_err_t UVDSymbolMap::setSym(const std::string &key, UVDOperator *operatorSymbol)
{
	UVDSymbol *sym = NULL;
	
	sym = new UVDSymbol();
	uv_assert_ret(sym);
	sym->m_operator = operatorSymbol;
	sym->m_type = UVD_SYMBOL_OPERATOR;

	return UV_DEBUG(setSym(key, sym, NULL));
}

uv_err_t UVDSymbolMap::getSym(const std::string &key, UVDSymbol **sym)
{
	if( !sym )
	{
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	if( m_map.find(key) == m_map.end() )
	{
		printf_debug("Could not find: %s\n", key.c_str());
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	*sym = m_map[key];
	return UV_ERR_OK;
}

uv_err_t UVDSymbolMap::getSym(const std::string &key, UVDAddressSpace **memorySymbol)
{
	UVDSymbol *symbol = NULL;

	uv_assert_ret(memorySymbol);
	uv_assert_err_ret(getSym(key, &symbol));
	uv_assert_err_ret(symbol->getValue(memorySymbol));
	return UV_ERR_OK;
}

uv_err_t UVDSymbolMap::getSym(const std::string &key, UVDOperator **operatorSymbol)
{
	UVDSymbol *symbol = NULL;

	uv_assert_ret(operatorSymbol);
	uv_assert_err_ret(getSym(key, &symbol));
	uv_assert_err_ret(symbol->getValue(operatorSymbol));
	return UV_ERR_OK;
}

void UVDSymbolMap::print()
{
	printf_debug("Symbol table:\n");
	for( SymbolMapMap::iterator iter = m_map.begin(); iter != m_map.end(); ++iter )
	{
		std::string key = (*iter).first;
		UVDSymbol *value = (*iter).second;
		std::string sValue;
		
		if( !value )
		{
			sValue = "<NULL>"; 
		}
		else
		{
			sValue = value->toString();
		}
		
		printf_debug("m_map[%s] = %s\n", key.c_str(), sValue.c_str());
	}
}

