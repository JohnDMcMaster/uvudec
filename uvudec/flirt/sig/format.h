/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details

Some code originally from
http://www.woodmann.com/forum/showthread.php?7517-IDA-signature-file-format
*/

#include <stdint.h>
#include <string>

#ifndef UVD_FLIRT_SIGNATURE_FORMAT_H
#define UVD_FLIRT_SIGNATURE_FORMAT_H

/*
Feature flags
Uh need to figure out what these mean
*/
#define UVD__IDASIG__FEATURE__STARTUP					0x01
#define UVD__IDASIG__FEATURE__CTYPE_CRC					0x02
#define UVD__IDASIG__FEATURE__2BYTE_CTYPE				0x04
#define UVD__IDASIG__FEATURE__ALT_CTYPE_CRC				0x08
//Compressed of course
#define UVD__IDASIG__FEATURE__COMPRESSED				0x10

/*
Architecture enumeration
*/
//"Intel 80x86"
#define UVD__IDASIG__ARCH__80X86						0x00
//"8085, Z80"
#define UVD__IDASIG__ARCH__Z80							0x01
//"Intel 860"
#define UVD__IDASIG__ARCH__INTEL_860					0x02
//"8051"
#define UVD__IDASIG__ARCH__8051							0x03
//"TMS320C5x"
#define UVD__IDASIG__ARCH__TMS320C5X					0x04
//"6502"
#define UVD__IDASIG__ARCH__6502							0x05
//"PDP11"
#define UVD__IDASIG__ARCH__PDP11						0x06
//"Motorola 680x0"
#define UVD__IDASIG__ARCH__MOTOROLA_680X0				0x07
//"Java"
#define UVD__IDASIG__ARCH__JAVA							0x08
//"Motorola 68xx"
#define UVD__IDASIG__ARCH__MOTOROLA_68XX				0x09
//"SGS-Thomson ST7"
#define UVD__IDASIG__ARCH__SGS_THOMSON_ST7				0x0A
//"Motorola 68HC12"
#define UVD__IDASIG__ARCH__MOTOROLA_68HC12				0x0B
//"MIPS"
#define UVD__IDASIG__ARCH__MIPS							0x0C
//"Advanced RISC Machines"
#define UVD__IDASIG__ARCH__ADVANCED_RISC				0x0D
//"TMS320C6x"
#define UVD__IDASIG__ARCH__TMS320C6X					0x0E
//"PowerPC"
#define UVD__IDASIG__ARCH__POWERPC						0x0F
//"Intel 80196"
#define UVD__IDASIG__ARCH__INTEL_80196					0x10
//"Z8"
#define UVD__IDASIG__ARCH__Z8							0x11
//"Hitachi SH"
#define UVD__IDASIG__ARCH__HITACHI_SH					0x12
//"Microsoft Visual Studio.Net"
#define UVD__IDASIG__ARCH__MSVS_DOT_NET					0x13
//"Atmel 8-bit RISC processor(s)"
#define UVD__IDASIG__ARCH__ATMEL_8_BIT_RISC				0x14
//"Hitachi H8/300, H8/2000"
#define UVD__IDASIG__ARCH__HITACHI_H8_300_H8_2000		0x15
//"Microchip's PIC"
#define UVD__IDASIG__ARCH__MICROCHIP_PIC				0x16
//"SPARC"
#define UVD__IDASIG__ARCH__SPARC						0x17
//"DEC Alpha"
#define UVD__IDASIG__ARCH__DEC_ALPHA					0x18
//"Hewlett-Packard PA-RISC"
#define UVD__IDASIG__ARCH__HP_PA_RISC					0x19
//"Hitachi H8/500"
#define UVD__IDASIG__ARCH__HITACHI_H8_500				0x1A
//"Tasking Tricore"
#define UVD__IDASIG__ARCH__TASKING_TRICORE				0x1B
//"Motorola DSP5600x"
#define UVD__IDASIG__ARCH__MOTOROLA_DSP5600X			0x1C
//"Siemens C166 family"
#define UVD__IDASIG__ARCH__SIEMENS_C166					0x1D
//"SGS-Thomson ST20"
#define UVD__IDASIG__ARCH__SGS_THOMSON_ST20				0x1E
//"Intel Itanium IA64"
#define UVD__IDASIG__ARCH__INTEL_ITANIUM_IA64			0x1F
//"Intel 960"
#define UVD__IDASIG__ARCH__INTEL_I960					0x20
//"Fujistu F2MC-16"
#define UVD__IDASIG__ARCH__FUJITSU_F2MC_16				0x21
//"TMS320C54xx"
#define UVD__IDASIG__ARCH__TMS320C54XX					0x22
//"TMS320C55xx"
#define UVD__IDASIG__ARCH__TMS320C55XX					0x23
//"Trimedia"
#define UVD__IDASIG__ARCH__TRIMEDIA						0x24
//"Mitsubishi 32bit RISC"
#define UVD__IDASIG__ARCH__MITSUBISH_32_BIT_RISC		0x25
//"NEC 78K0"
#define UVD__IDASIG__ARCH__NEC_78K0						0x26
//"NEC 78K0S"
#define UVD__IDASIG__ARCH__NEC_78K0S					0x27
//"Mitsubishi 8bit"
#define UVD__IDASIG__ARCH__MITSUBISHI_8_BIT				0x28
//"Mitsubishi 16bit"
#define UVD__IDASIG__ARCH__MITSIBUSHI_16_BIT			0x29
//"ST9+"
#define UVD__IDASIG__ARCH__ST9PLUS						0x2A
//"Fujitsu FR Family"
#define UVD__IDASIG__ARCH__FUJITSU_FR					0x2B
//"Motorola 68HC16"
#define UVD__IDASIG__ARCH__MOTOROLA_68HC16				0x2C
//"Mitsubishi 7900"
#define UVD__IDASIG__ARCH__MITSUBISHI_7900				0x2D

