/*
UVNet Universal Decompiler (uvudec)
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

/* Get the name and args as a string */
uv_err_t uvd_parse_func(const char *text, char **name, char **content);

uv_err_t parseFunc(const std::string &text, std::string &name, std::string &content)
{
	char *nameTemp = NULL;
	char *contentTemp = NULL;

	if( UV_FAILED(uvd_parse_func(text.c_str(), &nameTemp, &contentTemp)) )
	{
		return UV_ERR_GENERAL;
	}

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
	if( !loc )
	{
		free(copy);
		return UV_ERR_GENERAL;
	}
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
				return UV_ERR_GENERAL;
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
	return rc;
}

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

uv_err_t isCSymbol(const std::string &in)
{
	//[a-zA-Z_][z-zA-Z0-9_].

	if( in.empty() )
	{
		return UV_ERR_GENERAL;
	}
	
	//[a-zA-Z_]
	if( !(in[0] >= 'a' && in[0] <= 'z'
			|| in[0] >= 'A' && in[0] <= 'Z'
			|| in[0] == '_') )
	{
		return UV_ERR_GENERAL;
	}
	
	//[z-zA-Z0-9_].
	for( std::string::size_type i = 1; i < in.size(); ++i )
	{
		if( !(in[0] >= 'a' && in[0] <= 'z'
				|| in[0] >= 'A' && in[0] <= 'Z'
				|| in[0] >= '0' && in[0] <= '9'
				|| in[0] == '_') )
		{
			return UV_ERR_GENERAL;
		}
	}

	return UV_ERR_OK;
}

uv_err_t isConfigIdentifier(const std::string &in)
{
	return isCSymbol(in);
}
