/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVDOBJBIN_OBJECT_H
#define UVDOBJBIN_OBJECT_H

#include "uvd/object/object.h"
#include "uvd/util/types.h"
#include "uvd/data/data.h"
#include "uvd/object/object.h"

/*
A raw binary object as would be ripped off of a ROM (ex: EPROM)
*/
class UVDBinaryObject : public UVDObject
{
public:
	UVDBinaryObject();
	~UVDBinaryObject();

	virtual uv_err_t init(UVDData *data);

	//Returns UV_ERR_NOTSUPPORTED if can't load
	static uv_err_t canLoad(const UVDData *data, const std::string &object, const std::string &architecture, uvd_priority_t *confidence);
	//How could this fail?
	//indicates we need a priorty system to load ELF files first etc
	static uv_err_t tryLoad(UVDData *data, const std::string &object, const std::string &architecture, UVDObject **out);

public:
};

#endif

