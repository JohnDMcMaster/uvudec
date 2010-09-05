/*
UVNet Utils (uvutils)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under GPL V3+, see COPYING for details

Original copyright below, from:
http://www.woodmann.com/forum/showthread.php?7517-IDA-signature-file-format

License: WTFPL
	In other words: public domain, do what you want with it
FLAIR kit documents .pat format
SDK has funcs.hpp with limited info about sigs and only some external API calls 
Written by Rheax <rheaxmascot@gmail.com>
*/

#include <ctype.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <string>
#include <stdint.h>


#define IDASIG__FEATURE__STARTUP				0x01
#define IDASIG__FEATURE__CTYPE_CRC				0x02
#define IDASIG__FEATURE__2BYTE_CTYPE			0x04
#define IDASIG__FEATURE__ALT_CTYPE_CRC			0x08
#define IDASIG__FEATURE__COMPRESSED				0x10

 
//"Intel 80x86"
#define IDASIG__ARCH__80X86						0x00
//"8085, Z80"
#define IDASIG__ARCH__Z80						0x01
//"Intel 860"
#define IDASIG__ARCH__INTEL_860					0x02
//"8051"
#define IDASIG__ARCH__8051						0x03
//"TMS320C5x"
#define IDASIG__ARCH__TMS320C5X					0x04
//"6502"
#define IDASIG__ARCH__6502						0x05
//"PDP11"
#define IDASIG__ARCH__PDP11						0x06
//"Motorola 680x0"
#define IDASIG__ARCH__MOTOROLA_680X0			0x07
//"Java"
#define IDASIG__ARCH__JAVA						0x08
//"Motorola 68xx"
#define IDASIG__ARCH__MOTOROLA_68XX				0x09
//"SGS-Thomson ST7"
#define IDASIG__ARCH__SGS_THOMSON_ST7			0x0A
//"Motorola 68HC12"
#define IDASIG__ARCH__MOTOROLA_68HC12			0x0B
//"MIPS"
#define IDASIG__ARCH__MIPS						0x0C
//"Advanced RISC Machines"
#define IDASIG__ARCH__ADVANCED_RISC				0x0D
//"TMS320C6x"
#define IDASIG__ARCH__TMS320C6X					0x0E
//"PowerPC"
#define IDASIG__ARCH__POWERPC					0x0F
//"Intel 80196"
#define IDASIG__ARCH__INTEL_80196				0x10
//"Z8"
#define IDASIG__ARCH__Z8						0x11
//"Hitachi SH"
#define IDASIG__ARCH__HITACHI_SH				0x12
//"Microsoft Visual Studio.Net"
#define IDASIG__ARCH__MSVS_DOT_NET				0x13
//"Atmel 8-bit RISC processor(s)"
#define IDASIG__ARCH__ATMEL_8_BIT_RISC			0x14
//"Hitachi H8/300, H8/2000"
#define IDASIG__ARCH__HITACHI_H8_300_H8_2000	0x15
//"Microchip's PIC"
#define IDASIG__ARCH__MICROCHIP_PIC				0x16
//"SPARC"
#define IDASIG__ARCH__SPARC						0x17
//"DEC Alpha"
#define IDASIG__ARCH__DEC_ALPHA					0x18
//"Hewlett-Packard PA-RISC"
#define IDASIG__ARCH__HP_PA_RISC				0x19
//"Hitachi H8/500"
#define IDASIG__ARCH__HITACHI_H8_500			0x1A
//"Tasking Tricore"
#define IDASIG__ARCH__TASKING_TRICORE			0x1B
//"Motorola DSP5600x"
#define IDASIG__ARCH__MOTOROLA_DSP5600X			0x1C
//"Siemens C166 family"
#define IDASIG__ARCH__SIEMENS_C166				0x1D
//"SGS-Thomson ST20"
#define IDASIG__ARCH__SGS_THOMSON_ST20			0x1E
//"Intel Itanium IA64"
#define IDASIG__ARCH__INTEL_ITANIUM_IA64		0x1F
//"Intel 960"
#define IDASIG__ARCH__INTEL_I960				0x20
//"Fujistu F2MC-16"
#define IDASIG__ARCH__FUJITSU_F2MC_16			0x21
//"TMS320C54xx"
#define IDASIG__ARCH__TMS320C54XX				0x22
//"TMS320C55xx"
#define IDASIG__ARCH__TMS320C55XX				0x23
//"Trimedia"
#define IDASIG__ARCH__TRIMEDIA					0x24
//"Mitsubishi 32bit RISC"
#define IDASIG__ARCH__MITSUBISH_32_BIT_RISC		0x25
//"NEC 78K0"
#define IDASIG__ARCH__NEC_78K0					0x26
//"NEC 78K0S"
#define IDASIG__ARCH__NEC_78K0S					0x27
//"Mitsubishi 8bit"
#define IDASIG__ARCH__MITSUBISHI_8_BIT			0x28
//"Mitsubishi 16bit"
#define IDASIG__ARCH__MITSIBUSHI_16_BIT			0x29
//"ST9+"
#define IDASIG__ARCH__ST9PLUS					0x2A
//"Fujitsu FR Family"
#define IDASIG__ARCH__FUJITSU_FR				0x2B
//"Motorola 68HC16"
#define IDASIG__ARCH__MOTOROLA_68HC16			0x2C
//"Mitsubishi 7900"
#define IDASIG__ARCH__MITSUBISHI_7900			0x2D


