/*
Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include "uvd_util.h"
#include "uvd_debug.h"
#include "uvd_error.h"
#include "uvd.h"
#include <linux/limits.h>

std::string uv_basename(const std::string &file)
{
	std::string::size_type pos = file.rfind("/");
	if( pos == std::string::npos )
	{
		return "";
	}
	return file.substr(pos);
}

std::string uv_dirname(const std::string &file)
{
	std::string::size_type pos = file.rfind("/");
	if( pos == std::string::npos )
	{
		return file;
	}
	return file.substr(0, pos - 1);
}

uv_err_t isRegularFile(const std::string &file)
{
	struct stat buff;
	
	if( stat(file.c_str(), &buff) )
	{
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	
	if( buff.st_mode & S_IFREG )
	{
		return UV_ERR_OK;
	}
	else
	{
		return UV_ERR_GENERAL;
	}
}

uv_err_t isDir(const std::string &file)
{
	struct stat buff;
	
	if( stat(file.c_str(), &buff) )
	{
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	
	if( buff.st_mode & S_IFDIR )
	{
		return UV_ERR_OK;
	}
	else
	{
		return UV_ERR_GENERAL;
	}
}

uv_err_t createDir(const std::string &file, bool bestEffort)
{
	//if( !bestEffort )
	{
		uv_assert_ret(!mkdir(file.c_str(), 0775));
	}
	return UV_ERR_OK;
}

uv_err_t deleteFile(std::string &sFile)
{
	if( unlink(sFile.c_str()) )
	{
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	return UV_ERR_OK;
}

uv_err_t readFile(const std::string &sFile, std::string &sRet)
{
	uv_err_t rc = UV_ERR_GENERAL;
	char *sTemp = NULL;
		
	rc = read_filea(sFile.c_str(), &sTemp);
	if( sTemp )
	{
		sRet = sTemp;
		free(sTemp);
	}
	return UV_DEBUG(rc);
}

uv_err_t writeFile(const std::string &sFile, const std::string &sIn)
{
	return UV_DEBUG(writeFile(sFile, sIn.c_str(), sIn.size()));
}

uv_err_t writeFile(const std::string &sFile, const char *buff, size_t buffsz)
{
	uv_err_t rc = UV_ERR_GENERAL;
	FILE *pFile = NULL;
	
	pFile = fopen(sFile.c_str(), "w");
	if( !pFile )
	{
		UV_DEBUG(rc);
		printf("Could not open file: %s\n", sFile.c_str());
		goto error;
	}

	if( fwrite(buff, sizeof(char), buffsz, pFile) != buffsz )
	{
		printf_debug("Bad write\n");
		UV_DEBUG(rc);
		goto error;
	}

	rc = UV_ERR_OK;

error:
	if( pFile )
	{
		fclose(pFile);
		pFile = NULL;
	}
	return UV_DEBUG(rc);
}

uv_err_t read_file(const char *file, uint8_t **ret, unsigned int *size)
{
	uv_err_t rc = UV_ERR_GENERAL;
	uint8_t *dat = NULL;	
	size_t dat_sz = 0;
	FILE *p_file = NULL;
	struct stat size_stat;
	int read = 0;
	unsigned int pos = 0;

	UV_ENTER();

	assert(file);
	if( ret == NULL && size == NULL )
	{
		UV_ERR(rc);
		goto error;
	}

	p_file = fopen(file, "r");
	if( !p_file )
	{
		//printf_debug("not opened: %s\n", strerror(errno));
		printf_debug("File not found/opened: %s\n", file);
		UV_ERR(rc);
		goto error;
	}

	if( stat(file, &size_stat) )
	{
		//printf("ERROR: couldn't stat\n");
		UV_ERR(rc);
		goto error;
	}
	dat_sz = size_stat.st_size;

	if( !dat_sz )
	{
		//printf("Warning: file is empty\n");
		if( ret )
		{
			*ret = NULL;
		}
		if( size )
		{
			*size = 0;
		}
		rc = UV_ERR_OK;
		goto error;
	}
	//printf("Size: 0x%x (%d)\n", dat_sz, dat_sz);
	if( size )
	{
		dat = (uint8_t *)malloc(dat_sz);
	}
	else
	{
		dat =  (uint8_t *)malloc(dat_sz + 1);
	}
	if( !dat )
	{
		//printf("ERROR: out of memory\n");
		rc = UV_ERR_OUTMEM;
		UV_ERR(rc);
		goto error;
	}
	if( !size )
	{
		dat[dat_sz] = 0;
	}
	
	/*
	Read file
	This is raw dat, NOT null terminated string
	Consider buffering this for faster reading
	*/
	while( (read = fgetc(p_file)) != EOF && pos < dat_sz )
	{
		dat[pos] = (uint8_t)read;
		++pos;
	}
	
	//File got truncated?
	if( pos != dat_sz )
	{
		UV_ERR(rc);
		printf("pos: %d, dat_sz: %d\n", pos, dat_sz);
		goto error;
	}
	if( size )
	{
		*size = dat_sz;
	}
	if( ret )
	{
		*ret = dat;
	}
	
	rc = UV_ERR_OK;
	
error:
	if( p_file )
	{
		fclose(p_file);
		p_file = NULL;
	}
	return UV_DEBUG(rc);
}

uv_err_t read_filea(const char *file, char **ret)
{
	return read_file(file, (uint8_t **)ret, NULL);
}
