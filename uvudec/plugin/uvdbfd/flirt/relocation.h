/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_FLIRT_PATTERN_BFD_RELOCATION_H
#define UVD_FLIRT_PATTERN_BFD_RELOCATION_H

#include <string>
#include "uvd/util/types.h"
#include "bfd.h"

/*
UVDBFDPatRelocation
*/
class UVDBFDPatModule;
class UVDBFDPatRelocation
{
public:
	UVDBFDPatRelocation();
	~UVDBFDPatRelocation();
	
	//Module offset as opposed to section
	uv_err_t offset(UVDBFDPatModule *module, uint32_t *offsetOut);

public:
	//Symbol name
	//char *name;
	std::string m_symbolName;
	//Address within section
	uv_addr_t m_address;
	uint32_t m_size;
	//relative to module, not section
	//FIXME: we should probably remove this?
	//yeah, use function instead
	//uint32_t m_offset;
};

/*
UVDBFDPatRelocations
*/
class UVDBFDPatRelocations
{
public:
	UVDBFDPatRelocations();
	~UVDBFDPatRelocations();

	//Returns UV_ERR_GENERAL if we can't add, but won't log the error
	uv_err_t isApplicable(arelent *bfdRelocation);
	uv_err_t addRelocation(arelent *bfdRelocation);

public:
	//Sorted lowest address first
	std::vector<UVDBFDPatRelocation *> m_relocations;
	UVDBFDPatModule *m_module;
};

#endif