#define IDASIG__OS__MSDOS						0x01
#define IDASIG__OS__WIN							0x02
#define IDASIG__OS__OS2							0x04
#define IDASIG__OS__NETWARE						0x08
#define IDASIG__OS__UNIX						0x10


#define IDASIG__APP__CONSOLE					0x0001
#define IDASIG__APP__GRAPHICS					0x0002
#define IDASIG__APP__EXE						0x0004
#define IDASIG__APP__DLL						0x0008
#define IDASIG__APP__DRV						0x0010
#define IDASIG__APP__SINGLE_THREADED			0x0020
#define IDASIG__APP__MULTI_THREADED				0x0040
#define IDASIG__APP__16_BIT						0x0080
#define IDASIG__APP__32_BIT						0x0100
#define IDASIG__APP__64_BIT						0x0200


#define IDASIG__FILE__DOS_EXE_OLD				0x00000001
#define IDASIG__FILE__DOS_COM_OLD				0x00000002
#define IDASIG__FILE__BIN						0x00000004
#define IDASIG__FILE__DOSDRV					0x00000008
#define IDASIG__FILE__NE						0x00000010
#define IDASIG__FILE__INTELHEX					0x00000020
#define IDASIG__FILE__MOSHEX					0x00000040
#define IDASIG__FILE__LX						0x00000080
#define IDASIG__FILE__LE						0x00000100
#define IDASIG__FILE__NLM						0x00000200
#define IDASIG__FILE__COFF						0x00000400
#define IDASIG__FILE__PE						0x00000800
#define IDASIG__FILE__OMF						0x00001000
#define IDASIG__FILE__SREC						0x00002000
#define IDASIG__FILE__ZIP						0x00004000
#define IDASIG__FILE__OMFLIB					0x00008000
#define IDASIG__FILE__AR						0x00010000
#define IDASIG__FILE__LOADER					0x00020000
#define IDASIG__FILE__ELF						0x00040000
#define IDASIG__FILE__W32RUN					0x00080000
#define IDASIG__FILE__AOUT						0x00100000
#define IDASIG__FILE__PILOT						0x00200000
#define IDASIG__FILE__DOS_EXE					0x00400000
#define IDASIG__FILE__AIXAR						0x00800000

#define printf_indented(format, ...) printf("%s" format, g_indent.c_str(), ##__VA_ARGS__)
#define err(format, ...) do{printf("ERROR (%s:%d): " format, __FILE__, __LINE__, ## __VA_ARGS__); exit(1);}while(0)

struct sig_header_t
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

std::string g_indent;
uint32_t g_file_pos = 0;	
char *g_file_contents = NULL;
char *g_cur_ptr = NULL;
unsigned int g_file_size = 0;

#define __stringify_1(x)	#x
#define __stringify(x)		__stringify_1(x)
#define FLAG_STRING(flag, str)			if (flags & flag) { if(!ret.empty()) ret += " "; ret += str; }
#define FLAG_STRING_RAW(flag)			FLAG_STRING(flag, __stringify(flag))
#define CASE_STRING(define, str)		if (in == define) return str;

