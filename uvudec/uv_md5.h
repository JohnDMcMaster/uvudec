/*
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under the terms of the BSD license.  See LICENSE for details.
*/

#pragma once

#include "uvd_types.h"

//Simple MD5 for text
uv_err_t uv_md5(const std::string &sIn, std::string &sOut);

//For binary
uv_err_t uv_md5(const char *buff, uint32_t buffSize, std::string &sOut);
