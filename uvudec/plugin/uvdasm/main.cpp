/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "plugin/plugin.h"
#include "util/io.h"
#include "uvd.h"
#include "util/uvd_version.h"
#include "uvdasm/plugin.h"

extern "C"
{
	uv_err_t UVD_PLUGIN_MAIN_SYMBOL(UVDConfig *config, UVDPlugin **out)
	{
		UVDPrint("Plugin main entry!\n");
		*out = new UVDDisassemblerPlugin();
	
		//initialize arg structures here...
	
		return UV_ERR_OK;
	}
}

