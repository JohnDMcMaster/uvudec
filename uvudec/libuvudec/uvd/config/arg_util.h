/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_ARG_UTIL_H
#define UVD_ARG_UTIL_H

#include "uvd/config/arg.h"
#include "uvd/util/types.h"
#include <stdio.h>
#include <string>

uv_err_t parseFileOption(const std::string optionFileIn, FILE **pOptionFileIn);
bool UVDArgToBool(const std::string &sArg);
uv_err_t UVDProcessArgCore(const std::string &arg, UVDParsedArg &parsedArg);
uv_err_t UVDProcessArg(const std::string &arg, std::vector<UVDParsedArg> &parsedArgs);
uv_err_t UVDMatchArgConfig(const UVDArgConfigs &argConfigs, UVDParsedArg &arg, UVDArgConfig const **matchedArgConfig);

#endif
