/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_OBJECT_BINARY_H
#define UVD_OBJECT_BINARY_H

#include "object/object.h"

/*
A raw binary object as would be ripped off of a ROM (ex: EPROM)
*/
class UVDBinaryObject : public UVDObject
{
public:
	UVDBinaryObject();
	~UVDBinaryObject();

	virtual uv_err_t init(UVDData *data) = 0;

	static uv_err_t tryLoad(const UVDData *data, const std::string &architecture, UVDObject **out);
};

#endif

