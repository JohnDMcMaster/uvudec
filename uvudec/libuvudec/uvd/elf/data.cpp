/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

/*
ELF I/O related stuff
*/

#include "uvd/elf/elf.h"
#include "uvd/elf/writer.h"

/*
Top level construction
*/
	
uv_err_t UVDElf::constructBinary(UVDData **dataOut)
{
	UVDElfWriter writer;
	
	uv_assert_err_ret(writer.init(this));
	uv_assert_err_ret(writer.constructBinary(dataOut));
	
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
