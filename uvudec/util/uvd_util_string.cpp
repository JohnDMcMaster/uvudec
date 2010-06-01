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

uv_err_t parseNumericRangeString(const std::string &s, uint32_t *first, uint32_t *second)
{
	char delim = 0;
	std::vector<std::string> parts;
	
	uv_assert_ret(first);
	uv_assert_ret(second);
	
	if( s.find('-') != std::string::npos )
	{
		delim = '-';
	}
	else if( s.find(',') != std::string::npos )
	{
		delim = ',';
	}
	else if( s.find(':') != std::string::npos )
	{
		delim = ':';
	}
	//Self to self then
	else
	{
		*first = strtol(s.c_str(), NULL, 0);
		*second = *first;
		return UV_ERR_OK;
	}
	
	parts = split(s, delim, true);
	uv_assert_ret(parts.size() == 2);
	*first = strtol(parts[0].c_str(), NULL, 0);
	*second = strtol(parts[1].c_str(), NULL, 0);
	
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
XXX: this is really old code that seems to work, but really should be phased out

str: string to split
delim: character to delimit by
	If null, will return the array with a single string
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

char *cap_fixup(char *str)
{
	char *ptr = str;
	
	UV_ENTER();

	if( !g_config )
	{
		return NULL;
	}

	if( !str )
	{
		return str;
	}
	if( g_config->m_caps )
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