/*
Applicable operating system flags
*/
#define UVD__IDASIG__OS__MSDOS							0x01
#define UVD__IDASIG__OS__WIN							0x02
#define UVD__IDASIG__OS__OS2							0x04
#define UVD__IDASIG__OS__NETWARE						0x08
#define UVD__IDASIG__OS__UNIX							0x10

/*
Application types
Most of these are seemingly Windows-centric
*/
#define UVD__IDASIG__APP__CONSOLE						0x0001
#define UVD__IDASIG__APP__GRAPHICS						0x0002
#define UVD__IDASIG__APP__EXE							0x0004
#define UVD__IDASIG__APP__DLL							0x0008
#define UVD__IDASIG__APP__DRV							0x0010
#define UVD__IDASIG__APP__SINGLE_THREADED				0x0020
#define UVD__IDASIG__APP__MULTI_THREADED				0x0040
#define UVD__IDASIG__APP__16_BIT						0x0080
#define UVD__IDASIG__APP__32_BIT						0x0100
#define UVD__IDASIG__APP__64_BIT						0x0200

/*
Applicable file format flags
*/
#define UVD__IDASIG__FILE__DOS_EXE_OLD					0x00000001
#define UVD__IDASIG__FILE__DOS_COM_OLD					0x00000002
#define UVD__IDASIG__FILE__BIN							0x00000004
#define UVD__IDASIG__FILE__DOSDRV						0x00000008
#define UVD__IDASIG__FILE__NE							0x00000010
#define UVD__IDASIG__FILE__INTELHEX						0x00000020
#define UVD__IDASIG__FILE__MOSHEX						0x00000040
#define UVD__IDASIG__FILE__LX							0x00000080
#define UVD__IDASIG__FILE__LE							0x00000100
#define UVD__IDASIG__FILE__NLM							0x00000200
#define UVD__IDASIG__FILE__COFF							0x00000400
#define UVD__IDASIG__FILE__PE							0x00000800
#define UVD__IDASIG__FILE__OMF							0x00001000
#define UVD__IDASIG__FILE__SREC							0x00002000
#define UVD__IDASIG__FILE__ZIP							0x00004000
#define UVD__IDASIG__FILE__OMFLIB						0x00008000
#define UVD__IDASIG__FILE__AR							0x00010000
#define UVD__IDASIG__FILE__LOADER						0x00020000
#define UVD__IDASIG__FILE__ELF							0x00040000
#define UVD__IDASIG__FILE__W32RUN						0x00080000
#define UVD__IDASIG__FILE__AOUT							0x00100000
#define UVD__IDASIG__FILE__PILOT						0x00200000
#define UVD__IDASIG__FILE__DOS_EXE						0x00400000
#define UVD__IDASIG__FILE__AIXAR						0x00800000

//#define UVD__IDASIG__MAGIC							{'I', 'D', 'A', 'S', 'I', 'G'}
#define UVD__IDASIG__MAGIC								"IDASIG"
#define UVD__IDASIG__MAGIC_SIZE							6

/*
Name printing related
*/
//Another name should be read
#define UVD__IDASIG__NAME__MORE_NAMES					0x01
//What is this?  External reference
//printf(" (0x%.4X: 0x%.2X)", first, second);
#define UVD__IDASIG__NAME__PAREN						0x02
//Symbol linked reference?
#define UVD__IDASIG__NAME__SYMBOL_LINKED_REFERENCE		0x04
//More lowest level nodes
#define UVD__IDASIG__NAME__MORE_BASIC					0x08
//More crc16 nodes
#define UVD__IDASIG__NAME__MORE_HASH					0x10
//Two of these in a row terminate a name
//If only one, it should be printed as part of the symbol name
#define UVD__IDASIG__NAME__ESCAPE_MAX					0x1F

/*
Header on the file
*/
struct UVD_IDA_sig_header_t
{
	char magic[6];
	uint8_t version;
	uint8_t processor;
	uint32_t file_types;
	uint16_t OS_types;
	uint16_t app_types;
	uint8_t feature_flags;
	char pad;
	uint16_t old_number_modules;
	uint16_t crc16;
	char ctype[0x22 - 0x16];
	uint8_t library_name_sz;
	uint16_t alt_ctype_crc;
	uint32_t n_modules;
}  __attribute__((__packed__));

std::string UVDIDASigFeaturesToString(uint32_t flags);
std::string UVDIDASigArchToString(uint32_t in);
std::string UVDIDASigOSToString(uint32_t flags);
std::string UVDIDASigApplicationToString(uint32_t flags);
std::string UVDIDASigFileToString(uint32_t flags);

#endif

