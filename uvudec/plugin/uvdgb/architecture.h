/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVDGB_ARCHITECTURE_H
#define UVDGB_ARCHITECTURE_H

#include "uvd/architecture/architecture.h"

class UVDGBArchitecture : public UVDArchitecture
{
public:
	UVDGBArchitecture();
	virtual ~UVDGBArchitecture();

	virtual uv_err_t init();	
	virtual uv_err_t deinit();	

	virtual uv_err_t getInstruction(UVDInstruction **out);

public:	
	//UVDDisasmOpcodeLookupTable *m_opcodeTable;
};

#endif

