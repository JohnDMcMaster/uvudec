/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#ifndef UVD_BINARY_SYMBOL_H
#define UVD_BINARY_SYMBOL_H

#include "uvd_data.h"

#include <string>
#include <vector>

/*
A symbol as if found in an object file, not necessarily ELF format
A format easily workable in memory
*/
class UVDBinarySymbol
{
public:
	UVDBinarySymbol();
	~UVDBinarySymbol();

	void setName(const std::string &name);

public:
	//The symbol's (function's/variable's) name
	std::string m_name;
	//The actual data this symbol represents, if its resolved
	//If the symbol is not resolved, data will be NULL
	UVDData *m_data;
};

/*
A function is a peice of code with some relocatables in it
If the function is external, it may or may not have a known value
*/
class UVDFunctionBinarySymbol : public UVDBinarySymbol
{
public:
	UVDFunctionBinarySymbol();
	~UVDFunctionBinarySymbol();
};

/*
A variable is a reserved location to store data
*/
class UVDVariableBinarySymbol : public UVDBinarySymbol
{
public:
	UVDVariableBinarySymbol();
	~UVDVariableBinarySymbol();
};

/*
A symbol of read only data
Their value cannot be changed
ELF notes
	Do not have a true symbol after compilation
	Like anonymous symbols
	These appear in the .bss section
*/
class UVDROMBinarySymbol : public UVDBinarySymbol
{
public:
	UVDROMBinarySymbol();
	~UVDROMBinarySymbol();
};

class UVDBinarySymbolManager
{
public:
	UVDBinarySymbolManager();
	~UVDBinarySymbolManager();

	uv_err_t findSymbol(std::string &name, UVDBinarySymbol **symbol);
	uv_err_t addSymbol(UVDBinarySymbol *symbol);

public:
	//Symbol names must be unique
	std::map<std::string, UVDBinarySymbol *> m_symbols;
};

#endif
