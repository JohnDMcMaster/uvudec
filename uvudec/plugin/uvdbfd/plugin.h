/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVDBFD_PLUGIN_H
#define UVDBFD_PLUGIN_H

#include "uvd/plugin/plugin.h"
#include "uvd/util/types.h"

class UVDBFDPlugin : public UVDPlugin
{
public:
	UVDBFDPlugin();
	~UVDBFDPlugin();

	virtual uv_err_t onUVDInit(UVD *uvd);
	virtual uv_err_t getName(std::string &out);
	virtual uv_err_t getDescription(std::string &out);	
	virtual uv_err_t getVersion(UVDVersion &out);
	virtual uv_err_t getAuthor(std::string &out);
	virtual uv_err_t getObject(UVDData *data, const UVDRuntimeHints &hints, UVDObject **out);
	virtual uv_err_t getArchitecture(UVDObject *object, const UVDRuntimeHints &hints, UVDArchitecture **out);
};

#endif

