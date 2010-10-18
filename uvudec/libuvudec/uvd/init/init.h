/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_INIT_H
#define UVD_INIT_H

//Call these before doing any other operations with libuvudec
uv_err_t UVDInit();
//When you are done, call this
//Any objects allocated from libuvudec should be properly free'd before calling this
uv_err_t UVDDeinit();

#endif
