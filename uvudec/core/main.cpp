/*
Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <string>
#include "uvd_error.h"
#include "uvd_util.h"
#include "uvd_log.h"
#include "uvd.h"
#include "uvd_data.h"
#include "uvd_format.h"
#include "uvd_address.h"
#include "uvd_language.h"

/*
During parse, several things can happen:
-Exact opcode match
-A prefix
-A multibyte opcode

Before this was handled by function pointers and corresponding opcode table
Seems a solid architecture, should stick with it
*/

//typedef uv_err_t (*uv_disasm_func)(struct );

UVD *g_uvd = NULL;
std::vector<UVDMemoryLocation> g_noncodingAddresses;

uv_err_t disassemble(std::string file, UVDConfig *config)
{
	uv_err_t rc = UV_ERR_GENERAL;
	std::string output;
	UVD *disasm = NULL;
	UVDData *data = NULL;

	printf_debug_level(UVD_DEBUG_PASSES, "main: initializing data streams\n");
	//Select input
	printf_debug_level(UVD_DEBUG_SUMMARY, "Initializing data stream on %s...\n", file.c_str());
	if( UV_FAILED(UVDDataFile::getUVDDataFile(&data, file)) )
	{
		printf_error("Failed to initialize data stream!\n");
		printf_error("Could not read file: %s\n", file.c_str());
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	uv_assert_all(data);
	
	//Create a disassembler engine active on that input
	printf_debug_level(UVD_DEBUG_SUMMARY, "Initializing engine...\n");
	if( UV_FAILED(UVD::getUVD(&disasm, data)) )
	{
		printf_error("Failed to initialize engine\n");
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	uv_assert_all(disasm);
	g_uvd = disasm;
	disasm->m_noncodingAddresses = g_noncodingAddresses;
	disasm->changeConfig(config);
	
	//Get string output
	printf_debug_level(UVD_DEBUG_SUMMARY, "Disassembling...\n");
	rc = disasm->disassemble(file, output);
	if( UV_FAILED(rc) )
	{
		printf_error("Failed to disassemble!\n");
		return UV_DEBUG(UV_ERR_GENERAL);
	}

	printf_debug_level(UVD_DEBUG_PASSES, "main: decompiled\n");

	printf_debug_level(UVD_DEBUG_SUMMARY, "Ready to print!\n");
	//Print string output
	fprintf(g_pOutputFile, "%s\n", output.c_str());
	rc = UV_ERR_OK;
	
error:
	return rc;
	
#if 0
	uv_err_t rc = UV_ERR_GENERAL;
	uint8_t *dat = NULL;	
	unsigned int dat_sz = 0;
	unsigned int read = 0;

	UV_ENTER();

	g_verbose = g_verbose_init;
	printf_debug("Initializing opcodes...\n");
	if( UV_FAILED(uv_disasm_opcode_init()) )
	{
		printf_error("failed 8051 init\n");
		UV_ERR(rc);
		goto error;
	}
	/*
	Read file
	This is raw dat, NOT null terminated string
	*/
	g_verbose = g_verbose_processing;
	printf_debug("Reading program data...\n");
	if( UV_FAILED(read_file(file, &dat, &dat_sz)) )
	{
		printf("Couldn't read file: %s\n", file);
		UV_ERR(rc);
		goto error;		
	}
	printf_debug("Size: 0x%x (%d)\n", dat_sz, dat_sz);

	/* Process file */	
	read = 0;
	printf_debug("Processing data...\n\n");
	for( ;; )
	{ 
		uv_err_t rc_local = UV_ERR_GENERAL;
		struct uv_inst_t *inst = NULL;
		const size_t buff_sz = 256;
		char buff[buff_sz];
		int min_pos = read;
		
		//printf("\nProcessing @ offset 0x%.4X...\n", read); fflush(stdout);
		fflush(stdout);
		if( read > g_addr_max )
		{
			printf("\n\nDebug break\n");
			break;
		}
		
		rc_local = uv_disasm_next(dat, dat_sz, &read, 0, 0, 0, NULL, &inst);
		if( UV_FAILED(rc_local) )
		{
			printf("Failed to get next instruction\n");
			UV_ERR(rc);
			goto error;
		}
		else if( rc_local == UV_ERR_DONE )
		{
			break;
		}
		
		/* Print it then */
		if( UV_FAILED(print_disasm(inst, &buff[0], buff_sz)) )
		{
			printf("Failed to print\n");
			UV_ERR(rc);
			goto error;
		}
		
		if( g_addr_comment )
		{
			printf("# 0x%.4X\n", min_pos);
		}
		if( g_addr_label )
		{
			printf("X%.4X: ", min_pos);
		}

		if( g_verbose )
		{
			printf("INST: 0x%.4X: %s\n\n", min_pos, &buff[0]);
		}
		else
		{
			printf("%s\n", &buff[0]);
		}
		uv_inst_free(inst);
		inst = NULL;
	}
	
	if( g_verbose )
	{
		printf("\nDone\n");
	}
	
	uv_disasm_opcode_deinit();
	
	rc = UV_ERR_OK;
	
error:
	return UV_DEBUG(rc);
#endif

}

/*
It would be cool to set system preferences to compile this selectivly in programs
*/
#define printf_help(format, ...) fprintf(stdout, format, ## __VA_ARGS__)

static void version(void)
{
	printf_help("uvudec version " UVUDEC_VER_STRING "\n");
	printf_help("Copyright 2009 John McMaster\n");
	printf_help("Portions copyright GNU (MD5 implementation)\n");
	printf_help("JohnDMcMaster@gmail.com\n");
}

static void usage(const char *program_name)
{
	printf_help("\n");
	printf_help("Usage: %s <args>\n", program_name);
	printf_help("Args:\n");
	printf_help("--verbose: verbose output.  Equivilent to --verbose=3\n");
	printf_help("--verbose=<level>: set verbose level.  0 (none) - 3 (most)\n");
	printf_help("--verbose-init: for selectivly debugging configuration file reading\n");
	printf_help("--verbose-analysis: for selectivly debugging code analysis\n");
	printf_help("--verbose-processing: for selectivly debugging code post-analysis\n");
	printf_help("--verbose-printing: for selectivly debugging print routine\n");
	printf_help("--config-language=<language>: default config interpreter language (plugins may require specific)\n");
#ifdef USING_LUA
	printf_help("\tlua: use Lua\n");
#endif //USING_LUA
#ifdef USING_PYTHON
	printf_help("\tpython: use Python\n");
#endif //USING_PYTHON
#ifdef USING_JAVASCRIPT
	printf_help("\tjavascript: use javascript\n");
#endif //USING_JAVASCRIPT
	printf_help("--addr-min=<min>: minimum analysis address\n");
	printf_help("--addr-max=<max>: maximum analysis address\n");
	//printf_help("--addr-include=<min>,<max>: analyze range\n");
	printf_help("--addr-exclude-min=<min>: minimum exclusion address\n");
	printf_help("--addr-exclude-max=<max>: maximum exclusion address\n");
	//printf_help("--addr-exclude=<min>,<max>: don't analyze range\n");
	printf_help("--addr-comment: put comments on addresses\n");
	printf_help("--addr-label: label addresses for jumping\n");
	printf_help("--analysis-only[=<bool>]: only do analysis, don't print data\n");
	printf_help("--analysis-address=<address>: only output analysis data for specified address\n");
	printf_help("--opcode-usage: opcode usage count table\n");
	printf_help("--analysis-dir=<dir>: create skeleton data suitible for stored analysis\n");
	printf_help("--input=<file name>: source for data\n");
	printf_help("--output=<file name>: output program (default: stdout)\n");
	printf_help("--debug=<file name>: debug output (default: stdout)\n");
	printf_help("--print-jumped-addresses=<bool>: whether to print information about jumped to addresses (*1)\n");
	printf_help("--print-called-addresses=<bool>: whether to print information about called to addresses (*1)\n");
	printf_help("--useless-ascii-art: append nifty ascii art headers to output files\n");
	printf_help("--help: print this message and exit\n");
	printf_help("--version: print version and exit\n");
	printf_help("\n");
	printf_help("Special files: -: stdin\n");
	printf_help("<bool>:\n");
	printf_help("\ttrue includes case insensitive \"true\", non-zero numbers (ie 1)\n");
	printf_help("\tfalse includes case insensitve \"false\", 0\n");
	//printf("--binary\n");
	printf_help("\n");
	printf_help("*1: WARNING: currently slow, may be fixed in future releases\n");
}

static void help(const char *program_name)
{
	version();
	usage(program_name);
}

#define UVD_OPTION_FILE_STDIN		"-"
#define UVD_OPTION_FILE_STDOUT		"/dev/stdout"
#define UVD_OPTION_FILE_STDERR		"/dev/stderr"

uv_err_t parseFileOption(const std::string optionFileIn, FILE **pOptionFileIn)
{
	std::string optionFile = optionFileIn;
	FILE *pOptionFile = NULL;

	//Assume blank means discard
	if( optionFile == "" )
	{
		optionFile = "/dev/null";
	}

	//Translate file into a I/O structure
	if( optionFile == UVD_OPTION_FILE_STDOUT )
	{
		pOptionFile = stdout;
	}
	else if( optionFile == UVD_OPTION_FILE_STDERR )
	{
		pOptionFile = stderr;
	}
	else
	{
		pOptionFile = fopen(optionFile.c_str(), "w");
	}
	uv_assert_ret(pOptionFile);

	//Save the file pointer we got
	uv_assert_ret(pOptionFileIn);
	*pOptionFileIn = pOptionFile;

	return UV_ERR_OK;
}

bool argToBool(const std::string &sArg)
{
	if( strcmp(sArg.c_str(), "true") == 0 )
	{
		return true;
	}
	else if( strcmp(sArg.c_str(), "false") == 0 )
	{
		return false;
	}
	else if( sArg == "0" )
	{
		return false;
	}
	else if( sArg == "1" )
	{
		return true;
	}
	else
	{
		//Default...should error?
		return true;
	}
}

int main(int argc, char **argv)
{
	int rc = 1;
	int arg_index = 0;
	std::string targetFile = DEFAULT_DECOMPILE_FILE;
	std::string outputFile = UVD_OPTION_FILE_STDOUT;
	std::string debugFile = UVD_OPTION_FILE_STDOUT;
	unsigned int exclusion_addr_min = 0, exclusion_addr_max = 0;
	UVDConfig *config = NULL;
	
	config = new UVDConfig();
	if( !config )
	{
		return 1;
	}
	g_config = config;
	
	for( arg_index = 1; arg_index < argc; ++arg_index )
	{
		const char *cur_arg = argv[arg_index];
		std::string sArg = cur_arg;

		if( !strcmp(cur_arg, "--verbose") )
		{
			g_verbose = TRUE;
			g_verbose_init = TRUE;
			g_verbose_processing = TRUE;
			g_verbose_analysis = TRUE;
			g_verbose_printing = TRUE;
		
			g_verbose_level = UVD_DEBUG_VERBOSE;
		}
		else if( strstr(cur_arg, "--verbose=") == cur_arg )
		{
			g_verbose = TRUE;
			g_verbose_init = TRUE;
			g_verbose_processing = TRUE;
			g_verbose_analysis = TRUE;
			g_verbose_printing = TRUE;
		
			g_verbose_level = strtol(cur_arg + sizeof("--verbose=") - 1, NULL, 0);
		}
		else if( !strcmp("--verbose-init", cur_arg) )
		{
			g_verbose_init = TRUE;
		}
		else if( !strcmp("--verbose-processing", cur_arg) )
		{
			g_verbose_processing = TRUE;
		}
		else if( !strcmp("--verbose-analysis", cur_arg) )
		{
			g_verbose_analysis = TRUE;
		}
		else if( !strcmp("--verbose-printing", cur_arg) )
		{
			g_verbose_printing = TRUE;
		}
		else if( strstr(cur_arg, "--config-language=") == cur_arg )
		{
			std::string sLang = cur_arg + sizeof("--config-language=") - 1;
			//To make selection pre-processable
			if( false )
			{
			}
#ifdef USING_LUA
			else if( sLang == "lua" )
			{
				config->m_configInterpreterLanguage = UVD_LANGUAGE_LUA;
			}
#endif //USING_LUA
#ifdef USING_PYTHON
			else if( sLang == "python" )
			{
				config->m_configInterpreterLanguage = UVD_LANGUAGE_PYTHON;
			}
#endif //USING_PYTHON
#ifdef USING_JAVASCRIPT
			else if( sLang == "javascript" )
			{
				config->m_configInterpreterLanguage = UVD_LANGUAGE_JAVASCRIPT;
			}
#endif //USING_PYTHON
			else
			{
				printf("Unknown language: <%s>\n", cur_arg);
				help(argv[0]);
				goto error;
			}
		}
		//Positive strategy: specify the address we want
		else if( strstr(cur_arg, "--addr-min=") == cur_arg )
		{
			g_addr_min = strtol(cur_arg + sizeof("--addr-min=") - 1, NULL, 0);
		}
		else if( strstr(cur_arg, "--addr-max=") == cur_arg )
		{
			g_addr_max = strtol(cur_arg + sizeof("--addr-max=") - 1, NULL, 0);
		}
		//Negative strategy: specify the addresses we don't want
		else if( strstr(cur_arg, "--addr-exclude-min=") == cur_arg )
		{
			exclusion_addr_min = strtol(cur_arg + sizeof("--addr-exclude-min=") - 1, NULL, 0);
		}
		else if( strstr(cur_arg, "--addr-exclude-max=") == cur_arg )
		{
			exclusion_addr_max = strtol(cur_arg + sizeof("--addr-exclude-max=") - 1, NULL, 0);
		}
		else if( !strcmp("--addr-comment", cur_arg) )
		{
			g_addr_comment = TRUE;
		}
		else if( !strcmp("--addr-label", cur_arg) )
		{
			g_addr_label = TRUE;
		}
		else if( !strcmp("--binary", cur_arg) )
		{
			g_binary = TRUE;
		}
		else if( !strcmp("--opcode-usage", cur_arg) )
		{
			g_print_used = TRUE;
		}
		else if( strstr(cur_arg, "--input=") == cur_arg )
		{
			targetFile = cur_arg + sizeof("--input=") - 1;
		}
		else if( strstr(cur_arg, "--output=") == cur_arg )
		{
			outputFile = cur_arg + sizeof("--output=") - 1;
		}
		else if( strstr(cur_arg, "--debug=") == cur_arg )
		{
			debugFile = cur_arg + sizeof("--debug=") - 1;
		}
		else if( strstr(cur_arg, "--analysis-dir=") == cur_arg )
		{
			config->m_analysisDir = cur_arg + sizeof("--analysis-dir=") - 1;
		}
		else if( strstr(cur_arg, "--analysis-only=") == cur_arg )
		{
			std::string sAnalysisOnly = cur_arg + sizeof("--analysis-only=") - 1;
			config->m_analysisOnly = argToBool(sAnalysisOnly);
		}
		else if( strcmp(cur_arg, "--analysis-only") == 0 )
		{
			config->m_analysisOnly = true;
		}
		else if( strstr(cur_arg, "--analysis-address=") == cur_arg )
		{
			std::string sAnalysisAddress = cur_arg + sizeof("--analysis-address=") - 1;
			int iAnalysisAddress = strtol(sAnalysisAddress.c_str(), NULL, 0);
			config->m_analysisOutputAddresses.insert(iAnalysisAddress);
		}
		else if( strcmp(cur_arg, "--print-jumped-addresses") == 0 )
		{
			g_jumped_sources = true;
		}
		else if( strstr(cur_arg, "--print-jumped-addresses=") == cur_arg )
		{
			std::string arg = cur_arg + sizeof("--print-jumped-addresses=") - 1;
			g_jumped_sources  = argToBool(arg);
		}
		else if( strcmp(cur_arg, "--print-called-addresses") == 0 )
		{
			g_called_sources = true;
		}
		else if( strstr(cur_arg, "--print-called-addresses=") == cur_arg )
		{
			std::string arg = cur_arg + sizeof("--print-called-addresses=") - 1;
			g_called_sources = argToBool(arg);
		}
		else if( !strcmp("--useless-ascii-art", cur_arg))
		{
			config->m_uselessASCIIArt = true;
		}
		else if( !strcmp("--version", cur_arg))
		{
			version();
			rc = 0;
			goto error;
		}
		else if( !strcmp("--help", cur_arg) || !strcmp("-h", cur_arg) )
		{
			help(argv[0]);
			rc = 0;
			goto error;
		}
		else
		{
			printf("Unknown option: <%s>\n", cur_arg);
			help(argv[0]);
			goto error;
		}
	}
	
	if( targetFile.empty() )
	{
		printf_error("Target file not specified\n");
		help(argv[0]);
		goto error;
	}

	if( UV_FAILED(parseFileOption(outputFile, &g_pOutputFile)) )
	{
		printf_error("Could not open file: %s\n", outputFile.c_str());
		goto error;
	}
	if( UV_FAILED(parseFileOption(debugFile, &g_pDebugFile)) )
	{
		printf_error("Could not open file: %s\n", debugFile.c_str());
		goto error;
	}

	g_noncodingAddresses.push_back(UVDMemoryLocation(exclusion_addr_min, exclusion_addr_max));

	printf_debug_level(UVD_DEBUG_SUMMARY, "Initializing log...\n");
	//Keep same as debug output
	uv_log_init(NULL);
	//uv_log_init("-");
	
	printf_debug("min: 0x%.4X, max: 0x%.4X\n", g_addr_min, g_addr_max);

	if( UV_FAILED(disassemble(targetFile, config)) )
	{
		printf_error("Top level disassemble failed\n");
		goto error;
	}
	
	uv_log_deinit();
	rc = 0;

error:
	return rc;
}
