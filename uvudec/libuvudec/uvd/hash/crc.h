/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_CRC_H
#define UVD_CRC_H

#include <stdint.h>

uint16_t uvd_crc16(const char *data_p, uint32_t length);

#endif
