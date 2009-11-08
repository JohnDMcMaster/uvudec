/*
Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

/*
Utilities based on code from UVNet, which was one of my earliest projects
	With the tracks I just got though, UVNet may be back online soon...
These needed to be reworked for speed at some point
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

/* Get the name and args as a string */
uv_err_t uvd_parse_func(const char *text, char **name, char **content);

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

uv_err_t parseFunc(const std::string &text, std::string &name, std::string &content)
{
	char *nameTemp = NULL;
	char *contentTemp = NULL;

	uv_assert_err_ret(uvd_parse_func(text.c_str(), &nameTemp, &contentTemp));

	uv_assert_ret(nameTemp);
	name = nameTemp;
	free(nameTemp);
	nameTemp = NULL;

	uv_assert_ret(contentTemp);
	content = contentTemp;
	free(contentTemp);
	contentTemp = NULL;
	
	return UV_ERR_OK;
}

std::vector<std::string> split(const std::string &s, char delim, bool ret_blanks)
{
	char **coreRet = NULL;
	char **cur = NULL;
	std::vector<std::string> ret;
	
	coreRet = uv_split_core(s.c_str(), delim, NULL, ret_blanks);
	if( !coreRet )
	{
		return ret;
	}

	for( cur = coreRet; *cur; ++cur )
	{
		ret.push_back(*cur);
		free(*cur);
	}
	free(coreRet);

	return ret;
}

std::string parseSubstring(const std::string &in, const std::string &seek, const std::string &start, const std::string &end, std::string::size_type *pos)
{
	std::string::size_type posStart = 0;
	std::string::size_type posEnd = std::string::npos;
	if (pos)
	{
		posStart = *pos;
	}
	if( !seek.empty() )
	{
		posStart = in.find(seek);
		if( posStart == std::string::npos )
		{
			return "";
		}
		posStart += seek.size();
	}
	if( !start.empty() )
	{
		posStart = in.find(start, posStart);
		if( posStart == std::string::npos )
		{
			return "";
		}
		posStart = posStart + start.size();
	}
	if( !end.empty() )
	{
		posEnd = in.find(end, posStart);
	}
	if( posEnd == std::string::npos )
	{
		return in.substr(posStart);
	}
	else
	{
		return in.substr(posStart, posEnd - posStart);
	}
}

