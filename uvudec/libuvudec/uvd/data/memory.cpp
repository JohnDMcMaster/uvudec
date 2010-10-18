/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "uvd/util/debug.h"
#include "uvd/util/util.h"
#include "uvd/data/data.h"
#include "uvd/util/types.h"

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
	//memset(m_buffer, 0xCD, bufferSize);
	memset(m_buffer, 0xDC, bufferSize);
#endif //NDEBUG
	m_freeAtDestruction = true;
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
	int32_t delta = 0;
	
	//No change?
	delta = bufferSize - m_bufferSize;
	if( delta == 0 )
	{
		return UV_ERR_OK;
	}
	/*
	free(m_buffer);
	//In case we can't realloc
	m_bufferSize = 0;
	m_buffer = (char *)malloc(bufferSize);
	*/
	m_buffer = (char *)::realloc(m_buffer, bufferSize);
	uv_assert_ret(m_buffer);
	if( delta > 0 )
	{
		memset(m_buffer + m_bufferSize, 0, delta);
	}
	m_bufferSize = bufferSize;
	return UV_ERR_OK;
}

uv_err_t UVDDataMemory::writeData(unsigned int offset, const char *buffer, unsigned int bufferSize)
{
	uint32_t thisSize = size();
	
	//Do we have enough space?
	if( thisSize < bufferSize + offset )
	{
		printf_error("buffer availible (thisSize): 0x%08X, needed (bufferSize(0x%08X) + offset(0x%08X)): 0x%08X\n",
				thisSize, bufferSize, offset, bufferSize + offset);
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
	snprintf(buffer, 64, "0x%08X:+0x%08X", (unsigned int)m_buffer, (unsigned int)m_bufferSize);
	return std::string(buffer);
}

int UVDDataMemory::read(unsigned int offset, char *buffer, unsigned int bufferSize) const
{
	uint32_t thisBufferSize = size();
	//printf("read, offset: 0x%08X, dest buffer: 0x%08X, dest buffer size: 0x%08X, src buffer: 0x%08X, src buffer size: 0x%08X\n", offset, buffer, bufferSize, m_buffer, thisBufferSize);
	if( offset > thisBufferSize )
	{
		return -1;
	}
	
	unsigned int leftToRead = thisBufferSize - offset;
	if( leftToRead > bufferSize )
	{
		bufferSize = leftToRead;
	}
	
	memcpy(buffer, m_buffer, bufferSize);
	return bufferSize;
}

uv_err_t UVDDataMemory::deepCopy(UVDData **out)
{
	//FIXME: this won't work as well for buffered version
	UVDDataMemory *ret = NULL;
	uint32_t thisSize = size();
	
	ret = new UVDDataMemory();
	uv_assert_ret(ret);

	ret->m_buffer = (char *)malloc(thisSize);
	uv_assert_ret(ret->m_buffer);
	uv_assert_ret(m_buffer);
	memcpy(ret->m_buffer, m_buffer, thisSize);
	ret->m_bufferSize = thisSize;
	
	//We allocated this, must free it
	ret->m_freeAtDestruction = true;

	uv_assert_ret(out);
	*out = ret;
	
	return UV_ERR_OK;
}

/*
UVDBufferedDataMemory
*/

UVDBufferedDataMemory::UVDBufferedDataMemory(uint32_t bufferSize)
{
	m_virtualSize = bufferSize;
	m_growScalar = 2;
	m_growConstant = 1;
}

UVDBufferedDataMemory::UVDBufferedDataMemory(const char *buffer, uint32_t bufferSize)
{
	m_virtualSize = bufferSize;
	m_growScalar = 2;
	m_growConstant = 1;
}

UVDBufferedDataMemory::~UVDBufferedDataMemory()
{
}

uv_err_t UVDBufferedDataMemory::operator+=(const UVDData *other)
{
	//Or should we just return?
	uv_assert_ret(other);
	
	if( m_bufferSize + other->size() > m_virtualSize )
	{
		uv_assert_err_ret(realloc((m_bufferSize + other->size() > m_virtualSize) * m_growScalar + m_growConstant));
	}
	uv_assert_err_ret(UVDData::writeData(m_virtualSize, other));
	m_virtualSize += other->size();

	return UV_ERR_OK;
}

uv_err_t UVDBufferedDataMemory::operator+=(const std::string &other)
{
	if( m_bufferSize + other.size() > m_virtualSize )
	{
		uv_assert_err_ret(realloc((m_bufferSize + other.size()) * m_growScalar + m_growConstant));
	}
	uv_assert_err_ret(writeData(m_virtualSize, other.c_str(), other.size()));
	m_virtualSize += other.size();
	uv_assert_ret(m_virtualSize <= m_bufferSize);

	return UV_ERR_OK;
}

uv_err_t UVDBufferedDataMemory::append(const char *buffer, uint32_t bufferLength)
{
	uint32_t needed = 0;
	uint32_t originalSize = m_virtualSize;

	uv_assert_ret(m_bufferSize >= m_virtualSize);
	//m_bufferSize < bufferSize + offset
	//printf("needed: 0x%08X, m_bufferSize: 0x%08X, bufferLength: 0x%08X\n", needed, m_bufferSize, bufferLength);
	needed = m_virtualSize + bufferLength;
	if( needed > m_bufferSize )
	{
		uint32_t reallocSize = needed * m_growScalar + m_growConstant;
		uv_assert_err_ret(realloc(reallocSize));
	}
	uv_assert_ret(m_buffer);
	m_virtualSize += bufferLength;
	uv_assert_err_ret(writeData(originalSize, buffer, bufferLength));
	uv_assert_ret(m_virtualSize <= m_bufferSize);

	return UV_ERR_OK;
}

uv_err_t UVDBufferedDataMemory::appendByte(uint8_t in)
{
	return UV_DEBUG(append((char *)&in, sizeof(in)));
}

uint32_t UVDBufferedDataMemory::size() const
{
	return m_virtualSize;
}

uv_err_t UVDBufferedDataMemory::compact()
{
	uv_assert_ret(m_virtualSize <= m_bufferSize);
	if( m_virtualSize == m_bufferSize )
	{
		return UV_ERR_OK;
	}
	
	//Shrink it
	uv_assert_err_ret(realloc(m_virtualSize));

	return UV_ERR_OK;
}

