/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#if USING_VECTORS
#ifndef UVD_CPU_H
#define UVD_CPU_H

#include "uvd/assembly/cpu_vector.h"
#include "uvd_opcode.h"
#include <vector>

/*
Where code vectors to
*/
class UVDCPU
{
public:
	UVDCPU();
	uv_err_t init();
	
public:
	//For opcodes
	//If unused, set to NULL
	//UVDOpcodeLookupTable *m_opcodeTable;
	//Where code may jump due to interrupts
	std::vector<UVDCPUVector *> m_vectors;
};

#endif

#endif