std::string IDASigFeaturesToString(uint32_t flags)
{
	std::string ret;

	FLAG_STRING(IDASIG__FEATURE__STARTUP, "STARTUP");
	FLAG_STRING(IDASIG__FEATURE__CTYPE_CRC, "CTYPE_CRC");
	FLAG_STRING(IDASIG__FEATURE__2BYTE_CTYPE, "2BYTE_CTYPE");
	FLAG_STRING(IDASIG__FEATURE__ALT_CTYPE_CRC, "ALT_CTYPE_CRC");
	FLAG_STRING(IDASIG__FEATURE__COMPRESSED, "COMPRESSED");

	return ret;
}

std::string IDASigArchToString(uint32_t in)
{
	CASE_STRING(IDASIG__ARCH__80X86, "80X86");
	CASE_STRING(IDASIG__ARCH__Z80, "Z80");
	CASE_STRING(IDASIG__ARCH__INTEL_860, "INTEL_860");
	CASE_STRING(IDASIG__ARCH__8051, "8051");
	CASE_STRING(IDASIG__ARCH__TMS320C5X, "TMS320C5X");
	CASE_STRING(IDASIG__ARCH__6502, "6502");
	CASE_STRING(IDASIG__ARCH__PDP11, "PDP11");
	CASE_STRING(IDASIG__ARCH__MOTOROLA_680X0, "MOTOROLA_680X0");
	CASE_STRING(IDASIG__ARCH__JAVA, "JAVA");
	CASE_STRING(IDASIG__ARCH__MOTOROLA_68XX, "MOTOROLA_68XX");
	CASE_STRING(IDASIG__ARCH__SGS_THOMSON_ST7, "SGS_THOMSON_ST7");
	CASE_STRING(IDASIG__ARCH__MOTOROLA_68HC12, "MOTOROLA_68HC12");
	CASE_STRING(IDASIG__ARCH__MIPS, "MIPS");
	CASE_STRING(IDASIG__ARCH__ADVANCED_RISC, "ADVANCED_RISC");
	CASE_STRING(IDASIG__ARCH__TMS320C6X, "TMS320C6X");
	CASE_STRING(IDASIG__ARCH__POWERPC, "POWERPC");
	CASE_STRING(IDASIG__ARCH__INTEL_80196, "INTEL_80196");
	CASE_STRING(IDASIG__ARCH__Z8, "Z8");
	CASE_STRING(IDASIG__ARCH__HITACHI_SH, "HITACHI_SH");
	CASE_STRING(IDASIG__ARCH__MSVS_DOT_NET, "MSVS_DOT_NET");
	CASE_STRING(IDASIG__ARCH__ATMEL_8_BIT_RISC, "ATMEL_8_BIT_RISC");
	CASE_STRING(IDASIG__ARCH__HITACHI_H8_300_H8_2000, "HITACHI_H8_300_H8_2000");
	CASE_STRING(IDASIG__ARCH__MICROCHIP_PIC, "MICROCHIP_PIC");
	CASE_STRING(IDASIG__ARCH__SPARC, "SPARC");
	CASE_STRING(IDASIG__ARCH__DEC_ALPHA, "DEC_ALPHA");
	CASE_STRING(IDASIG__ARCH__HP_PA_RISC, "HP_PA_RISC");
	CASE_STRING(IDASIG__ARCH__HITACHI_H8_500, "HITACHI_H8_500");
	CASE_STRING(IDASIG__ARCH__TASKING_TRICORE, "TASKING_TRICORE");
	CASE_STRING(IDASIG__ARCH__MOTOROLA_DSP5600X, "MOTOROLA_DSP5600X");
	CASE_STRING(IDASIG__ARCH__SIEMENS_C166, "SIEMENS_C166");
	CASE_STRING(IDASIG__ARCH__SGS_THOMSON_ST20, "SGS_THOMSON_ST20");
	CASE_STRING(IDASIG__ARCH__INTEL_ITANIUM_IA64, "INTEL_ITANIUM_IA64");
	CASE_STRING(IDASIG__ARCH__INTEL_I960, "INTEL_I960");
	CASE_STRING(IDASIG__ARCH__FUJITSU_F2MC_16, "FUJITSU_F2MC_16");
	CASE_STRING(IDASIG__ARCH__TMS320C54XX, "TMS320C54XX");
	CASE_STRING(IDASIG__ARCH__TMS320C55XX, "TMS320C55XX");
	CASE_STRING(IDASIG__ARCH__TRIMEDIA, "TRIMEDIA");
	CASE_STRING(IDASIG__ARCH__MITSUBISH_32_BIT_RISC, "MITSUBISH_BIT_RISC");
	CASE_STRING(IDASIG__ARCH__NEC_78K0, "NEC_78K0");
	CASE_STRING(IDASIG__ARCH__NEC_78K0S, "NEC_78K0S");
	CASE_STRING(IDASIG__ARCH__MITSUBISHI_8_BIT, "MITSUBISHI_8_BIT");
	CASE_STRING(IDASIG__ARCH__MITSIBUSHI_16_BIT, "MITSIBUSHI_16_BIT");
	CASE_STRING(IDASIG__ARCH__ST9PLUS, "ST9PLUS");
	CASE_STRING(IDASIG__ARCH__FUJITSU_FR, "FUJITSU_FR");
	CASE_STRING(IDASIG__ARCH__MOTOROLA_68HC16, "MOTOROLA_68HC16");
	CASE_STRING(IDASIG__ARCH__MITSUBISHI_7900, "MITSUBISHI_7900");
	return "UNKNOWN";
}

