/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_GUI_FORMAT_H
#define UVD_GUI_FORMAT_H

#include "uvd/language/format.h"

class UVDGUIFormat : public UVDFormat
{
public:
	UVDGUIFormat();
	~UVDGUIFormat();

	virtual uv_err_t formatAddress(uint32_t address, std::string &ret);
	uv_err_t addressToAnchorName(uv_addr_t address, std::string &ret);
};

#endif

