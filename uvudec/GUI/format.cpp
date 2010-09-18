/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "GUI/format.h"

UVDGUIFormat::UVDGUIFormat()
{
}

UVDGUIFormat::~UVDGUIFormat()
{
}

uv_err_t UVDGUIFormat::formatAddress(uint32_t address, std::string &ret)
{
	std::string plainAddress;
	std::string anchor;
	
	uv_assert_err_ret(UVDFormat::formatAddress(address, plainAddress));
	uv_assert_err_ret(addressToAnchorName(address, anchor));
	
	ret = std::string("<A href=\"#") + anchor + "\">" + plainAddress + "</A>";
	
	return UV_ERR_OK;
}

uv_err_t UVDGUIFormat::addressToAnchorName(uv_addr_t address, std::string &ret)
{
	char buff[32];
	
	snprintf(buff, sizeof(buff), "0x%08X", address);
	ret = buff;

	return UV_ERR_OK;
}

