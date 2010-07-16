/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_ARG_UTIL_H
#define UVD_ARG_UTIL_H

#include "uvd_arg.h"
#include "uvd_types.h"
#include <stdio.h>
#include <string>

uv_err_t parseFileOption(const std::string optionFileIn, FILE **pOptionFileIn);
bool argToBool(const std::string &sArg);
uv_err_t processArgCore(const std::string &arg, UVDParsedArg &parsedArg);
uv_err_t processArg(const std::string &arg, std::vector<UVDParsedArg> &parsedArgs);
uv_err_t matchArgConfig(const std::vector<UVDArgConfig *> &argConfigs, UVDParsedArg &arg, UVDArgConfig const **matchedArgConfig);

#endif
