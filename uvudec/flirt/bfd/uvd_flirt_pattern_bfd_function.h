/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_FLIRT_PATTERN_BFD_FUNCTION_H
#define UVD_FLIRT_PATTERN_BFD_FUNCTION_H

#include "uvd_flirt_pattern_bfd_relocation.h"
#include <stdint.h>
#include "bfd.h"
#include "uvd_string_writter.h"

/*
UVDBFDFunction
*/
class UVDBFDPatSection;
class UVDBFDPatFunction
{
public:
	UVDBFDPatFunction();
	~UVDBFDPatFunction();
	
	uv_err_t print();
	//There is NO error handling on this
	uint8_t readByte(uint32_t offset);

	/*
	For various reasons, we may trim off various things:
	-Relocations at the end?
		Since they are wildcard, they are not part of the signature?
		Old code did this, but seems incorrect
		
	-Non-recursive descent reachable bytes
	-Compiler alignment fill bytes (typically 0xFF or 0x00.  I've also seen 0x7F and 0xCC)
	*/
	uv_err_t getEffectiveEndPosition(uint32_t *endPos);
	void printRelocationByte();
	uv_err_t printLeadingBytes();
	uv_err_t printRelocations();
	uv_err_t printCRC();
	//start, end relative to function start
	//end is not inclusive
	uv_err_t printPatternBytes(uint32_t start, uint32_t end);
	uv_err_t shouldPrintFunction();

protected:
	UVDStringWritter *getStringWritter();

public:
	asymbol *m_bfdAsymbol;
	UVDBFDPatRelocations m_relocations;
	//Parent section, do not own
	UVDBFDPatSection *m_section;
	//Offset within the section
	uint32_t m_offset;
	//Size of function in bytes
	uint32_t m_size;
};

/*
UVDBFDFunctions
*/
class UVDBFDPatFunctions
{
public:
	UVDBFDPatFunctions();
	~UVDBFDPatFunctions();
	
	uv_err_t add(UVDBFDPatFunction *function);
	
public:
	//We own these
	std::vector<UVDBFDPatFunction *> m_functions;
};

#endif

