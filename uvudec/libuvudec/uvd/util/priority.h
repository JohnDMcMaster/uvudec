/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_UTIL_PRIORITY_H
#define UVD_UTIL_PRIORITY_H

#include <limits.h>
#include <stdint.h>

typedef uint32_t uvd_priority_t;

//Force a match
//Either because we are doing a hack or because there is very good reason to do so
//You probably shouldn't be using this except for testing, use manual selection instead of required
#define UVD_MATCH_OVERRIDE				0
//Something designed for exactly this situation has been found
//Ex: a PlayStation module
//It knows *exactly* what it means to have this binary format as far as peripherals etc
#define UVD_MATCH_GOOD					0x00000100
//We can load and do reasonable analysis
#define UVD_MATCH_ACCEPTABLE			0x00010000
//While its possible it could work, will require guidance at very least
//Ex: 8051 ISA, but we can only load 8051 binaries and we have a (specially coded) ELF file
#define UVD_MATCH_POOR					0x00100000
//Would not produce coherent analysis
//Ex: we know its x86 ISA and its an ARM module
//These shouldn't even be presented as an option to the user as a match (unless manually selected for w/e reason)
#define UVD_MATCH_NONE					UINT_MAX

#endif

