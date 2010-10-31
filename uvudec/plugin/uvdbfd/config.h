/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVDBFD_CONFIG_H
#define UVDBFD_CONFIG_H

#include "uvd/config/config.h"
#include <string>

#define UVDBFD_ARCHITECTURE_HINT						"plugin." UVD_PLUGIN_NAME ".architecture_hint"
//This means automatic in libbfd
#define UVDBFD_ARCHITECTURE_HINT_DEFAULT				""

class UVDBFDConfig
{
public:
	UVDBFDConfig();
	~UVDBFDConfig();
	
	uv_err_t init(UVDConfig *config);

public:
	std::string m_architecture;
};

extern UVDBFDConfig *g_bfdConfig;

#endif

