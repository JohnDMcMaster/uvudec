/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#ifndef UVD_CPU_VECTOR_H
#define UVD_CPU_VECTOR_H

#include "uvd_types.h"
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
	uint32_t m_offset;
	//TODO: add something about conditions for vectoring?
};

#endif
