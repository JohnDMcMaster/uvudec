/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#pragma once

#include "uvd/assembly/address.h"

class UVDRegisterShared
//struct uv_disasm_reg_shared_t
{
public:
	UVDRegisterShared();

public:
	std::string m_name;
	std::string m_desc;
	/* In bits */
	uint32_t m_size;
	
	
	/* If this is a memory mapped register, describes where and what type */
	//uint32_t m_mem_addr;
	//UVDAddressSpace *m_mem; 
};