uv_err_t getTempFile(std::string &sFile)
{
	char szBuffer[PATH_MAX] = UV_TEMP_PREFIX;
	int fd = 0;
	
	fd = mkstemp(szBuffer);
	if( fd < 0 )
	{
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	close(fd);
	sFile = szBuffer;
	return UV_ERR_OK;
}

std::string escapeArg(const std::string &sIn)
{
	std::string sRet;
	for( std::string::size_type i = 0; i < sIn.size(); ++i )
	{
		if( !((sIn[i] >= 'A' && sIn[i] <= 'Z')
				|| (sIn[i] >= 'a' && sIn[i] <= 'z')
				|| (sIn[i] >= '0' && sIn[i] <= '9')
				|| sIn[i] == '/'
				|| sIn[i] == '.'
				|| sIn[i] == '_'
				|| sIn[i] == '-'
				) )
		{
		  	sRet += '\\';
		}
		sRet += sIn[i];
	}
	return sRet;
}

uv_err_t deleteFile(std::string &sFile)
{
	if( unlink(sFile.c_str()) )
	{
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	return UV_ERR_OK;
}

uv_err_t executeToFile(const std::string &sCommand,
		const std::vector<std::string> &args,
		int &rcProcess,
		const std::string *stdOutIn,
		const std::string *stdErrIn)
{
	uv_err_t rc = UV_ERR_GENERAL;
	std::string stdOut;
	std::string stdErr;
	int iRet = 0;
	std::string sExec;
	
	UV_ENTER();

	if( stdOutIn )
	{
		stdOut = *stdOutIn;
	}
	if( stdErrIn )
	{
		stdErr = *stdErrIn;
	}
	
	sExec = escapeArg(sCommand);
	for( std::vector<std::string>::size_type i = 0; i < args.size(); ++i )
	{
		sExec += " ";
		sExec += escapeArg(args[i]);
	}
	if( stdOut.empty() )
	{
		stdOut = "/dev/null";
	}
	sExec += " 1>";
	sExec += stdOut;	
	if( stdErr.empty() )
	{
		stdErr = "/dev/null";
	}
	sExec += " 2>";
	sExec += stdErr;	
	
	printf_debug("Executing: %s\n", sExec.c_str());
	iRet = system(sExec.c_str());
	printf_debug("Ret: %d\n", iRet);
	uv_assert(!(iRet & 0xFF));
	rc = UV_ERR_OK;

error:
	return UV_DEBUG(rc);
}

uv_err_t executeToText(const std::string &sCommand,
		const std::vector<std::string> &args,
		int &rcProcess,
		std::string *stdOut,
		std::string *stdErr)
{
	uv_err_t rc = UV_ERR_GENERAL;
	static std::string stdOutFile;
	static std::string stdErrFile;
	std::string stdOutFileLocal;
	std::string stdErrFileLocal;
	
	UV_ENTER();
	
	if( stdOutFile.empty() )
	{
		uv_assert_err(getTempFile(stdOutFile));
	}
	if( stdErrFile.empty() )
	{
		uv_assert_err(getTempFile(stdErrFile));
	}
	
	if( stdOut )
	{
		stdOutFileLocal = stdOutFile;		
	}
	if( stdErr )
	{
		if( stdOut == stdErr )
		{
			stdErrFileLocal = stdOutFile;
			stdErr = NULL;
		}
		else
		{
			stdErrFileLocal = stdErrFile;
		}
	}
	
	uv_assert_err(executeToFile(sCommand,
			args,
			rcProcess,
			&stdOutFileLocal,
			&stdErrFileLocal));
	
	if( stdOut && UV_FAILED(readFile(stdOutFile, *stdOut)) )
	{
		goto error;
	}
	if( stdErr && UV_FAILED(readFile(stdErrFile, *stdErr)) )
	{
		goto error;
	}
	
	rc = UV_ERR_OK;
	
error:
	
	if( stdOut )
	{
		if( UV_FAILED(deleteFile(stdOutFileLocal)) && UV_SUCCEEDED(rc) )
		{
			rc = UV_ERR_GENERAL;
		}
	}
	if( stdErr )
	{
		if( UV_FAILED(deleteFile(stdErrFileLocal)) && UV_SUCCEEDED(rc) )
		{
			rc = UV_ERR_GENERAL;
		}
	}
	
	return UV_DEBUG(rc);
}

uv_err_t uvdPreprocessLine(const std::string &lineIn, std::string &lineOut)
{
	/*
	Skip comments, empty 
	Must be at beginning, some assemblers require # chars
	*/
	if( lineIn.empty() )
	{
		lineOut = "";
		return UV_ERR_BLANK;
	}
	if( lineIn.c_str()[0] == '#' )
	{
		lineOut = "";
		return UV_ERR_BLANK;
	}
	
	lineOut = lineIn;
	
	return UV_ERR_OK;
}

uv_err_t uvdParseLine(const std::string &line, std::string &key, std::string &value)
{
	uv_err_t rc = UV_ERR_GENERAL;
	std::string::size_type equalsPos = 0;

	/*
	Skip comments, empty 
	Must be at beginning, some assemblers require # chars
	*/
	if( line.empty() )
	{
		rc = UV_ERR_BLANK;
		goto error;	
	}
	if( line.c_str()[0] == '#' )
	{
		rc = UV_ERR_BLANK;
		goto error;	
	}

	/* Double check we aren't a section */
	uv_assert(line[0] != '.');

	/* Setup key/value pairing */
	equalsPos = line.find("=");
	if( equalsPos == std::string::npos )
	{
		UV_ERR(rc);
		goto error;
	}
	//Should have at least one char for key
	if( equalsPos < 1 )
	{
		UV_ERR(rc);
		goto error;
	}
	key = line.substr(0, equalsPos);
	//Skip the equals sign, make key comparisons easier
	//Value can be empty
	++equalsPos;
	if( equalsPos >= line.size() )
	{
		value = "";
	}
	else
	{
		value = line.substr(equalsPos);
	}

	printf_debug("key: %s, value: %s\n", key.c_str(), value.c_str());
	rc = UV_ERR_OK;
	
error:
	return UV_DEBUG(rc);
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
		printf_debug("Could not open file: %s\n", sFile.c_str());
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

#ifdef __cplusplus
//extern "C"
//{
#endif /* ifdef __cplusplus */

//FIXME: make this parse directly, will make O(n**2) -> O(n)
char **uv_split_lines(const char *str, unsigned int *n_ret)
{
	return uv_split_core(str, '\n', n_ret, TRUE);
}

char **uv_split(const char *str, char delim)
{
	return uv_split_core(str, delim, NULL, 1);
}

/*
str: string to split
delim: character to delimit by
n_ret: if set, will return number of items in the output, otherwise, output is a null terminated array
ret_blanks: return empty strings?
*/
char **uv_split_core(const char *str, char delim, unsigned int *n_ret, int ret_blanks)
{
	unsigned int n = 0;
	char *buff = NULL;
	static char **ret = NULL;
	unsigned int str_index = 0;
	unsigned int i = 0;

	n = uv_get_num_tokens_core(str, delim, ret_blanks);
	if( n_ret )
	{
		*n_ret = n;
	}
	else
	{
		++n;
	}
	ret = (char **)malloc(sizeof(char *) * (n));
	if( !ret || (n_ret && n == 0) )
	{
		return NULL;
	}
	/* The extra bit of n is only needed shortly during allocation */
	if( !n_ret )
	{
		--n;
		ret[n] = NULL;
	}
	buff = strdup(str);
	if( !buff )
	{
		return NULL;
	}

	for( i = 0; i < n; ++i )
	{
		unsigned int j = 0;
		for( j = 0; str[str_index] != delim && str[str_index] != 0; ++j, ++str_index )
		{
			buff[j] = str[str_index];
		}
		buff[j] = 0;
		/* skip over the null */
		++str_index;
		
		ret[i] = strdup(buff);
		if( ret[i] == NULL )
		{
			goto error;
		}
	}
	free(buff);
	return ret;

error:
	for( ;; )
	{
		free(ret[i]);
		if( i == 0 )
		{
			break;
		}
		--i;
	}
	free(ret);
	free(buff);
	return NULL;
}

unsigned int uv_get_num_lines(const char *str)
{
	return uv_get_num_tokens_core(str, '\n', TRUE);
}

unsigned int uv_get_num_tokens_core(const char *text, char delim, int ret_blanks)
{
	int ret = 1;
	unsigned int i = 0;
	for( i = 0; text[i] != 0; ++i )
	{
		if( text[i] == delim && (ret_blanks || (i != 0 && text[i - 1] != delim)) )
		{
			++ret;
		}
	}
	return ret;
}

char *uv_get_line(const char *text, unsigned int lineNumber)
{
	char *line = NULL;
	unsigned int index = 0;
	char cur_char='a';
	unsigned int line_size = 0;
	unsigned int i = 0;

	for(i = 0; i < lineNumber; ++i)
	{
		while( text[index] != '\n' )
		{
			++index;
		}
	}
	//Find line size
	for( line_size = 0; TRUE; ++line_size )
	{
		cur_char = text[index + line_size];
		if( cur_char == '\n' || cur_char == '\r' || cur_char == 0 )
		{
			break;
		}
	}
	line = (char *)malloc(sizeof(char) * (line_size + 1));
	if( !line )
	{
		return NULL;
	}
	//Then copy
	for( i = 0; TRUE; ++i )
	{
		cur_char = text[index + i];
		if( cur_char == '\n' || cur_char == '\r' || cur_char == 0 )
		{
			break;
		}
		line[i] = cur_char;
	}
	line[i] = 0;
	
	return line;
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

/*
Disassembly specific utilities 
*/
/*
inline char toupper(char c)
{
	//upper A: 0x41
	//lower A: 0x61
	return c & (~0x20);
}

inline char toupper(char c)
{
	//upper A: 0x41
	//lower A: 0x61
	return c | 0x20;
}
*/

char *cap_fixup(char *str)
{
	char *ptr = str;
	
	UV_ENTER();

	if( !str )
	{
		return str;
	}
	if( g_caps )
	{
		while( *ptr )
		{
			*ptr = toupper(*ptr);
			++ptr;
		}
	}
	else
	{
		while( *ptr )
		{
			*ptr = tolower(*ptr);
			++ptr;
		}
	}
	return str;
}

uv_err_t uvd_parse_line(const char *line_in, char **key_in, char **value_in)
{
	uv_err_t rc = UV_ERR_GENERAL;
	char *key = NULL;
	char *value = NULL;
	char *line = strdup(line_in);

	uv_assert(key_in);
	uv_assert(value_in);
	/* Double check we aren't a section */
	uv_assert(line[0] != '.');

	/*
	Skip comments, empty 
	Must be at beginning, some assemblers require # chars
	*/
	if( line[0] == 0 || line[0] == '#' )
	{
		rc = UV_ERR_BLANK;
		goto error;	
	}

	/* Setup key/value pairing */
	value = strstr(line, "=");
	if( !value )
	{
		UV_ERR(rc);
		goto error;
	}
	//Make equals zero for key
	value[0] = 0;
	//Skip equals
	++value;
	key = strdup(line);
	if( !key )
	{
		UV_ERR(rc);
		goto error;
	}
	/* Skip the equals sign, make key comparisons easier */
	value = strdup(value);
	if( !value )
	{
		free(key);
		free(value);
		//printf_debug("ERROR: out of memory allocating value string\n");
		UV_ERR(rc);
		goto error;
	}
	free(line);
	line = NULL;
	*key_in = key;
	*value_in = value;

	printf_debug("key: %s, value: %s\n", key, value);
	rc = UV_ERR_OK;
	
error:
	return UV_DEBUG(rc);
}

uv_err_t uvd_parse_func(const char *text, char **name, char **content)
{
	uv_err_t rc = UV_ERR_GENERAL;
	char *loc = NULL;
	char *copy = NULL;
	char *end = NULL;
	
	uv_assert(text);
	uv_assert(name);
	uv_assert(content);

	printf_debug("uvd_parse_func, in: <%s>\n", text);

	copy = strdup(text);
	uv_assert_all(copy);

	loc = strstr(copy, "(");
	uv_assert_all(loc);
	loc[0] = 0;
	++loc;
	
	//Find last )
	end = loc;
	for( ;; )
	{
		char *temp = NULL;
		
		temp = strstr(end, ")");
		if( temp == NULL )
		{
			//Did we find at least one?
			if( end == loc )
			{
				return UV_DEBUG(UV_ERR_GENERAL);
			}
			break;
		}
		end = temp + 1;
	}
	end[0] = 0;
	
	*name = strdup(copy);
	uv_assert_all(*name);

	*content = strdup(loc);
	uv_assert_all(*content);
		
	free(copy);
	
	rc = UV_ERR_OK;
	
error:
	return UV_DEBUG(rc);
}

#ifdef __cplusplus
//}
#endif /* ifdef __cplusplus */

std::vector<std::string> charPtrArrayToVector(char **argv, int argc)
{
	std::vector<std::string> ret;
	for( int i = 0; i < argc; ++i )
	{
		ret.push_back(argv[i]);
	}
	return ret;
}

/*
For splitting up config sections
Sections must start with delim and output will include it
*/
uv_err_t splitConfigLinesVector(const std::vector<std::string> &in, const std::string &delim, std::vector< std::vector<std::string> > &out)
{
	std::vector<std::string> next;
	for( std::vector<std::string>::size_type i = 0; i < in.size(); ++i )
	{
		std::string partRaw = in[i];
		std::string part;
		
		// FIXME: this should be preprocessed out before this
		uv_assert_err_ret(uvdPreprocessLine(partRaw, part));
		if( part.empty() )
		{
			continue;
		}

		//Did we find delim?
		if( part.find(delim) == 0 )
		{
			//Add last entry, if present
			if( !next.empty() )
			{
				out.push_back(next);
				next.clear();
			}
		}
		else
		{
			//We should have an entry on first usable line, early error indicator
			if( next.empty() )
			{
				printf_debug("Got: %s, expected: %s\n", part.c_str(), delim.c_str());
				return UV_DEBUG(UV_ERR_GENERAL);
			}
		}
		
		//Build current item
		next.push_back(part);
	}
	
	//Add final entry, if present (none if blank input)
	if( !next.empty() )
	{
		out.push_back(next);
		next.clear();
	}
	
	return UV_ERR_OK;
}

uv_err_t getArguments(const std::string &in, std::vector<std::string> &out)
{
	std::string cur;
	int parenCount = 0;
	
	UV_ENTER();
	
	for( std::string::size_type i = 0; ; ++i )
	{
		char c  = 0;
		if( i < in.size() )
		{
			c = in[i];
		}
		
		if( c == '(' )
		{
			++parenCount;
			cur += c;
		}
		else if( c == ')' )
		{
			if( parenCount == 0 )
			{
				return UV_DEBUG(UV_ERR_GENERAL);
			}
			--parenCount;
			cur += c;
		}
		//Nested function argument?
		else if( c == ',' && parenCount > 0 )
		{
			cur += c;
		}
		//End of argument?
		else if( c == ',' || i >= in.size() )
		{
			//Current level argument
			if( cur.empty() )
			{
				return UV_DEBUG(UV_ERR_GENERAL);
			}
			out.push_back(cur);
			cur = "";
			
			//Done?
			if( i >= in.size() )
			{
				break;
			}
		}
		//Otherwise add to current
		else
		{
			cur += c;
		}
	}

	printf_debug("Separated: %s, parts: %d\n", in.c_str(), out.size());
	
	return UV_ERR_OK;
}

uint64_t getTimingMicroseconds(void)
{
	struct timeval tv;
	struct timezone tz;
	struct tm *tm;
	gettimeofday(&tv, &tz);
	tm=localtime(&tv.tv_sec);
	return ((tm->tm_hour * 60 + tm->tm_min) * 60 + tm->tm_sec) * 1000000 + tv.tv_usec;
}
