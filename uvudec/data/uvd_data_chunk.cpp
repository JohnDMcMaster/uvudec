/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
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

UVDDataChunk::UVDDataChunk()
{
	m_data = NULL;
	m_offset = 0;
	m_bufferSize = 0;
#ifdef UGLY_READ_HACK
	m_buffer = NULL;
#endif //UGLY_READ_HACK
}

uv_err_t UVDDataChunk::init(UVDData *data)
{
	uv_assert_ret(data);
	return UV_DEBUG(init(data, 0, data->size()));
}

uv_err_t UVDDataChunk::init(UVDData *data, unsigned int minAddr, unsigned int maxAddr)
{
	uv_err_t rc = UV_ERR_GENERAL;
#ifdef UGLY_READ_HACK
	unsigned int dataSize = maxAddr - minAddr;
	int readRc = -1;
#endif //UGLY_READ_HACK
	
	uv_assert_ret(maxAddr >= minAddr);
	
	m_offset = minAddr;
	m_bufferSize = maxAddr - minAddr;
	
#ifdef UGLY_READ_HACK
	printf_debug("Constructing block of size: 0x%.8X\n", dataSize);
	m_buffer = (char *)malloc(dataSize * sizeof(char));
	uv_assert(m_buffer);
	
	readRc = data->read(minAddr, m_buffer, dataSize);
	uv_assert(readRc >= 0);
	uv_assert(((unsigned int)readRc) == dataSize);
#else
	m_data = data;
#endif //UGLY_READ_HACK
	
	rc = UV_ERR_OK;

#ifdef UGLY_READ_HACK
error:
#endif //UGLY_READ_HACK
	return UV_DEBUG(rc);
}

UVDDataChunk::~UVDDataChunk()
{
#ifdef UGLY_READ_HACK
	free(m_buffer);
	m_buffer = NULL;
#endif
}

#ifdef UGLY_READ_HACK
uv_err_t UVDDataChunk::ensureRead()
{
	/*
	Returned the cached data if its availible
	Otherwise, read source and return data
	*/

	//Data already read?
	if( m_buffer )
	{
		return UV_ERR_OK;
	}
	
	uv_assert_ret(m_data);
	m_buffer = (char *)malloc(m_bufferSize * sizeof(char));
	uv_assert_ret(m_buffer);
	uv_assert_err_ret(m_data->read(m_offset, m_buffer, m_bufferSize));

	return UV_ERR_OK;
}

uv_err_t UVDDataChunk::getData(const char * &buffer) const
{
	uv_err_t rc = UV_ERR_GENERAL;

	uv_assert_err(ensureRead());
	
	uv_assert_ret(m_buffer);
	buffer = m_buffer;
	
	rc = UV_ERR_OK;
	
error:
	return UV_DEBUG(rc);
}

int UVDDataChunk::read(unsigned int offset, char *buffer, unsigned int bufferSize) const
{
	const char *pData = NULL;
	unsigned int dataSize = size();
	unsigned int readSize = dataSize;
	
printf("read, offset: %d, bufferSize: %d, data size: %d\n", offset, bufferSize, dataSize);

	//Direct buffer overflow?
	if( offset > dataSize )
	{
		return -1;
	}
	
	//Default to read into entire buffer
	readSize = bufferSize;
	//Read only partial if we'd run out of room
	if( offset + bufferSize > dataSize )
	{
		//Read only remaining data then
		readSize = dataSize - offset;
	}
	
	uv_assert_err_ret(getData(pData));

	uv_assert_ret(buffer);
	memcpy(buffer, pData + offset, readSize);

	return readSize;
}

bool UVDDataChunk::operator==(UVDDataChunk &other)
{
	//Size must be equal
	if( m_bufferSize != other.m_bufferSize )
	{
		return false;
	}

	//See if we don't have to read, are the positions equal?
	if( m_data == other.m_data && m_offset == other.m_offset )
	{
		return true;
	}
	
	//Okay, we have to play hardball
	//Brute force compare
	ensureRead();
	uv_assert_ret(m_buffer);
	other.ensureRead();
	uv_assert_ret(other.m_buffer);	
	return !memcmp(m_buffer, other.m_buffer, m_bufferSize);
}

#else //UGLY_READ_HACK

int UVDDataChunk::read(unsigned int offset, char *buffer, unsigned int bufferSize) const
{
	if( !m_data )
	{
		return -1;
	}
	//No caching, but works
	return m_data->read(m_offset + offset, buffer, bufferSize);
}

bool UVDDataChunk::operator==(UVDDataChunk &other)
{
	char *localBuffer = NULL;
	char *otherBuffer = NULL;
	
	//Size must be equal
	if( m_bufferSize != other.m_bufferSize )
	{
		return false;
	}

	//See if we don't have to read, are the positions equal?
	if( m_data == other.m_data && m_offset == other.m_offset )
	{
		return true;
	}
	
	//Okay, we have to play hardball
	//Brute force compare
	if( UV_FAILED(readData(&localBuffer)) )
	{
		return false;
	}
	if( !localBuffer )
	{
		return false;
	}
	if( UV_FAILED(other.readData(&otherBuffer)) )
	{
		return false;
	}
	if( !otherBuffer )
	{
		return false;
	}	
	bool ret = !memcmp(localBuffer, otherBuffer, m_bufferSize);
	free(localBuffer);
	free(otherBuffer);

	return ret;
}
#endif //else UGLY_READ_HACK
	
uint32_t UVDDataChunk::getMin()
{
	return m_offset;
}

uint32_t UVDDataChunk::getMax()
{
	return m_offset + m_bufferSize;
}

uint32_t UVDDataChunk::getOffset()
{
	return m_offset;
}

uint32_t UVDDataChunk::size() const
{
	return m_bufferSize;
}

uv_err_t UVDDataChunk::deepCopy(UVDData **out)
{
#ifdef UGLY_READ_HACK
#error unsupported
#endif

	UVDDataChunk *ret = NULL;

	ret = new UVDDataChunk();
	uv_assert_ret(ret);
	
	//By the very nature of this, we should just be able to do a shallow copy
	//..unless you did the UGLY_READ_HACK which should be fully removed anyway
	*ret = *this;

	uv_assert_ret(out);
	*out = ret;
	
	return UV_ERR_OK;
}
