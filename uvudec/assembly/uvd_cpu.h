/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#ifndef UVD_CPU_H
#define UVD_CPU_H

#include "uvd_cpu_vector.h"
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
	UVDOpcodeLookupTable *m_opcodeTable;
	//Where code may jump due to interrupts
	std::vector<UVDCPUVector *> m_vectors;
};

#endif
