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
UVDDataFile
*/

UVDDataFile::UVDDataFile()
{
	m_pFile = NULL;
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

uv_err_t UVDDataFile::getUVDDataFile(UVDDataFile** pDataFile, const std::string &file)
{
	return UV_DEBUG(getUVDDataFile((UVDData **)pDataFile, file));
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

std::string UVDDataFile::getSource() const
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
