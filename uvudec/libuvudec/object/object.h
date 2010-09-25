/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_OBJECT_OBJECT_H
#define UVD_OBJECT_OBJECT_H

#include "uvd_address.h"
#include "uvd_types.h"
#include <vector>

class UVDSection
{
public:
	UVDSection();
	virtual ~UVDSection();

	//Not all sections may have names
	//they also might be duplicate
	virtual uv_err_t getName(std::string &out);
	//We own the returned object
	//It should be deleted before the section object is
	//If its a address space only initialized at runtime or such, UV_ERR_NOTSUPPORTED should be returned
	virtual uv_err_t getData(UVDData **out);

public:
	//Not reccomended to access this directly
	//if NULL, section does not have any data associated with it
	UVDData *data;
	//If this represents a memory section, this should be filled in
	//We own this
	UVDAddressSpace *m_addressSpace;
};

/*
A ELF, raw binary, COFF, etc type object
(there is no Object class everything in UVD descends from)
*/
class UVDObject
{
public:
	UVDObject();
	virtual ~UVDObject();
	
	//Load given data as this type of object
	virtual uv_err_t init(UVDData *data) = 0;
	
	virtual uv_err_t getAddressSpaces(UVDAddressSpaces *out);
	//All of the sections returned to the best of our ability
	//We own the returned pointers, possibly in the sections
	virtual uv_err_t getSections(std::vector<UVDSection *> &out);
};

#endif

