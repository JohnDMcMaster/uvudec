/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#ifndef UVD_CRC_H
#define UVD_CRC_H

#include <stdint.h>

uint16_t uvd_crc16(const char *data_p, uint32_t length);

#endif
