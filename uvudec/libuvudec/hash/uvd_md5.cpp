/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd_types.h" 
#include "uvd_md5.h"
#include "uvd_md5_GNU.h"
#include <stdlib.h>

void uv_md5_format(char resblock[16], std::string &sOut)
{
	sOut = "";
	for( unsigned int i = 0; i < 16; ++i )
	{
		char buff[8];
		snprintf(buff, 8, "%.2X", (unsigned int)(unsigned char)resblock[i]);
	
		sOut += buff;
	}
	printf_debug("Hash: %s\n", sOut.c_str());
}

//Simple MD5 for text
uv_err_t uv_md5(const std::string &sIn, std::string &sOut)
{
	//Binary MD5
	char resblock[16];
	
	md5_buffer(sIn.c_str(), sIn.size(), (void *)resblock);

	uv_md5_format(resblock, sOut);
	
	return UV_ERR_OK;
}

//For binary
uv_err_t uv_md5(const char *buff, uint32_t buffSize, std::string &sOut)
{
	//Binary MD5
	char resblock[16];
	
	uv_assert_ret(buff);
	
	md5_buffer(buff, buffSize, resblock);

	uv_md5_format(resblock, sOut);

	return UV_ERR_OK;
}
