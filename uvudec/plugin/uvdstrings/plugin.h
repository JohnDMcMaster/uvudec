/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVDSTRINGS_PLUGIN_H
#define UVDSTRINGS_PLUGIN_H

#include "uvd/plugin/plugin.h"
#include "uvd/util/types.h"

class UVDStringsPlugin : public UVDPlugin
{
public:
	UVDStringsPlugin();
	~UVDStringsPlugin();
	virtual uv_err_t init(UVDConfig *config);

	virtual uv_err_t getName(std::string &out);
	virtual uv_err_t getDescription(std::string &out);	
	virtual uv_err_t getVersion(UVDVersion &out);
	virtual uv_err_t getAuthor(std::string &out);

public:
};

#endif

