/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_CORE_BLOCK_H
#define UVD_CORE_BLOCK_H

#include "uvd/assembly/address.h"
#include <vector>


/*
A block of code
	May contain nested blocks and/or actual code
Highest level block should be the entire program
	Next level is functions
	The lowest level blocks should be non-branching segments
*/
class UVDDataChunk;
class UVDAnalyzedCode;
class UVDAnalyzedBlock
{
public:
	UVDAnalyzedBlock();
	~UVDAnalyzedBlock();
	uv_err_t deinit();
	
	//Get the actual code representation of this block
	uv_err_t getDataChunk(UVDDataChunk **dataChunk);
	
	uv_err_t getMinAddress(uv_addr_t *address);
	uv_err_t getMaxAddress(uv_addr_t *address);
	uv_err_t getSize(size_t *size);
	
public:
	//Both of following can be set
	//m_code should always indicate the range and subblocks, if present, also indicated
	//If it contains code
	//We own this
	UVDAnalyzedCode *m_code;
	//If it contains blocks, usually should be more than one
	std::vector<UVDAnalyzedBlock *> m_blocks;
	UVDAddressSpace *m_addressSpace;
};

#endif

