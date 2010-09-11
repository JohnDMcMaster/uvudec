/*
UVNet Utils (uvutils)
uvsigutil: IDA .sig format utilities
In particular, compression, decompression, and dumping
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under GPL V3+, see COPYING for details

Some code taken from
http://www.woodmann.com/forum/showthread.php?7517-IDA-signature-file-format
*/

#include "flirt.h"
#include "flirt/sig/format.h"
#include "uvd_arg_property.h"
#include "uvd_init.h"
#include <ctype.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <string>
#include <stdint.h>

#define ACTION_NONE				0
#define ACTION_DECOMPRESS		1
#define ACTION_COMPRESS			2
#define ACTION_DUMP				3

#define PROP_ACTION_DECOMPRESS			"action.decompress"
#define PROP_ACTION_COMPRESS			"action.compress"
#define PROP_ACTION_DUMP				"action.dump"

uint32_t g_action = ACTION_NONE;

#define printf_indented(format, ...) printf("%s" format, g_indent.c_str(), ##__VA_ARGS__)
#define err(format, ...) do{printf("ERROR (%s:%d): " format, __FILE__, __LINE__, ## __VA_ARGS__); exit(1);}while(0)

std::string g_indent;
uint32_t g_file_pos = 0;	
char *g_file_contents = NULL;
char *g_cur_ptr = NULL;
unsigned int g_file_size = 0;

#define __stringify_1(x)	#x
#define __stringify(x)		__stringify_1(x)
#define FLAG_STRING(flag, str)			if( flags & flag) { if(!ret.empty()) ret += " "; ret += str; }
#define FLAG_STRING_RAW(flag)			FLAG_STRING(flag, __stringify(flag))
#define CASE_STRING(define, str)		if( in == define) return str;


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
	for( int i = 0; i < sz; ++i) {
		char buff[3];
		sprintf(buff, "%02X", in[i]);
		ret += buff;
	}
	return ret;
}

std::string safestr(const char *in, int sz)
{
	std::string ret;
	
	for( int i = 0; i < sz; ++i) {
		if( in[i] == 0)
			break;
		if( isprint(in[i]))
			ret += in[i];
		else
			ret += '.';
	}
	return ret;
}

