#pragma once

#include "uvd_error.h"
#include "uvd_relocation.h"
#include <string>

/*
A relocation value based on finding the index of the element in a table
*/
class UVDStringTableRelocatableElement : public UVDRelocatableElement
{
public:
	UVDStringTableRelocatableElement();
	//Find the index of s in the given string table as the value
	UVDStringTableRelocatableElement(UVDElfStringTableSectionHeaderEntry *stringTable, std::string &sTarget);
	virtual ~UVDStringTableRelocatableElement();
	
	virtual uv_err_t updateDynamicValue();
	
public:
	//The string table we will be searching
	UVDElfStringTableSectionHeaderEntry *m_stringTable;
	//The string we are looking for
	std::string m_sTarget;
};
