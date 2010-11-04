/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

//I've read this is unreliable, but good enough for current needs
#include <endian.h>
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

std::string UVDData::getSource() const
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
	if( ((unsigned int)readVal) != bufferSize )
	{
		printf("Read bytes: %d, expected: %d\n", readVal, bufferSize);
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	return UV_ERR_OK;
}

uv_err_t UVDData::readData(unsigned int offset, std::string &s, unsigned int readSize) const
{
	int readValue = read(offset, s, readSize);
	uv_assert_ret(readValue >= 0);
	uv_assert_ret(((unsigned int)readValue) == readSize);
	return UV_ERR_OK;
}

uv_err_t UVDData::readData(uint32_t offset, char *c) const
{
	int val = read(offset);
	
	uv_assert_ret(val >= 0);
	uv_assert_ret(c);
	*c = val;
	
	return UV_ERR_OK;
}

uv_err_t UVDData::readData(uint32_t offset, uint8_t *c) const
{
	return UV_DEBUG(readData(offset, (char *)c));	
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

	ret = read(offset, &c, sizeof(char));
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

uv_err_t UVDData::writeData(uint32_t offset, const char *buffer, unsigned int bufferSize)
{
	return UV_DEBUG(UV_ERR_GENERAL);
}

uv_err_t UVDData::writeData(uint32_t offset, const UVDData *data)
{
	char *buffer = NULL;
	uv_assert_ret(data);
	
	uv_assert_err_ret(data->readData(&buffer));
	//Let all bounds checking occur here
	uv_assert_err_ret(writeData(offset, buffer, data->size()));
	free(buffer);

	return UV_ERR_OK;
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

uv_err_t UVDData::saveToFile(const std::string &file) const
{
	uint32_t dataSize = 0;
	char *buffer = NULL;

	uv_assert_err_ret(size(&dataSize));

	//printf("pre read\n");	
	uv_assert_err_ret(readData(0, &buffer, dataSize));	
	//printf("post read\n");

	//::hexdump(buffer, dataSize);

	uv_assert_err_ret(writeFile(file, buffer, dataSize));
	free(buffer);
	return UV_ERR_OK;
}

uv_err_t UVDData::deepCopy(UVDData **out)
{
	//Base class should prob be pure virtual anyway and seems like an error to try this on it
	return UV_DEBUG(UV_ERR_GENERAL);
}

void UVDData::hexdump()
{
	uint32_t dataSize = 0;
	char *buffer = NULL;
	
	if( UV_FAILED(size(&dataSize)) )
	{
		printf_debug("Could not read data size\n"); 
		return;
	}
	
	if( UV_FAILED(readData(&buffer) ) )
	{
		printf_debug("Could not read data\n"); 
		return;
	}
	
	if( !buffer )
	{
		printf_debug("No data");
		return;
	}
	
	//::hexdump(buffer, dataSize);
	free(buffer);
}

uv_err_t UVDData::readU8(uint32_t offset, uint8_t *out) const
{
	return UV_DEBUG(readData(offset, (uint8_t *)out));	
}

uv_err_t UVDData::read8(uint32_t offset, int8_t *out) const
{
	return UV_DEBUG(readData(offset, (uint8_t *)out));	
}

uv_err_t UVDData::readU16(uint32_t offset, uint16_t *out, uint32_t endianness) const
{
	uint16_t data;
	uv_assert_err_ret(readData(offset, (char *)&data, sizeof(uint16_t)));
	switch( endianness )
	{
	case UVD_DATA_ENDIAN_BIG:
		data = be16toh(data);
		break;
	case UVD_DATA_ENDIAN_LITTLE:
		data = le16toh(data);
		break;
	/*
	Hmm I guess they don't define macros for this for some reason
	case UVD_DATA_ENDIAN_PDP:
		data = be16toh(data);
		break;
	*/
	default:
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	uv_assert_ret(out);
	*out = data;
	return UV_ERR_OK;
}

uv_err_t UVDData::read16(uint32_t offset, int16_t *out, uint32_t endianness) const
{
	return UV_DEBUG(readU16(offset, (uint16_t *)out, endianness));	
}

uv_err_t UVDData::readU32(uint32_t offset, uint32_t *out, uint32_t endianness) const
{
	uint32_t data;
	uv_assert_err_ret(readData(offset, (char *)&data, sizeof(uint32_t)));
	switch( endianness )
	{
	case UVD_DATA_ENDIAN_BIG:
		data = be32toh(data);
		break;
	case UVD_DATA_ENDIAN_LITTLE:
		data = le32toh(data);
		break;
	/*
	Hmm I guess they don't define macros for this for some reason
	case UVD_DATA_ENDIAN_PDP:
		data = be16toh(data);
		break;
	*/
	default:
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	uv_assert_ret(out);
	*out = data;
	return UV_ERR_OK;
}

uv_err_t UVDData::read32(uint32_t offset, int32_t *out, uint32_t endianness) const
{
	return UV_DEBUG(readU32(offset, (uint32_t *)out, endianness));	
}

uv_err_t UVDData::writeU8(uint32_t offset, uint8_t in)
{
	return UV_DEBUG(writeData(offset, (const char *)&in, sizeof(in)));	
}

uv_err_t UVDData::write8(uint32_t offset, int8_t in)
{
	return UV_DEBUG(writeU8(offset, in));	
}

uv_err_t UVDData::writeU16(uint32_t offset, uint16_t in, uint32_t endianness)
{
	uint16_t data = 0;
	
	switch( endianness )
	{
	case UVD_DATA_ENDIAN_BIG:
		data = htobe16(in);
		break;
	case UVD_DATA_ENDIAN_LITTLE:
		data = htole16(in);
		break;
	default:
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	uv_assert_err_ret(writeData(offset, (const char *)&data, sizeof(data)));	
	return UV_ERR_OK;
}

uv_err_t UVDData::write16(uint32_t offset, int16_t in, uint32_t endianness)
{
	return UV_DEBUG(writeU16(offset, (uint16_t)in, endianness));	
}

uv_err_t UVDData::writeU32(uint32_t offset, uint32_t in, uint32_t endianness)
{
	uint32_t data = 0;
	
	switch( endianness )
	{
	case UVD_DATA_ENDIAN_BIG:
		data = htobe32(in);
		break;
	case UVD_DATA_ENDIAN_LITTLE:
		data = htole32(in);
		break;
	default:
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	uv_assert_err_ret(writeData(offset, (const char *)&data, sizeof(data)));
	return UV_ERR_OK;
}

uv_err_t UVDData::write32(uint32_t offset, int32_t in, uint32_t endianness)
{
	return UV_DEBUG(writeU32(offset, (uint32_t)in, endianness));	
}

