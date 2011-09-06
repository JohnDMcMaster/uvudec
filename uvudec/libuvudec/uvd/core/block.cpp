/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/assembly/cpu_vector.h"
#include "uvd/assembly/function.h"
#include "uvd/core/uvd.h"
#include "uvd/core/analyzer.h"
#include "uvd/core/block.h"
#include "uvd/core/event.h"
#include "uvd/core/runtime.h"
#include "uvd/event/engine.h"
#include "uvd/string/engine.h"
#include "uvd/util/benchmark.h"
#include <algorithm>
#include <stdio.h>

UVDAnalyzedBlock::UVDAnalyzedBlock()
{
	m_code = NULL;
	m_addressSpace = NULL;
}

UVDAnalyzedBlock::~UVDAnalyzedBlock()
{
	deinit();
}

uv_err_t UVDAnalyzedBlock::deinit()
{
	delete m_code;
	m_code = NULL;
	
	for( std::vector<UVDAnalyzedBlock *>::iterator iter = m_blocks.begin(); iter != m_blocks.end(); ++iter )
	{
		delete *iter;
	}

	return UV_ERR_OK;
}

uv_err_t UVDAnalyzedBlock::getDataChunk(UVDDataChunk **dataChunkIn)
{
	UVDDataChunk *dataChunk = NULL;
	
	uv_assert_ret(m_code);
	dataChunk = m_code->m_dataChunk;
	uv_assert_ret(dataChunk);
	
	uv_assert_ret(dataChunkIn);
	*dataChunkIn = dataChunk;

	return UV_ERR_OK;
}

uv_err_t UVDAnalyzedBlock::getMinAddress(uv_addr_t *out)
{
	UVDDataChunk *dataChunk = NULL;
	
	uv_assert_err_ret(getDataChunk(&dataChunk));
	uv_assert_ret(dataChunk);
	
	uv_assert_ret(out);
	*out = dataChunk->m_offset;

	return UV_ERR_OK;
}

uv_err_t UVDAnalyzedBlock::getMaxAddress(uv_addr_t *out)
{
	UVDDataChunk *dataChunk = NULL;
	
	uv_assert_err_ret(getDataChunk(&dataChunk));
	uv_assert_ret(dataChunk);
	
	uv_assert_ret(out);
	*out = dataChunk->m_offset + dataChunk->m_bufferSize;

	return UV_ERR_OK;
}

uv_err_t UVDAnalyzedBlock::getSize(size_t *out)
{
	UVDDataChunk *dataChunk = NULL;
	
	uv_assert_err_ret(getDataChunk(&dataChunk));
	uv_assert_ret(dataChunk);
	
	uv_assert_ret(out);
	*out = dataChunk->m_bufferSize;

	return UV_ERR_OK;
}


