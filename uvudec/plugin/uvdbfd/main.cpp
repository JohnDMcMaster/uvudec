/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/init/config.h"
#include "uvd/util/version.h"
#include "uvd/util/io.h"
#include "uvdbfd/plugin.h"

//extern "C"
//{
	uv_err_t UVD_PLUGIN_MAIN_SYMBOL(UVDConfig *config, UVDPlugin **out)
	{
		UVDPrint("Plugin main entry!\n");
		*out = new UVDBFDPlugin();
	
		//initialize arg structures here...
	
		return UV_ERR_OK;
	}
//}

