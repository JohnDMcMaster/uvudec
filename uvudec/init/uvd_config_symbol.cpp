/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#include "uvd_config_symbol.h"
#include <string>

UVDSymbol::UVDSymbol()
{
	m_type = UVD_SYMBOL_UNKNOWN;
	m_mem = NULL;
}

uv_err_t UVDSymbol::init()
{
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

uv_err_t UVDSymbol::getValue(UVDMemoryShared **memoryShared)
{
	uv_assert_ret(memoryShared);
	uv_assert_ret(m_type == UVD_SYMBOL_MEM);
	*memoryShared = m_mem;
	return UV_ERR_OK;
}

uv_err_t UVDSymbol::setValue(UVDMemoryShared *memoryShared)
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

uv_err_t UVDSymbolMap::setSym(const std::string &key, UVDMemoryShared *mem_sym)
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

uv_err_t UVDSymbolMap::getSym(const std::string &key, UVDMemoryShared **memorySymbol)
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

#if 0
/* 
String map of address spaces mapping to actual addresses?
Then map keyed on addresses
Should keep track of called count
*/
struct uvd_sym_map_t *g_sym_root = NULL;

uv_err_t uvd_set_sym_core(struct uvd_sym_map_t **root_in, const std::string key, struct uvd_sym_t *value, struct uvd_sym_t **old_value)
{
	uv_err_t rc = UV_ERR_GENERAL;

	uv_assert(root_in);
	uv_assert(key);
	/* Let value be NULL */
	uv_assert(value);

	/* Base case: previous key did not exist */
	if( !(*root_in) )
	{
		struct uvd_sym_map_t *root_temp;

		root_temp = uvd_sym_map_alloc();
		uv_assert_all(root_temp);

		root_temp->key = strdup(key);
		uv_assert_all(root_temp->key);

		root_temp->left = NULL;
		root_temp->right = NULL;
		root_temp->value = value;
		*root_in = root_temp;
		if( old_value )
		{
			*old_value = NULL;
		}
	}
	/* Node exists */
	else
	{
		struct uvd_sym_map_t *root = *root_in;
		int comp = 0;
		
		comp = strcmp(key, root->key);
		
		/* Previous is less than new */
		if( comp < 0 )
		{
			uv_assert_err(uvd_set_sym_core(&root->left, key, value, old_value));
		}
		/* Previous is greater than new */
		else if( comp > 0 )
		{
			uv_assert_err(uvd_set_sym_core(&root->right, key, value, old_value));
		}
		/* Same key: replace */
		else
		{
			if( old_value )
			{
				*old_value = root->value;
			}
			else
			{
				uvd_sym_free(root->value);
			}
			root->value = value;
			/* Key is still valid */
		}
	}

	rc = UV_ERR_OK;

error:
	return UV_DEBUG(rc);
}

uv_err_t uvd_set_sym(const std::string key, struct uvd_sym_t *value, struct uvd_sym_t **old_value)
{
	return UV_DEBUG(uvd_set_sym_core(&g_sym_root, key, value, old_value));
}

uv_err_t uvd_get_sym_core(struct uvd_sym_map_t *root, const std::string key, struct uvd_sym_t **value)
{
	uv_err_t rc = UV_ERR_GENERAL;
	int comp = 0;

	UV_ENTER();
	uv_assert_all(root);
	uv_assert(root->key);
	uv_assert(key);
	uv_assert(value);
	
	printf_debug("assert done\n"); fflush(stdout);
	
	printf_debug("Want: %s, cur key: %s, left: 0x%.8X, right: 0x%.8X\n", key, root->key, (unsigned int)root->left, (unsigned int)root->right);
	fflush(stdout);
	
	comp = strcmp(key, root->key);
	/* Previous is less than check */
	if( comp < 0 )
	{
		uv_assert_err(uvd_get_sym_core(root->left, key, value));
	}
	/* Previous is greater than check */
	else if( comp > 0 )
	{
		uv_assert_err(uvd_get_sym_core(root->right, key, value));
	}
	/* Same key: match */
	else
	{
		/* Allow NULL key? */
		/* uv_assert(root->value); */
		*value = root->value;
	}

	rc = UV_ERR_OK;

error:
	return UV_DEBUG(rc);
}

uv_err_t uvd_get_sym(const std::string key, struct uvd_sym_t **value)
{
	return UV_DEBUG(uvd_get_sym_core(g_sym_root, key, value));
}

uv_err_t uvd_get_sym_mem(const std::string key, struct uvd_mem_shared_t **value)
{
	uv_err_t rc = UV_ERR_GENERAL;
	struct uvd_sym_t *value_sym = NULL;
	
	uv_assert_all(value);
	uv_assert_err(uvd_get_sym(key, &value_sym));
	uv_assert_all(value_sym->type == UVD_SYMBOL_MEM);
	
	*value = value_sym->mem;
	
	rc = UV_ERR_OK;

error:
	return UV_DEBUG(rc);
}
#endif