uv_err_t dumpInit(const std::string &in)
{
	FILE *file = NULL;
	struct stat astat;

	file = fopen(in.c_str(), "rb");
	if( !file )
	{
		printf_error("file not found\n");
		return UV_DEBUG(UV_ERR_GENERAL);
	}

	if( stat(in.c_str(), &astat) )
	{
		printf_error("no size\n");
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	
	g_file_size = astat.st_size;
	g_file_contents = (char *)malloc(g_file_size);
	
	if( !g_file_contents )
	{
		printf_error("alloc fail\n");
		return UV_DEBUG(UV_ERR_GENERAL);
	}

	if( fread(g_file_contents, 1, g_file_size, file) != g_file_size )
	{
		printf_error("bad file read\n");
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	g_cur_ptr = g_file_contents;

	fclose(file);

	return UV_ERR_OK;
}

uv_err_t decompress()
{
	//website says something about InfoZIP algorithm
	printf_error("ZIP decompression not supported\n");
	return UV_DEBUG(UV_ERR_GENERAL);
}

uv_err_t advance(int bytes)
{
	g_file_pos += bytes;
	g_cur_ptr += bytes;
	return UV_ERR_OK;
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
	
	if( first & 0x80 )
		return ((first & 0x7F) << 8) + read_byte();
	return first;
}

int read_relocation_bitmask()
{
	uint32_t first = 0;
	uint32_t lower = 0;
	uint32_t upper = 0;

	first = read_byte();
	printf_flirt_debug("first byte: 0x%02X\n", first);
	
	//No bit prefix
	//Max val 0x7F
	if( (first & 0x80) != 0x80 )
	{
		return first;
	}
	//0x80 bit prefix
	//Max val 0x7FFF
	else if( (first & 0xC0) != 0xC0 )
	{
		//0x7F trims off the 0x80 escape bit
		return ((first & 0x7F) << 8) + read_byte();
	}
	//0xC0 bit prefix
	//Max val 0x3FFFFFFF
	else if( (first & 0xE0) != 0xE0 )
	{
		upper = ((first & 0x3F) << 8) + read_byte();
		lower = read16();
	}
	//0xE0 bit prefix
	//Max val 0xFFFFFFFF
	//NOTE: 0xF0 etc seems reserved
	else
	{
		upper = read16();
		lower = read16();
	}
	printf_flirt_debug("upper: 0x%02X, lower: 0x%02X\n", upper, lower);
	uint32_t ret = lower + (upper << 16);
	return ret;
}

uv_err_t dump_tree()
{
	uint16_t n_internal_nodes = bitshift_read();

	printf_flirt_debug("n_internal_nodes: 0x%04X\n", n_internal_nodes);
	//Internal node
	if( n_internal_nodes )
	{
		uint32_t relocation_bitmask;

		//Simply all of the ones with the same prefix
		for( int i = 0; i < n_internal_nodes; ++i )
		{
			uint32_t n_node_bytes = 0;
			uint32_t cur_relocation_bitmask = 0;
	
			n_node_bytes = read_byte();
			printf_flirt_debug("n_node_bytes: 0x%02X\n", n_node_bytes);
			//Only allowed 32 bytes
			if( n_node_bytes > 0x20u)
			{
				printf_error("Too many bytes, leading max 0x20, found 0x%02X\n", n_node_bytes);
				return UV_DEBUG(UV_ERR_GENERAL);
			}

			cur_relocation_bitmask = 1 << (n_node_bytes - 1);

			if( n_node_bytes >= 0x10 )
			{
				relocation_bitmask = read_relocation_bitmask();
			}
			else
			{
				relocation_bitmask = bitshift_read();
			}
			printf_flirt_debug("relocation_bitmask: 0x%08X\n", n_internal_nodes);

			//Relocations don't appear until the end
			printf_indented("");
			for( uint32_t j = 0; j < n_node_bytes; ++j )
			{
				if( cur_relocation_bitmask & relocation_bitmask )
				{
					printf("..");
				}
				else
				{
					printf("%02X", read_byte());
				}
				cur_relocation_bitmask >>= 1;
			}
			printf(":\n");
			inc_indent();
			uv_assert_err_ret(dump_tree());
			dec_indent();
		}
	}
	//Leaf node
	else
	{
		uint32_t read_flags;
		uint32_t func_index = 0;
		//Loop for each element with the same prefix, but possibly different crc16
		//Listed in increasing sorted order of crc16
		do
		{
			uint32_t tree_block_len = read_byte();
			uint32_t a_crc16 = read16();
			//Loop for each bucketed signature
			//All in this loop have the same crc16 and same length
			//What may be different is what the relocation symbols are
			do
			{
				uint32_t total_len;
				uint32_t ref_cur_offset = 0;
								
				total_len = bitshift_read();
				printf_indented("%d. tree_block_len:0x%02X a_crc16:0x%04X total_len:0x%04X", func_index, tree_block_len, a_crc16, total_len);
				++func_index;
			
				//Loop for each reference
				do
				{
					std::string name;
					uint32_t delta = 0;
					bool has_negative;
					
					delta = bitshift_read();
				
					read_flags = read_byte();
					//whys neg ref useful?
					has_negative = read_flags < 0x20;
					
					//Read reference name
					for( int i = 0; ; ++i )
					{
						if( i >= 1024 )
						{
							printf_error("reference length exceeded\n");
							return UV_DEBUG(UV_ERR_GENERAL);
						}
					
						if( read_flags < 0x20 )
						{
							read_flags = read_byte();
						}
						if( read_flags < 0x20 )
						{
							break;
						}
				
						name += (char)read_flags;
						read_flags = 0;
					}
					ref_cur_offset += delta;
					/*
					if( ref_cur_offset == 0 )
					{
						printf(" ");
					}
					*/
					printf(" %04X:%s", ref_cur_offset, name.c_str());					
				} while( read_flags & 1 );
				
				//Not sure what this is
				if( read_flags & 2)
				{
					uint32_t first;
					uint32_t second;
				
					first = bitshift_read();
					second = read_byte();
					printf(" (0x%04X: 0x%02X)", first, second);
				}
				
				//Symbol linked references
				if( read_flags & 4)
				{
					uint32_t a_offset;
					std::string ref_name;
					uint32_t ref_name_len;
							
					a_offset = bitshift_read();
					ref_name_len = read_byte();
					if( !ref_name_len)
						ref_name_len = bitshift_read();
					
					ref_name = std::string(g_cur_ptr, ref_name_len);
					//If last char is 0, we have a special flag set
					if( g_cur_ptr[ref_name_len - 1] == 0)
						a_offset = -a_offset;
					uv_assert_err_ret(advance(ref_name_len));
				}
				printf("\n");
			} while( read_flags & 0x08 );
		} while( read_flags & 0x10 );
	}
	
	return UV_ERR_OK;
}

static uv_err_t dumpFile(const std::string &fileName)
{
	struct UVD_IDA_sig_header_t *header = NULL;

	uv_assert_err_ret(dumpInit(fileName));
	
	printf("File size: 0x%08X (%d)\n", g_file_size, g_file_size);
	
	header = (struct UVD_IDA_sig_header_t *)g_file_contents;
	uv_assert_err_ret(advance(sizeof(struct UVD_IDA_sig_header_t)));
	
	printf("magic: %s\n", hexstr(header->magic, sizeof(header->magic)).c_str());
	if( memcmp(header, "IDASGN", 6) )
	{
		printf_error("magic fail\n");
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	
	printf("version: %d\n", header->version);
	/*
	if( header->version == 5)
		numberModules = oldNumberModules;
	Maybe some differences when using ver 6
	Ver 5 issues apply as well
	*/
	if( header->version != 7)
	{
		printf_error("version mismatch\n");
		return UV_DEBUG(UV_ERR_GENERAL);
	}
		
	printf("last (n_module) offest: 0x%08X\n", offsetof(struct UVD_IDA_sig_header_t, n_modules));
	if( sizeof(struct UVD_IDA_sig_header_t) != 0x29)
	{
		printf_error("UVD_IDA_sig_header_t wrong size: 0x%08X\n", sizeof(struct UVD_IDA_sig_header_t));
		return UV_DEBUG(UV_ERR_GENERAL);
	}
		
	printf("processor: %s (0x%02X)\n", UVDIDASigArchToString(header->processor).c_str(), header->processor);
	printf("file_types: %s (0x%08X)\n", UVDIDASigFileToString(header->file_types).c_str(), header->file_types);
	printf("OS_types: %s (0x%04X)\n", UVDIDASigOSToString(header->OS_types).c_str(), header->OS_types);
	printf("app_types: %s (0x%04X)\n", UVDIDASigApplicationToString(header->app_types).c_str(), header->app_types);
	printf("feature_flags: %s (0x%02X)\n", UVDIDASigFeaturesToString(header->feature_flags).c_str(), header->feature_flags);
	printf("unknown (pad): 0x%02X\n", header->pad);
	printf("old_number_modules: 0x%04X\n", header->old_number_modules);
	printf("crc16: 0x%04X\n", header->crc16);	
	//Make sure its null terminated
	printf("ctype: %s\n", safestr(header->ctype, sizeof(header->ctype)).c_str());	
	printf("library_name_sz: 0x%02X\n", header->library_name_sz);	
	printf("alt_ctype_crc: 0x%04X\n", header->alt_ctype_crc);	
	printf("n_modules: 0x%08X (%d)\n", header->n_modules, header->n_modules);

	//Name is immediatly after header
	char library_name[256];
	memcpy(&library_name[0], g_cur_ptr, header->library_name_sz);
	library_name[header->library_name_sz] = 0;
	uv_assert_err_ret(advance(header->library_name_sz));
	printf("library name: %s\n", library_name);
		
	printf_flirt_debug("Root node at 0x%08X\n", g_file_pos);
	if( header->feature_flags & UVD__IDASIG__FEATURE__COMPRESSED)
	{
		uv_assert_err_ret(decompress());
	}
	
	uv_assert_err_ret(dump_tree());
		
	return UV_ERR_OK;
}

std::vector<std::string> g_inputFiles;

static uv_err_t argParser(const UVDArgConfig *argConfig, std::vector<std::string> argumentArguments)
{
	//If present
	std::string firstArg;
	uint32_t firstArgNum = 0;
	
	uv_assert_ret(g_config);
	uv_assert_ret(g_config->m_argv);
	uv_assert_ret(argConfig);

	if( !argumentArguments.empty() )
	{
		firstArg = argumentArguments[0];
		firstArgNum = strtol(firstArg.c_str(), NULL, 0);
	}

	if( argConfig->m_propertyForm == UVD_PROP_TARGET_FILE )
	{
		uv_assert_ret(!argumentArguments.empty());
		g_inputFiles.push_back(firstArg);
	}
	else if( argConfig->m_propertyForm == PROP_ACTION_DECOMPRESS )
	{
		g_action = ACTION_DECOMPRESS;
	}
	else if( argConfig->m_propertyForm == PROP_ACTION_COMPRESS )
	{
		g_action = ACTION_COMPRESS;
	}
	else if( argConfig->m_propertyForm == PROP_ACTION_DUMP )
	{
		g_action = ACTION_DUMP;
	}
	else
	{
		//return UV_DEBUG(argParserDefault(argConfig, argumentArguments));
		return UV_DEBUG(UV_ERR_GENERAL);
	}

	return UV_ERR_OK;
}

uv_err_t versionPrintPrefixThunk()
{
	const char *program_name = "flirtutil";
	
	printf_help("%s version %s\n", program_name, UVUDEC_VER_STRING);
	return UV_ERR_OK;
}

uv_err_t initProgConfig()
{
	uv_assert_err_ret(g_config->registerArgument(UVD_PROP_TARGET_FILE, 0, "input", "source file for data", 1, argParser, false));
	uv_assert_err_ret(g_config->registerArgument(PROP_ACTION_DECOMPRESS, 0, "decompress", "decompress input files, placing in output (default: stdout)", 0, argParser, false));
	uv_assert_err_ret(g_config->registerArgument(PROP_ACTION_COMPRESS, 0, "compress", "compress input files, overwritting them unless output specified", 0, argParser, false));
	uv_assert_err_ret(g_config->registerArgument(PROP_ACTION_DUMP, 0, "dump", "dump input files, placing in output (default: stdout)", 0, argParser, false));

	//Callbacks
	g_config->versionPrintPrefixThunk = versionPrintPrefixThunk;

	return UV_ERR_OK;	
}

uv_err_t uvmain(int argc, char **argv)
{
	uv_err_t rc = UV_ERR_GENERAL;
	UVDConfig *config = NULL;
	uv_err_t parseMainRc = UV_ERR_GENERAL;
	
	if( strcmp(UVUDEC_VER_STRING, UVDGetVersion()) )
	{
		printf_warn("libuvudec version mismatch (exe: %s, libuvudec: %s)\n", UVUDEC_VER_STRING, UVDGetVersion());
		fflush(stdout);
	}
	
	//Early library initialization.  Logging and arg parsing structures
	uv_assert_err_ret(UVDInit());
	config = g_config;
	uv_assert_ret(config);
	//Early local initialization
	uv_assert_err_ret(initProgConfig());
	
	//Grab our command line options
	parseMainRc = config->parseMain(argc, argv);
	uv_assert_err_ret(parseMainRc);
	if( parseMainRc == UV_ERR_DONE )
	{
		rc = UV_ERR_OK;
		goto error;
	}
	
	if( g_action == ACTION_DECOMPRESS )
	{
		printf_error("decompress unsupported\n");
		goto error;
	}
	else if( g_action == ACTION_COMPRESS )
	{
		printf_error("compress unsupported\n");
		goto error;
	}
	else if( g_action == ACTION_DUMP )
	{
		for( std::vector<std::string>::iterator iter = g_inputFiles.begin(); iter != g_inputFiles.end(); ++iter )
		{
			std::string file = *iter;
			
			uv_assert_err_ret(dumpFile(file));
		}
	}
	else if( g_action == ACTION_NONE )
	{
		printf_error("target action not specified\n");
		UVDHelp();
	}
	else
	{
		printf_error("confused\n");
		goto error;
	}

	rc = UV_ERR_OK;

error:
	uv_assert_err_ret(UVDDeinit());
		
	return UV_DEBUG(rc);
}

int main(int argc, char **argv)
{
	//Simple translation to keep most stuff in the framework
	uv_err_t rc = uvmain(argc, argv);
	if( UV_FAILED(rc) )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

