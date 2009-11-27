/*
Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under the terms of the BSD license.  See LICENSE for details.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "uvd_debug.h"
#include "uvd_util.h"
#include "uvd_data.h"
#include "uvd_types.h"

/*
UVDDataMemory
*/

UVDDataMemory::UVDDataMemory(unsigned int bufferSize)
{
	m_bufferSize = bufferSize;
	m_buffer = (char *)malloc(m_bufferSize);
	if( !m_buffer )
	{
		return;
	}

#ifndef NDEBUG
	//Poison it
	memset(m_buffer, 0xCD, bufferSize);
#endif //NDEBUG
}

UVDDataMemory::UVDDataMemory()
{
	m_bufferSize = 0;
	m_buffer = NULL;
	m_freeAtDestruction = true;
}

UVDDataMemory::UVDDataMemory(const char *buffer, unsigned int bufferSize)
{
	m_buffer = (char *)malloc(bufferSize);
	if( !m_buffer )
	{
		return;
	}
	
	m_bufferSize = bufferSize;
	memcpy(m_buffer, buffer, bufferSize);
	m_freeAtDestruction = true;
}

uv_err_t UVDDataMemory::getUVDDataMemoryByTransfer(UVDDataMemory **dataIn,
		char *buffer, unsigned int bufferSize,
		int freeAtDestruction)
{
	UVDDataMemory *data = NULL;

	//Allocate the raw object
	data = new UVDDataMemory();
	uv_assert_ret(data);
	//And copy the data in
	data->m_buffer = buffer;
	data->m_bufferSize = bufferSize;
	data->m_freeAtDestruction = freeAtDestruction;	
	
	uv_assert_ret(dataIn);
	*dataIn = data;

	return UV_ERR_OK;
}

uv_err_t UVDDataMemory::getUVDDataMemoryByCopy(const UVDData *dataIn, UVDData **dataOut)
{
	char *dataBuffer = NULL;
	
	//Get a copy of the data
	uv_assert_ret(dataIn);
	uv_assert_err_ret(dataIn->readData(&dataBuffer));
	
	//Create and return our ret object
	uv_assert_err_ret(getUVDDataMemoryByTransfer((UVDDataMemory **)dataOut, dataBuffer, dataIn->size()));	
	
	return UV_ERR_OK;
}

UVDDataMemory::~UVDDataMemory()
{
	if( m_freeAtDestruction )
	{
		free(m_buffer);
	}
	m_buffer = NULL;
}

uint32_t UVDDataMemory::size() const
{
	return m_bufferSize;
}

uv_err_t UVDDataMemory::realloc(unsigned int bufferSize)
{
	//No change?
	if( bufferSize == m_bufferSize )
	{
		return UV_ERR_OK;
	}
	free(m_buffer);
	//In case we can't realloc
	m_bufferSize = 0;
	m_buffer = (char *)malloc(bufferSize);
	uv_assert_ret(m_buffer);
	m_bufferSize = bufferSize;
	return UV_ERR_OK;
}

uv_err_t UVDDataMemory::writeData(unsigned int offset, const char *buffer, unsigned int bufferSize)
{
	//Do we have enough space?
	if( m_bufferSize < bufferSize + offset )
	{
		printf_error("buffer availible (m_bufferSize): 0x%.8X, needed (bufferSize + offset): 0x%.8X\n",
				m_bufferSize, bufferSize + offset);
		return UV_DEBUG(UV_ERR_GENERAL);		
	}
	
	uv_assert_ret(buffer);
	uv_assert_ret(m_buffer);
	//Do the copy
	memcpy(m_buffer + offset, buffer, bufferSize);

	return UV_ERR_OK;
}

std::string UVDDataMemory::getSource()
{
	char buffer[64];
	snprintf(buffer, 64, "0x%.8X:+0x%.8X", (unsigned int)m_buffer, (unsigned int)m_bufferSize);
	return std::string(buffer);
}

int UVDDataMemory::read(unsigned int offset, char *buffer, unsigned int bufferSize) const
{
	if( offset > m_bufferSize )
	{
		return -1;
	}
	
	unsigned int leftToRead = m_bufferSize - offset;
	if( leftToRead > bufferSize )
	{
		bufferSize = leftToRead;
	}
	
	memcpy(buffer, m_buffer, bufferSize);
	return bufferSize;
}
