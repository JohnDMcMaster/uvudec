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
#include "uv_debug.h"
#include "uv_util.h"
#include "uvd_data.h"
#include "uvd_types.h"

UVDData::UVDData()
{
}

UVDData::~UVDData()
{
	deinit();
}

void UVDData::deinit()
{
}

std::string UVDData::getSource()
{
	return "";
}

int UVDData::read(unsigned int offset, char *buffer, unsigned int bufferSize)
{
	unsigned int end = offset + bufferSize;
	unsigned int i = 0;

	//UV_ENTER();
	
	for( i = 0; i < bufferSize && offset < end; ++i, ++offset )
	{
		int res = read(offset);
		if( res < 0 )
		{
			return res;
		}
		else if( res == 0 )
		{
			break;
		}
		else
		{
			buffer[i] = res;
		}
	}
	return i;
}

int UVDData::read(unsigned int offset)
{
	char c;
	int ret;
	
	//UV_ENTER();

	ret = read(offset, &c, 1);
	if( ret < 0 )
	{
		return ret;
	}
	else if( ret == 0 )
	{
		return -1;
	}
	return (unsigned int)(unsigned char)c;
}

int UVDData::read(unsigned int offset, std::string &s, unsigned int readSize)
{
	char *buff = NULL;
	int rc = 0;
	
	buff = (char *)malloc(readSize);
	if( !buff )
	{
		return -1;
	}
	rc = read(offset, buff, readSize);
	s = buff;
	free(buff);
	buff = NULL;
	return rc;
}

unsigned int UVDData::size()
{
	int i = 0;

	//UV_ENTER();

	for( ;; )
	{
		if( read(i) < 0 )
		{
			return i;
		}
		++i;
	}
}

UVDDataFile::UVDDataFile()
{
}

uv_err_t UVDDataFile::init(const std::string &file)
{
	int ret = UV_ERR_GENERAL;
	m_sFile = file;
	m_pFile = fopen(file.c_str(), "rb");
	if( !m_pFile )
	{
		goto error;
	}
	
	ret = UV_ERR_OK;;
	
error:
	return ret;
}

uv_err_t UVDDataFile::getUVDDataFile(UVDData **pDataFile, const std::string &file)
{
	UVDDataFile *dataFile = NULL;
	uv_err_t rc = UV_ERR_GENERAL;

	if( !pDataFile )
	{
		return UV_ERR_GENERAL;
	}
	dataFile = new UVDDataFile();
	if( !dataFile )
	{
		return UV_ERR_GENERAL;
	}	
	rc = dataFile->init(file);
	if( UV_FAILED(rc) )
	{
		delete dataFile;
		goto error;
	}
	*pDataFile = dataFile;
	rc = UV_ERR_OK;

error:
	return rc;
}

UVDDataFile::~UVDDataFile()
{
	deinit();
}

void UVDDataFile::deinit()
{
	if( m_pFile )
	{
		fclose(m_pFile);
		m_pFile = NULL;
	}
	UVDData::deinit();
}

std::string UVDDataFile::getSource()
{
	return m_sFile;
}

unsigned int UVDDataFile::size()
{
	struct stat statStruct;

	//UV_ENTER();

	if( stat(m_sFile.c_str(), &statStruct) )
	{
		return 0;
	}
	return (unsigned int)statStruct.st_size;
}

int UVDDataFile::read(unsigned int offset, char *buffer, unsigned int bufferSize)
{
	int readRc = 0;
	//UV_ENTER();

	//printf_debug("Reading file %s, offset: %d, size: %d\n", m_sFile.c_str(), offset, bufferSize);
	//Start from beginning again...
	if( fseek(m_pFile, offset, SEEK_SET) )
	{
		return -1;
	}
	readRc = fread(buffer, sizeof(char), bufferSize, m_pFile);
	/*
	printf_debug("Read rc: %d\n", readRc);
	if( readRc >= 0 )
	{
		for( unsigned int i = 0; i < (unsigned int)readRc; ++i )
		{
			unsigned int readC = (unsigned char)buffer[i];
			printf_debug("read[%d]: 0x%.2X\n", i, readC);
		}
	}
	*/
	return readRc;
}

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

UVDDataMemory::UVDDataMemory(const char *buffer, unsigned int bufferSize)
{
	m_buffer = (char *)malloc(bufferSize);
	if( !m_buffer )
	{
		return;
	}
	
	m_bufferSize = bufferSize;
	memcpy(m_buffer, buffer, bufferSize);
}

UVDDataMemory::~UVDDataMemory()
{
	free(m_buffer);
	m_buffer = NULL;
}

std::string UVDDataMemory::getSource()
{
	char buffer[64];
	snprintf(buffer, 64, "0x%.8X:+0x%.8X", (unsigned int)m_buffer, (unsigned int)m_bufferSize);
	return std::string(buffer);
}

int UVDDataMemory::read(unsigned int offset, char *buffer, unsigned int bufferSize)
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

UVDDataChunk::UVDDataChunk()
{
	m_data = NULL;
	m_offset = 0;
	m_bufferSize = 0;
	m_buffer = NULL;
}

uv_err_t UVDDataChunk::init(UVDData *data)
{
	uv_assert_ret(data);
	return UV_DEBUG(init(data, 0, data->size()));
}

uv_err_t UVDDataChunk::init(UVDData *data, unsigned int minAddr, unsigned int maxAddr)
{
	uv_err_t rc = UV_ERR_GENERAL;
	unsigned int dataSize = maxAddr - minAddr;
	int readRc = -1;
	
	uv_assert_ret(maxAddr >= minAddr);
	
	m_offset = minAddr;
	m_bufferSize = maxAddr - minAddr;
	
	printf_debug("Constructing block of size: 0x%.8X\n", dataSize);
	m_buffer = (char *)malloc(dataSize * sizeof(char));
	uv_assert(m_buffer);
	
	readRc = data->read(minAddr, m_buffer, dataSize);
	uv_assert(readRc >= 0);
	uv_assert(((unsigned int)readRc) == dataSize);
	
	rc = UV_ERR_OK;

error:
	return UV_DEBUG(rc);
}

UVDDataChunk::~UVDDataChunk()
{
	free(m_buffer);
	m_buffer = NULL;
}

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

uv_err_t UVDDataChunk::getData(const char * &buffer)
{
	uv_err_t rc = UV_ERR_GENERAL;

	uv_assert_err(ensureRead());
	
	uv_assert_ret(m_buffer);
	buffer = m_buffer;
	
	rc = UV_ERR_OK;
	
error:
	return UV_DEBUG(rc);
}

uv_err_t UVDDataChunk::copyData(char *buffer)
{
	const char *pData = NULL;

	uv_assert_err_ret(getData(pData));

	uv_assert_ret(buffer);
	memcpy(buffer, pData, m_bufferSize);

	return UV_ERR_OK;
}

uv_err_t UVDDataChunk::copyData(char **bufferOut)
{
	const char *pData = NULL;
	char *buffer = NULL;
	
	uv_assert_err_ret(getData(pData));
	buffer = (char *)malloc(m_bufferSize * sizeof(char));
	uv_assert_ret(buffer);
	memcpy(buffer, pData, m_bufferSize);
	
	uv_assert_ret(bufferOut);
	*bufferOut = buffer;

	return UV_ERR_OK;
}
	
uv_err_t UVDDataChunk::saveToFile(const std::string &file)
{
	return UV_DEBUG(writeFile(file, m_buffer, m_bufferSize));
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

uint32_t UVDDataChunk::getSize()
{
	return m_bufferSize;
}
