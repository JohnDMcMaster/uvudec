/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

/*
These are config symbols

FIXME FIXME FIXME
Okay so this file is the reason why I switched from C to C++
Without STL, the easiest way to make a symbol mapper was to shove a bunch of casting operations using a shared core
Really, this is horrible code and could be replaced with some simple STL stuff (partially done here already) without much trouble
Also, its named poorly since you'd expect UVDSymbol to be what instead became UVDBinarySymbol
*/

#pragma once

#include <string>
#include <map>
#include "uvd/assembly/address.h"
#include "uvdasm/interpreter/operator.h"

/* Symbol is not referenced */
#define UVD_SYMBOL_UNKNOWN			0
/* Some sort of standard variable argument, ex: u8_0 */
#define UVD_SYMBOL_VAR				1
/* Defined by the .MEM section */
#define UVD_SYMBOL_MEM				2
/* To do math stuffs */
#define UVD_SYMBOL_OPERATOR			3

class UVDSymbol
//struct UVD_SYMBOL_t
{
public:
	UVDSymbol();
	~UVDSymbol();
	uv_err_t init();
	uv_err_t deinit();

	std::string toString();
	
	uv_err_t getValue(UVDAddressSpace **memoryShared);
	uv_err_t setValue(UVDAddressSpace *memoryShared);
	uv_err_t getValue(UVDOperator **oper);
	uv_err_t setValue(UVDOperator *oper);

public:
	std::string m_key;
	int m_type;

	/*
	union value
	{
		//Generic access value
		unsigned int m_varient;
	*/
		//Memory type
		UVDAddressSpace *m_mem;
		UVDOperator *m_operator;
	//};
};

class UVDSymbolMap
{
public:
	UVDSymbolMap();
	~UVDSymbolMap();
	uv_err_t init();
	
	uv_err_t setSym(const std::string &key, UVDSymbol *sym, UVDSymbol **old = NULL);
	uv_err_t setSym(const std::string &key, UVDAddressSpace *mem_sym);
	uv_err_t setSym(const std::string &key, UVDOperator *operatorSymbol);

	uv_err_t getSym(const std::string &key, UVDSymbol **sym);
	uv_err_t getSym(const std::string &key, UVDAddressSpace **mem_sym);
	uv_err_t getSym(const std::string &key, UVDOperator **operatorSymbol);

	void print();
	
	typedef std::map<std::string, UVDSymbol *> SymbolMapMap;

public:
	//Maps names to symbols
	SymbolMapMap m_map;
};
