/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVDFLIRT_PLUGIN_H
#define UVDFLIRT_PLUGIN_H

#include "uvd/plugin/plugin.h"
#include "uvd/util/types.h"
#include "uvdflirt/config.h"

class UVDFLIRT;
class UVDFLIRTPlugin : public UVDPlugin
{
public:
	UVDFLIRTPlugin();
	~UVDFLIRTPlugin();
	virtual uv_err_t init(UVDConfig *config);

	virtual uv_err_t getName(std::string &out);
	virtual uv_err_t getDescription(std::string &out);	
	virtual uv_err_t getVersion(UVDVersion &out);
	virtual uv_err_t getAuthor(std::string &out);
	
	virtual uv_err_t onUVDInit();
	virtual uv_err_t onUVDDeinit();

public:
	UVDFLIRTConfig m_config;
	UVDFLIRT *m_flirt;
};

extern UVDFLIRTPlugin *g_uvdFLIRTPlugin;

#endif

