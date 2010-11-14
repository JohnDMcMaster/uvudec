/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_ASCII_ART_H
#define UVD_ASCII_ART_H

#include <string>
#include <vector>
#include "uvd/util/types.h"

uv_err_t getUVNetASCIIArt(std::vector<std::string> &out);
uv_err_t getRandomUVNetASCIIArt(std::string &out);

#endif
