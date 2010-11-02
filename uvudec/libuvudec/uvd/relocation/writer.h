/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_RELOCATION_WRITER_H
#define UVD_RELOCATION_WRITER_H

#include "uvd/util/types.h"
#include "uvd/data/data.h"
#include "uvd/relocation/relocation.h"

class UVDRelocationWriter
{
public:
	UVDRelocationWriter();
	~UVDRelocationWriter();

	//Output data should be deleted by callee
	//It is reconstructed every time and not cached
	uv_err_t constructBinary(UVDData **data);

	//Phase 1
	virtual uv_err_t updateForWrite() = 0;

	//Phase 2
	virtual uv_err_t construct() = 0;

	//Phase 3
	virtual uv_err_t applyRelocations() = 0;

	uv_err_t addRelocatableData(char *dataIn, uint32_t dataInSize, bool shouldFreeData = false);
	uv_err_t addRelocatableData(UVDData *data, bool shouldFreeData = false);

public:
	UVDRelocationManager m_relocationManager;
};

#endif

