/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/plugin/plugin.h"
#include "uvd/util/io.h"
#include "uvd/core/uvd.h"

uv_err_t UVDPluginMain(UVDConfig *config, UVDPlugin **out)
{
	UVDPrint("Plugin loaded!\n");
	return UV_ERR_OK;
}

