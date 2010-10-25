/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_FLIRT_PATTERN_BFD_FUNCTION_H
#define UVD_FLIRT_PATTERN_BFD_FUNCTION_H

#include "uvdbfd/flirt/relocation.h"
#include <stdint.h>
#include "bfd.h"
#include "uvd/util/string_writer.h"

/*
UVDBFDFunction
*/
class UVDBFDPatModule;
class UVDBFDPatSection;
class UVDBFDPatFunction
{
public:	
	UVDBFDPatFunction();
	~UVDBFDPatFunction();
	
	//There is NO error handling on this
	//Relative to function start
	uint8_t readByte(uint32_t offset);

	/*
	Returns the first invalid index
	For various reasons, we may trim off various things:
	-Relocations at the end?
		Since they are wildcard, they are not part of the signature?
		Old code did this, but seems incorrect
		
	-Non-recursive descent reachable bytes
	-Compiler alignment fill bytes (typically 0xFF or 0x00.  I've also seen 0x7F and 0xCC)
	allowRelocations: only allow relocation free data
	*/
	uv_err_t getEffectiveEndPosition(uint32_t *endPos, uint32_t allowRelocations);

public:
	UVDBFDPatModule *m_module;
	asymbol *m_bfdAsymbol;
	//Offset within the section
	uint32_t m_offset;
	//Size of function in bytes
	uint32_t m_size;
};

#endif

