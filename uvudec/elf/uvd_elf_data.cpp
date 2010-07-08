/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

/*
ELF I/O related stuff
*/

#include "uvd_elf.h"
#include "uvd_elf_writter.h"

/*
Top level construction
*/
	
uv_err_t UVDElf::constructBinary(UVDData **dataOut)
{
	UVDElfWritter writter;
	
	uv_assert_err_ret(writter.init(this));
	uv_assert_err_ret(writter.constructBinary(dataOut));
	
	return UV_ERR_OK;
}

/*
Misc functions
*/

uv_err_t UVDElf::saveToFile(const std::string &file)
{
	UVDData *data = NULL;
	//Get the raw binary data
	uv_assert_err_ret(constructBinary(&data));
	uv_assert_ret(data);
	//And save it
	uv_assert_err_ret(data->saveToFile(file));
	
	return UV_ERR_OK;
}
