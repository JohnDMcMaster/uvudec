/*
UVNet Universal Decompiler (uvudec)
uvudec copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
This file contains code not written by the uvudec team

This is used in a variety of projects I see when you search for crc16 impl
My best guess is the original is at:
http://www.aerospacesoftware.com/checks.htm
Since it also contains explanation and other implementations
Code has minor modifications by John McMaster <JohnDMcMaster@gmail.com>
*/

/*
 *						  16   12   5
 * this is the CCITT CRC 16 polynomial X  + X  + X  + 1.
 * This is 0x1021 when x is 2, but the way the algorithm works
 * we use 0x8408 (the reverse of the bit pattern).  The high
 * bit is always assumed to be set, thus we only use 16 bits to
 * represent the 17 bit value.
*/

#include "uvd/hash/crc.h"

#define POLY 0x8408   /* 1021H bit reversed */

uint16_t uvd_crc16(const char *data_p, uint32_t length)
{
	uint8_t i;
	uint16_t data;
	uint16_t crc = 0xffff;

	if (length == 0)
		return (~crc);
	do
	{
		for (i=0, data=(unsigned int)0xff & *data_p++;
		     i < 8; 
		     i++, data >>= 1)
		{
			if ((crc & 0x0001) ^ (data & 0x0001))
				crc = (crc >> 1) ^ POLY;
			else  crc >>= 1;
		}
	} while (--length);

	crc = ~crc;
	data = crc;
	crc = (crc << 8) | (data >> 8 & 0xff);

	return (crc);
}
