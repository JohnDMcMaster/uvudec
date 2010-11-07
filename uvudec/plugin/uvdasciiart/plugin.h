/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVDASCIIART_PLUGIN_H
#define UVDASCIIART_PLUGIN_H

#include "uvd/plugin/plugin.h"
#include "uvd/util/types.h"
#include "uvdasciiart/config.h"

class UVDAsciiartPlugin : public UVDPlugin
{
public:
	UVDAsciiartPlugin();
	~UVDAsciiartPlugin();
	virtual uv_err_t init(UVDConfig *config);

	virtual uv_err_t getName(std::string &out);
	virtual uv_err_t getDescription(std::string &out);	
	virtual uv_err_t getVersion(UVDVersion &out);
	virtual uv_err_t getAuthor(std::string &out);

public:
	UVDASCIIArtConfig m_config;
};

extern UVDAsciiartPlugin *g_asciiArtPlugin;

#endif

