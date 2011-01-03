/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVDBFD_CONFIG_H
#define UVDBFD_CONFIG_H

#include "uvd/config/config.h"
#include <string>

#define UVDSTRINGS_ENCODINGS						UVD_PLUGIN_PROPERTY("encoding")
//ASCII probably
#define UVDSTRINGS_ENCODINGS_DEFAULT				""

class UVDStringsConfig
{
public:
	UVDStringsConfig();
	~UVDStringsConfig();
	
	uv_err_t init(UVDConfig *config);

public:
	std::string m_architecture;
};

#endif