std::string IDASigOSToString(uint32_t flags)
{
	std::string ret;

	FLAG_STRING(IDASIG__OS__MSDOS, "MSDOS");
	FLAG_STRING(IDASIG__OS__WIN, "WIN");
	FLAG_STRING(IDASIG__OS__OS2, "OS2");
	FLAG_STRING(IDASIG__OS__NETWARE, "NETWARE");
	FLAG_STRING(IDASIG__OS__UNIX, "UNIX");

	return ret;
}

std::string IDASigApplicationToString(uint32_t flags)
{
	std::string ret;

	FLAG_STRING(IDASIG__APP__CONSOLE, "CONSOLE");
	FLAG_STRING(IDASIG__APP__GRAPHICS, "GRAPHICS");
	FLAG_STRING(IDASIG__APP__EXE, "EXE");
	FLAG_STRING(IDASIG__APP__DLL, "DLL");
	FLAG_STRING(IDASIG__APP__DRV, "DRV");
	FLAG_STRING(IDASIG__APP__SINGLE_THREADED, "SINGLE_THREADED");
	FLAG_STRING(IDASIG__APP__MULTI_THREADED, "MULTI_THREADED");
	FLAG_STRING(IDASIG__APP__16_BIT, "16_BIT");
	FLAG_STRING(IDASIG__APP__32_BIT, "32_BIT");
	FLAG_STRING(IDASIG__APP__64_BIT, "64_BIT");

	return ret;
}

std::string IDASigFileToString(uint32_t flags)
{
	std::string ret;

	FLAG_STRING(IDASIG__FILE__DOS_EXE_OLD, "DOS_EXE_OLD");
	FLAG_STRING(IDASIG__FILE__DOS_COM_OLD, "DOS_COM_OLD");
	FLAG_STRING(IDASIG__FILE__BIN, "BIN");
	FLAG_STRING(IDASIG__FILE__DOSDRV, "DOSDRV");
	FLAG_STRING(IDASIG__FILE__NE, "NE");
	FLAG_STRING(IDASIG__FILE__INTELHEX, "INTELHEX");
	FLAG_STRING(IDASIG__FILE__MOSHEX, "MOSHEX");
	FLAG_STRING(IDASIG__FILE__LX, "LX");
	FLAG_STRING(IDASIG__FILE__LE, "LE");
	FLAG_STRING(IDASIG__FILE__NLM, "NLM");
	FLAG_STRING(IDASIG__FILE__COFF, "COFF");
	FLAG_STRING(IDASIG__FILE__PE, "PE");
	FLAG_STRING(IDASIG__FILE__OMF, "OMF");
	FLAG_STRING(IDASIG__FILE__SREC, "SREC");
	FLAG_STRING(IDASIG__FILE__ZIP, "ZIP");
	FLAG_STRING(IDASIG__FILE__OMFLIB, "OMFLIB");
	FLAG_STRING(IDASIG__FILE__AR, "AR");
	FLAG_STRING(IDASIG__FILE__LOADER, "LOADER");
	FLAG_STRING(IDASIG__FILE__ELF, "ELF");
	FLAG_STRING(IDASIG__FILE__W32RUN, "W32RUN");
	FLAG_STRING(IDASIG__FILE__AOUT, "AOUT");
	FLAG_STRING(IDASIG__FILE__PILOT, "PILOT");
	FLAG_STRING(IDASIG__FILE__DOS_EXE, "EXE");
	FLAG_STRING(IDASIG__FILE__AIXAR, "AIXAR");

	return ret;
}

