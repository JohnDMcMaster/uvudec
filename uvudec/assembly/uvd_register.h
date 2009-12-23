/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#pragma once

#include "uvd_address.h"

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
	//UVDMemoryShared *m_mem; 
};

