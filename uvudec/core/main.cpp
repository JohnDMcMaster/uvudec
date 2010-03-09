/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <string>
#include "uvd_arg_property.h"
#include "uvd_arg_util.h"
#include "uvd_error.h"
#include "uvd_log.h"
#include "uvd_init.h"
#include "uvd_util.h"
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

static std::string g_targetFile;
static std::string g_outputFile;
static FILE *g_pOutputFile = NULL;

uv_err_t versionPrintPrefixThunk();

const char *GetVersion()
{
	return UVUDEC_VER_STRING;
}

uv_err_t disassemble(std::string file)
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
	printf_debug_level(UVD_DEBUG_SUMMARY, "disassemble: initializing engine...\n");
	if( UV_FAILED(UVD::getUVD(&disasm, data)) )
	{
		printf_error("Failed to initialize engine\n");
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	uv_assert_all(disasm);
	uv_assert_all(g_uvd);

printf("Debug break\n");
rc = UV_ERR_OK;
goto error;

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
	if( data )
	{
		delete data;
	}
	return rc;
}

static uv_err_t argParser(const UVDArgConfig *argConfig, std::vector<std::string> argumentArguments)
{
	UVDConfig *config = NULL;
	//If present
	std::string firstArg;
	uint32_t firstArgNum = 0;
	
	config = g_config;
	uv_assert_ret(config);
	uv_assert_ret(config->m_argv);
	uv_assert_ret(argConfig);

	if( !argumentArguments.empty() )
	{
		firstArg = argumentArguments[0];
		firstArgNum = strtol(firstArg.c_str(), NULL, 0);
	}

	if( argConfig->m_propertyForm == UVD_PROP_TARGET_FILE )
	{
		uv_assert_ret(!argumentArguments.empty());
		g_targetFile = firstArg;
	}
	else if( argConfig->m_propertyForm == UVD_PROP_OUTPUT_FILE )
	{
		uv_assert_ret(!argumentArguments.empty());
		g_outputFile = firstArg;
	}
	else
	{
		//return UV_DEBUG(argParserDefault(argConfig, argumentArguments));
		return UV_DEBUG(UV_ERR_GENERAL);
	}

	return UV_ERR_OK;
}

uv_err_t initProgConfig()
{
	//Arguments
	g_config->m_configArgs.push_back(new UVDArgConfig(UVD_PROP_TARGET_FILE, 0, "input", "source file for data", 1, argParser, false));
	g_config->m_configArgs.push_back(new UVDArgConfig(UVD_PROP_OUTPUT_FILE, 0, "output", "output program (default: stdout)", 1, argParser, false));

	//Callbacks
	g_config->versionPrintPrefixThunk = versionPrintPrefixThunk;

	g_outputFile = "/dev/stdout";
	g_targetFile = DEFAULT_DECOMPILE_FILE;

	return UV_ERR_OK;	
}

uv_err_t versionPrintPrefixThunk()
{
	const char *program_name = "uvudec";
	
	/*
	if( g_config && g_config->m_argv )
	{
		program_name = g_config->m_argv[0];
	}
	*/

	printf_help("%s version %s\n", program_name, GetVersion());
	return UV_ERR_OK;
}

uv_err_t uvmain(int argc, char **argv)
{
	uv_err_t rc = UV_ERR_GENERAL;
	UVDConfig *config = NULL;
	uv_err_t parseMainRc = UV_ERR_GENERAL;
	
	if( strcmp(GetVersion(), UVDGetVersion()) )
	{
		printf_warn("libuvudec version mismatch (exe: %s, libuvudec: %s)\n", GetVersion(), UVDGetVersion());
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

	if( g_targetFile.empty() )
	{
		printf_error("Target file not specified\n");
		UVDHelp();
		uv_assert(UV_ERR_GENERAL);
	}

	if( UV_FAILED(parseFileOption(g_outputFile, &g_pOutputFile)) )
	{
		printf_error("Could not open file: %s\n", g_outputFile.c_str());
		uv_assert(UV_ERR_GENERAL);
	}

	//printf_debug_level(UVD_DEBUG_SUMMARY, "Initializing log...\n");
	//Keep same as debug output
	//uv_log_init(NULL);
	//uv_log_init("-");
	
	printf_debug("min: 0x%.4X, max: 0x%.4X\n", config->m_addr_min, config->m_addr_max);

	if( UV_FAILED(disassemble(g_targetFile)) )
	{
		printf_error("Top level disassemble failed\n");
		uv_assert(UV_ERR_GENERAL);
	}	

	rc = UV_ERR_OK;

error:
	uv_assert_err_ret(UVDDeinit());
		
	return UV_DEBUG(rc);
}

int main(int argc, char **argv)
{
	printf("main: enter\n");

	//Simple translation to keep most stuff in the framework
	uv_err_t rc = uvmain(argc, argv);
	printf("main: exit\n");
	if( UV_FAILED(rc) )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
