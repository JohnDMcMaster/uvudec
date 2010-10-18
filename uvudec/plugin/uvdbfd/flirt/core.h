/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_FLIRT_PATTERN_BFD_CORE_H
#define UVD_FLIRT_PATTERN_BFD_CORE_H

#include "uvdbfd/flirt/section.h"
#include "uvd/util/string_writer.h"

/*
UVDBFDPatCore
*/
class UVDBFDPatCore
{
public:
	UVDBFDPatCore();
	~UVDBFDPatCore();
	uv_err_t init(bfd *abfd);
	uv_err_t deinit();

	uv_err_t generate();

	uv_err_t buildSymbolTable();
	uv_err_t setFunctionSizes();
	uv_err_t placeRelocationsIntoFunctions();
	uv_err_t print();

public:
	//bfd output is derrived from
	//we do not own it
	bfd *m_bfd;
	//Contiguously allocated symbol table
	//we own this
	asymbol **m_contiguousSymbolTable;
	UVDBFDPatSections m_sections;
	//Gathers output during print
	UVDStringWriter m_writer;
};

#endif

