/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVDASM_PLUGIN_H
#define UVDASM_PLUGIN_H

#include "uvd/plugin/plugin.h"
#include "uvd/util/types.h"
#include "uvdasm/config.h"

class UVDAsmPlugin : public UVDPlugin
{
public:
	UVDAsmPlugin();
	~UVDAsmPlugin();
	virtual uv_err_t init(UVDConfig *config);
	virtual uv_err_t onUVDInit(UVD *uvd);

	virtual uv_err_t getName(std::string &out);
	virtual uv_err_t getDescription(std::string &out);	
	virtual uv_err_t getVersion(UVDVersion &out);
	virtual uv_err_t getAuthor(std::string &out);
	virtual uv_err_t canGetArchitecture(const UVDObject *object, const UVDRuntimeHints &hints, uvd_priority_t *confidence);
	virtual uv_err_t getArchitecture(UVDObject *object, const UVDRuntimeHints &hints, UVDArchitecture **out);

public:
	UVDAsmConfig m_config;
};

extern UVDAsmPlugin *g_asmPlugin;

#endif