void inc_indent()
{
	g_indent += "  "; 
}

void dec_indent()
{
	g_indent.erase(g_indent.begin());
	g_indent.erase(g_indent.begin());
}

std::string hexstr(const char *in, int sz)
{
	std::string ret;
	for (int i = 0; i < sz; ++i) {
		char buff[3];
		sprintf(buff, "%.2X", in[i]);
		ret += buff;
	}
	return ret;
}

std::string safestr(const char *in, int sz)
{
	std::string ret;
	
	for (int i = 0; i < sz; ++i) {
		if (in[i] == 0)
			break;
		if (isprint(in[i]))
			ret += in[i];
		else
			ret += '.';
	}
	return ret;
}

void init(const char *in)
{
	FILE *file = NULL;
	struct stat astat;

	file = fopen(in, "rb");
	if (!file)
		err("file not found\n");

	if (stat(in, &astat))
		err("no size\n");
	
	g_file_size = astat.st_size;
	g_file_contents = (char *)malloc(g_file_size);
	
	if (!g_file_contents)
		err("alloc fail\n");

	if (fread(g_file_contents, 1, g_file_size, file) != g_file_size)
		err("bad file read\n");
	g_cur_ptr = g_file_contents;

	fclose(file);
}

void decompress()
{
	err("ZIP decompression not supported\n");
	exit(1);
}

void advance(int bytes)
{
	g_file_pos += bytes;
	g_cur_ptr += bytes;
}

int read_byte()
{
	uint8_t ret = *g_cur_ptr;
	advance(1);
	return ret;
}

int read16()
{
	return (read_byte() << 8)
		+ read_byte();
}

int bitshift_read()
{
	uint32_t first = read_byte();
	
	if ( first & 0x80)
		return ((first & 0x7F) << 8) + read_byte();
	return first;
}

int read_relocation_bitmask()
{
	uint32_t first;
	uint32_t lower;
	uint32_t upper;

	first = read_byte();
	
	if ((first & 0x80) != 0x80)
		return first;
	
	if ((first & 0xC0) != 0xC0)
		return ((first & 0x7F) << 8) + read_byte();
	
	if ((first & 0xE0) != 0xE0) {
		upper = ((first & 0xFF3F) << 8) + read_byte();
		lower = read16();
	} else {
		upper = read16();
		lower = read16();
	}
	uint32_t ret = lower + (upper << 16);
	return ret;
}

void dump_tree()
{
	uint16_t n_internal_nodes = bitshift_read();

	//Internal node
	if (n_internal_nodes) {
		uint32_t relocation_bitmask;

		//Simply all of the ones with the same prefix
		for (int i = 0; i < n_internal_nodes; ++i) {
			uint32_t n_node_bytes;
			uint32_t cur_relocation_bitmask;
	
			n_node_bytes = read_byte();
			//Only allowed 32 bytes
			if (n_node_bytes > 0x20u)e
				err("Too many bytes\n");

			cur_relocation_bitmask = 1 << (n_node_bytes - 1);

			if (n_node_bytes >= 0x10)
				relocation_bitmask = read_relocation_bitmask();
			else
				relocation_bitmask = bitshift_read();

			//Relocations don't appear until the end
			printf_indented("");
			for (uint32_t j = 0; j < n_node_bytes; ++j) {
				if ( cur_relocation_bitmask & relocation_bitmask)
					printf("..");
				else
					printf("%.2X", read_byte());
				cur_relocation_bitmask >>= 1;
			}
			printf(":\n");
			inc_indent();
			dump_tree();
			dec_indent();
		}
	//Leaf node
	} else {
		uint32_t read_flags;
		uint32_t func_index = 0;
		//Loop for each element with the same prefix, but possibly different crc16
		//Listed in increasing sorted order of crc16
		do {
			uint32_t tree_block_len = read_byte();
			uint32_t a_crc16 = read16();
			//Loop for each bucketed signature
			//All in this loop have the same crc16 and same length
			//What may be different is what the relocation symbols are
			do {
				uint32_t total_len;
				uint32_t ref_cur_offset = 0;
								
				total_len = bitshift_read();
				printf_indented("%d. tree_block_len:0x%.2X a_crc16:0x%.4X total_len:0x%.4X", func_index, tree_block_len, a_crc16, total_len);
				++func_index;
			
				//Loop for each reference
				do {
					std::string name;
					uint32_t delta = 0;
					bool has_negative;
					
					delta = bitshift_read();
				
					read_flags = read_byte();
					//whys neg ref useful?
					has_negative = read_flags < 0x20;
					
					//Read reference name
					for (int i = 0; ; ++i) {
						if (i >= 1024)
							err("reference length exceeded\n");
					
						if ( read_flags < 0x20)
							read_flags = read_byte();
						if ( read_flags < 0x20)
							break;
				
						name += (char)read_flags;
						read_flags = 0;
					}
					ref_cur_offset += delta;
					if (ref_cur_offset == 0)
						printf(" ");
					printf(" %.4X:%s", ref_cur_offset, name.c_str());					
				} while (read_flags & 1);
				
				//Not sure what this is
				if (read_flags & 2) {
					uint32_t first;
					uint32_t second;
				
					first = bitshift_read();
					second = read_byte();
					printf(" (0x%.4X: 0x%.2X)", first, second);
				}
				
				//Symbol linked references
				if (read_flags & 4) {
					uint32_t a_offset;
					std::string ref_name;
					uint32_t ref_name_len;
							
					a_offset = bitshift_read();
					ref_name_len = read_byte();
					if (!ref_name_len)
						ref_name_len = bitshift_read();
					
					ref_name = std::string(g_cur_ptr, ref_name_len);
					//If last char is 0, we have a special flag set
					if (g_cur_ptr[ref_name_len - 1] == 0)
						a_offset = -a_offset;
					advance(ref_name_len);
				}
				printf("\n");
			} while (read_flags & 0x08);
		} while (read_flags & 0x10);
	}
}

