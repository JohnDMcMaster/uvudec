/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef TESTING_MAIN_HOOK_H
#define TESTING_MAIN_HOOK_H

#include "uvd/util/types.h"

uv_err_t uvudec_uvmain(int argc, char **argv);
uv_err_t uvobj2pat_uvmain(int argc, char **argv);
uv_err_t uvpat2sig_uvmain(int argc, char **argv);
uv_err_t uvflirtutil_uvmain(int argc, char **argv);

#endif
