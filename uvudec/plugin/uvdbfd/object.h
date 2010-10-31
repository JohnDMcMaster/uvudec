/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVDBFD_OBJECT_H
#define UVDBFD_OBJECT_H

#include "bfd.h"
#include "uvd/object/object.h"

class UVDBFDObject : public UVDObject
{
public:
	static uv_err_t canLoad(const UVDData *data, const UVDRuntimeHints &hints, uvd_priority_t *confidence, void *user);
	static uv_err_t tryLoad(UVDData *data, const UVDRuntimeHints &hints, UVDObject **out, void *user);

public:
	UVDBFDObject();
	virtual ~UVDBFDObject();
	
	virtual uv_err_t init(const UVDData *data);
	//Rebuild section table off of m_bfd
	uv_err_t rebuildSections();

public:
	bfd *m_bfd;
};

#endif