int main(int argc, char **argv)
{
	struct sig_header_t *header = NULL;

	if (argc < 2)
		err("usage: sigread [signature file]\n");
	init(argv[1]);
	
	printf("File size: 0x%.8X (%d)\n", g_file_size, g_file_size);
	
	header = (struct sig_header_t *)g_file_contents;
	advance(sizeof(struct sig_header_t));
	
	printf("magic: %s\n", hexstr(header->magic, sizeof(header->magic)).c_str());
	if (memcmp(header, "IDASGN", 6))
		err("magic fail\n");
	
	printf("version: %d\n", header->version);
	/*
	if (header->version == 5)
		numberModules = oldNumberModules;
	Maybe some differences when using ver 6
	Ver 5 issues apply as well
	*/
	if (header->version != 7)
		err("version mismatch\n");
		
	printf("last (n_module) offest: 0x%.8X\n", offsetof(struct sig_header_t, n_modules));
	if (sizeof(struct sig_header_t) != 0x29)
		err("sig_header_t wrong size: 0x%.8X\n", sizeof(struct sig_header_t));
		
	printf("processor: %s (0x%.2X)\n", IDASigArchToString(header->processor).c_str(), header->processor);
	printf("file_types: %s (0x%.8X)\n", IDASigFileToString(header->file_types).c_str(), header->file_types);
	printf("OS_types: %s (0x%.4X)\n", IDASigOSToString(header->OS_types).c_str(), header->OS_types);
	printf("app_types: %s (0x%.4X)\n", IDASigApplicationToString(header->app_types).c_str(), header->app_types);
	printf("feature_flags: %s (0x%.2X)\n", IDASigFeaturesToString(header->feature_flags).c_str(), header->feature_flags);
	printf("unknown (pad): 0x%.2X\n", header->pad);
	printf("old_number_modules: 0x%.4X\n", header->old_number_modules);
	printf("crc16: 0x%.4X\n", header->crc16);	
	//Make sure its null terminated
	printf("ctype: %s\n", safestr(header->ctype, sizeof(header->ctype)).c_str());	
	printf("library_name_sz: 0x%.2X\n", header->library_name_sz);	
	printf("alt_ctype_crc: 0x%.4X\n", header->alt_ctype_crc);	
	printf("n_modules: 0x%.8X (%d)\n", header->n_modules, header->n_modules);

	//Name is immediatly after header
	char library_name[256];
	memcpy(&library_name[0], g_cur_ptr, header->library_name_sz);
	library_name[header->library_name_sz] = 0;
	advance(header->library_name_sz);
	printf("library name: %s\n", library_name);
		
	if (header->feature_flags & IDASIG__FEATURE__COMPRESSED)
		decompress();
	
	dump_tree();
		
	return 0;
}

