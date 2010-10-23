/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/config/config.h"
#include "uvd/assembly/symbol.h"

UVDConfigSymbols::UVDConfigSymbols()
{
}

UVDConfigSymbols::~UVDConfigSymbols()
{
	UV_DEBUG(deinit());
}

uv_err_t UVDConfigSymbols::init()
{
	m_autoNameUvudecPrefix = "";
	m_autoNameMangeledDataSource = FALSE;
	m_autoNameMangeledDataSourceDelim = "__";

	m_autoNameUnknownPrefix = "unk_";
	m_autoNameFunctionPrefix = "sub_";
	m_autoNameLabelPrefix = "lab_";
	m_autoNameROMPrefix = "const_";
	m_autoNameVariablePrefix = "var_";
	
	return UV_ERR_OK;
}

uv_err_t UVDConfigSymbols::deinit()
{
	return UV_ERR_OK;
}

/*
For generating unknown symbol stuff
*/
uv_err_t UVDConfigSymbols::getSymbolTypeNamePrefix(int symbolType, std::string &ret)
{
	switch( symbolType )
	{
	case UVD__SYMBOL_TYPE__UNKNOWN:
		ret = m_autoNameUnknownPrefix;
		break;
	case UVD__SYMBOL_TYPE__FUNCTION:
		ret = m_autoNameFunctionPrefix;
		break;
	case UVD__SYMBOL_TYPE__LABEL:
		ret = m_autoNameLabelPrefix;
		break;
	case UVD__SYMBOL_TYPE__ROM:
		ret = m_autoNameROMPrefix;
		break;
	case UVD__SYMBOL_TYPE__VARIABLE:
		ret = m_autoNameVariablePrefix;
		break;
	default:
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	
	return UV_ERR_OK;
}


