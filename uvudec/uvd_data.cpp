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

uv_err_t UVDData::readData(char **buffer) const
{
	unsigned int dataSize = size();
	return UV_DEBUG(readData(0, buffer, dataSize));
}

uv_err_t UVDData::readData(unsigned int offset, char **buffer) const
{
	unsigned int dataSize = size();
	return UV_DEBUG(readData(offset, buffer, dataSize));
}

uv_err_t UVDData::readData(unsigned int offset, char **bufferOut, unsigned int bufferSize) const
{
	char *buffer = NULL;
	
	buffer = (char *)malloc(bufferSize);
	uv_assert_ret(buffer);
	
	uv_assert_err_ret(readData(offset, buffer, bufferSize));
	
	uv_assert_ret(bufferOut);
	*bufferOut = buffer;
	return UV_ERR_OK;
}

uv_err_t UVDData::readData(unsigned int offset, char *buffer, unsigned int bufferSize) const
{
	int readVal = read(offset, buffer, bufferSize);
	uv_assert_ret(readVal >= 0);
	uv_assert_ret(((unsigned int)readVal) == bufferSize);
	return UV_ERR_OK;
}

uv_err_t UVDData::readData(unsigned int offset, std::string &s, unsigned int readSize) const
{
	int readValue = read(offset, s, readSize);
	uv_assert_ret(readValue >= 0);
	uv_assert_ret(((unsigned int)readValue) == readSize);
	return UV_ERR_OK;
}

int UVDData::read(unsigned int offset, char *buffer, unsigned int bufferSize) const
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

int UVDData::read(unsigned int offset) const
{
	char c = 0;
	int ret = 0;
	
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

int UVDData::read(unsigned int offset, std::string &s, unsigned int readSize) const
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

uv_err_t UVDData::writeData(unsigned int offset, const char *buffer, unsigned int bufferSize)
{
	return UV_DEBUG(UV_ERR_GENERAL);
}

uv_err_t UVDData::size(uint32_t *sizeOut) const
{
	uv_assert_ret(sizeOut);
	*sizeOut = size();
	return UV_ERR_OK;
}

/*
uint32_t UVDData::size() const
{
printf("Size read\n");
fflush(stdout);
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
*/

static uv_err_t getDataSize(const std::vector<UVDData *> &dataVector, unsigned int *dataSizeOut)
{
	unsigned int dataSize = 0;
	
	for( std::vector<UVDData *>::const_iterator iter = dataVector.begin(); iter != dataVector.end(); ++iter )
	{
		UVDData *data = *iter;

		uv_assert_ret(data);
		dataSize += data->size();
	}
	uv_assert_ret(dataSizeOut);
	*dataSizeOut = dataSize;

	return UV_ERR_OK;
}

uv_err_t UVDData::concatenate(const std::vector<UVDData *> &dataVector, UVDData **dataOut)
{
	unsigned int expectedSize = 0;
	unsigned int writePos = 0;
	UVDDataMemory *fullData = NULL;
	
	uv_assert_err_ret(getDataSize(dataVector, &expectedSize));
	fullData = new UVDDataMemory(expectedSize);
	uv_assert_ret(fullData);
	
	for( std::vector<UVDData *>::const_iterator iter = dataVector.begin(); iter != dataVector.end(); ++iter )
	{
		UVDData *data = *iter;
		char *bufferTemp = NULL;

		uv_assert_ret(data);
		//Get a copy of all data
		uv_assert_err_ret(data->readData(0, &bufferTemp, data->size()));	
		//And copy it into the new data element
		uv_assert_err_ret(fullData->writeData(writePos, bufferTemp, data->size()));

		free(bufferTemp);
		bufferTemp = NULL;
		//Update our offset		
		writePos += data->size();
	}
	
	uv_assert_ret(dataOut);
	*dataOut = fullData;

	return UV_ERR_OK;
}

/*
UVDDataFile
*/

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

uint32_t UVDDataFile::size() const
{
	struct stat statStruct;

	//UV_ENTER();

	if( stat(m_sFile.c_str(), &statStruct) )
	{
		return 0;
	}
	return (uint32_t)statStruct.st_size;
}

int UVDDataFile::read(unsigned int offset, char *buffer, unsigned int bufferSize) const
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
	uv_assert_ret(m_bufferSize >= bufferSize + offset);
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

	return UV_ERR_OK;
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
	uv_assert_ret(m_data);
	//No caching, but works
	uv_assert_err_ret(m_data->read(m_offset + offset, buffer, bufferSize));
	return UV_ERR_OK;
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
	
uv_err_t UVDData::saveToFile(const std::string &file) const
{
	unsigned int dataSize = size();
	char *buffer = NULL;
	
	uv_assert_err_ret(readData(0, &buffer, dataSize));	

	uv_assert_err_ret(writeFile(file, buffer, dataSize));
	free(buffer);
	return UV_ERR_OK;
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

uint32_t UVDDataChunk::size() const
{
	return m_bufferSize;
}
