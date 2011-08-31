/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

//#if USING_VECTORS

#ifndef UVD_CPU_VECTOR_H
#define UVD_CPU_VECTOR_H

#include "uvd/util/types.h"
#include <string>

/*
Where code vectors to
*/
class UVDCPUVector
{
public:
	UVDCPUVector();
	
public:
	//An identifier
	std::string m_name;
	//Human readable description
	std::string m_description;
	//Address to vector to
	//FIXME: change this to UVDAddress
	uv_addr_t m_offset;
	//TODO: add something about conditions for vectoring?
};

#endif

//#endif
