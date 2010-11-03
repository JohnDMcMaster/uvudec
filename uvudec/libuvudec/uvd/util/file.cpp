/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
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
#include "uvd/util/util.h"
#include "uvd/util/debug.h"
#include "uvd/util/error.h"
#include "uvd/core/uvd.h"
#include <linux/limits.h>
#include <sys/types.h>
#include <unistd.h>

uv_err_t UVDGetInstallDir(std::string &installDir)
{
/*
UVD_INSTALL_DIR won't be installed if we want a drag and drop deploy executable
*/
//Static compile
#ifdef UVD_INSTALL_DIR
	installDir = UVD_INSTALL_DIR;
#else
	std::string programName;

	//FIXME: this doesn't quite work
	//eg: from python since we aren't in the correct dir
	uv_assert_err_ret(getProgramName(programName));
	//Like /opt/uvudec/3.0.0/bin/uvudec, need to remove two dirs
	installDir = uv_dirname(uv_dirname(programName));
#endif
	return UV_ERR_OK;
}

uv_err_t getProgramName(std::string &programName)
{
	//read /proc/$PID/exe
	char buff[512];
	int curPid = 0;
	
	curPid = getpid();
	uv_assert_ret(curPid > 0);
	snprintf(buff, sizeof(buff), "/proc/%d/exe", curPid);
	uv_assert_err_ret(resolveSymbolicLink(buff, programName));

	return UV_ERR_OK;
}

uv_err_t getWorkingDir(std::string &out)
{
	return UV_DEBUG(getEnvironmentVariable("PWD", out));
}

uv_err_t UVDGetHomeDir(std::string &out)
{
	return UV_DEBUG(getEnvironmentVariable("HOME", out));
}

uv_err_t getCannonicalFileName(const std::string &filename, std::string &cannonicalFileName)
{
	std::string relativeDir;

	uv_assert_err_ret(getWorkingDir(relativeDir));
	uv_assert_err_ret(getCannonicalFileNameCore(filename, relativeDir, cannonicalFileName));
	return UV_ERR_OK;
}

uv_err_t getCannonicalFileNameCore(const std::string &filename, const std::string &relativeCannonicalDir, std::string &cannonicalFileName)
{
	if( filename[0] == '/' )
	{
		cannonicalFileName = filename;
	}
	else
	{
		std::string temp;
		
		temp = relativeCannonicalDir + "/" + filename;
		uv_assert_err_ret(collapsePath(temp, cannonicalFileName));
	}
	return UV_ERR_OK;
}

uv_err_t isSymbolicLink(const std::string &sFile, uint32_t *isLink)
{
	struct stat statStruct;

	uv_assert_ret(lstat(sFile.c_str(), &statStruct) == 0);	
	if( (S_IFLNK & statStruct.st_mode) == S_IFLNK )
	{
		*isLink = true;
	}
	else
	{
		*isLink = false;
	}
	return UV_ERR_OK;;
}

uv_err_t collapsePath(const std::string &relativePath, std::string &pathRet)
{
	//Originial path parts
	std::vector<std::string> parts = split(relativePath, '/', false);
	//Reassembled path parts, we must buffer so we can pop off as ..'s cancel out dirs
	std::vector<std::string> vecBuffer;
	for( std::string::size_type i = 0; i < parts.size(); ++i )
	{
		//Ignore current directory in canonical paths
		if( parts[i] == "." )
		{
			//Skip...
		}
		//Pop off dis if availible
		else if( parts[i] == ".." )
		{
			if( !vecBuffer.empty() )
			{
				vecBuffer.pop_back();
			}
		}
		//Some regular path
		else
		{
			vecBuffer.push_back(parts[i]);
		}
	}
	
	//Reconstruct the remaining path now
	pathRet = "";
	//split would have removed this
	if( relativePath[0] == '/' )
	{
		pathRet = "/";
	}
	for( std::vector<std::string>::size_type i = 0; i < vecBuffer.size(); ++i )
	{
		pathRet += vecBuffer[i];
		if( i < vecBuffer.size() - 1 )
		{
			pathRet += "/";
		} 
	}
	
	return UV_ERR_OK;
}

uv_err_t weaklyEnsurePathEndsWithExtension(const std::string &in, const std::string &extension, std::string &out)
{
	if( in.find(".") == std::string::npos )
	{
		uv_assert_err_ret(ensurePathEndsWithExtension(in, extension, out));
	}
	else
	{
		out = in;
	}
	return UV_ERR_OK;
}

uv_err_t ensurePathEndsWithExtension(const std::string &in, const std::string &extension, std::string &out)
{
	std::string dot;
	if( extension[0] != '.' )
	{
		dot = ".";
	}
	return UV_DEBUG(ensurePathEndsWith(in, dot + extension, out));
}

uv_err_t ensurePathEndsWith(const std::string &in, const std::string &ending, std::string &out)
{
	//FIXME: make this technically correct
	if( in.find(ending) == std::string::npos )
	{
		out = in + ending;
	}
	else
	{
		out = in;
	}
	return UV_ERR_OK;
}

uv_err_t resolveSymbolicLink(const std::string &fileIn, std::string &ret)
{
	//In case link is circular or similar
	uint32_t linkCount = 0;
	const uint32_t linkCountMax = 16;
	//We need this to resolve symbolic links
	std::string last = ret;
	
	//Track what our most resolved link to date is
	ret = fileIn;
		
	for( ;; )
	{
		uint32_t isLink = 0;
		char szBuff[256];
		uint32_t readlinkRc = 0;
		
		++linkCount;
		uv_assert_ret(linkCount <= linkCountMax);
		
		uv_assert_err_ret(isSymbolicLink(ret, &isLink));
		//No link -> done
		if( !isLink )
		{
			return UV_ERR_OK;
		}

		readlinkRc = readlink(ret.c_str(), szBuff, sizeof(szBuff));
		uv_assert_ret(readlinkRc >= 0);
		//readlink doesn't null terminate
		ret = std::string(szBuff, readlinkRc);
		
		//Make it canonical if not so already
		if( ret[0] != '/' )
		{
			std::string retSimplified;
			
			//Since we required canonical, get absolute dir from our link location
			ret = uv_dirname(last) + "/" + ret;
			uv_assert_err_ret(collapsePath(ret, retSimplified));
			//Done, should have a simplified relative path
			ret = retSimplified;
		}
		last = ret;
	}
}

std::string uv_basename(const std::string &file)
{
	std::string::size_type pos = file.rfind("/");
	if( pos == std::string::npos )
	{
		return file;
	}
	//We should be gauranteed the null terminator for safe bound
	return file.substr(pos + 1);
}

std::string uv_dirname(const std::string &file)
{
	std::string::size_type pos = file.rfind("/");
	if( pos == std::string::npos )
	{
		return "";
	}
	return file.substr(0, pos);
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
		//Doesn't exist: not a dir
		return UV_ERR_GENERAL;
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
		printf_error("File not found/opened: %s\n", file);
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
