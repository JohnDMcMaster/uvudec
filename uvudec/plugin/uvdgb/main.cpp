/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/plugin/plugin.h"
#include "uvd/util/io.h"
#include "uvd/core/uvd.h"
#include "uvd/util/version.h"
#include "uvdgb/plugin.h"

uv_err_t UVD_PLUGIN_MAIN_SYMBOL(UVDConfig *config, UVDPlugin **out)
{
	UVDPrint("Plugin main entry!\n");
	*out = new UVDObjgbPlugin();
	
	return UV_ERR_OK;
}

