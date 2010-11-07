/*
UVNet Utils (uvutils)
uvsigutil: IDA .sig format utilities
In particular, compression, decompression, and dumping
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under GPL V3+, see COPYING for details

Some code taken from
http://www.woodmann.com/forum/showthread.php?7517-IDA-signature-file-format
*/

#include "uvd/flirt/flirt.h"
#include "uvd/flirt/sig/format.h"
#include "uvd/config/arg_property.h"
#include "uvd/core/init.h"
#include <ctype.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#define ACTION_NONE						0
#define ACTION_DECOMPRESS				1
#define ACTION_COMPRESS					2
#define ACTION_DUMP						3
#define ACTION_OBJECT					4

#define PROP_ACTION_DECOMPRESS			"action.decompress"
#define PROP_ACTION_COMPRESS			"action.compress"
#define PROP_ACTION_DUMP				"action.dump"
#define PROP_ACTION_OBJECT				"action.object"

uint32_t g_action = ACTION_NONE;

static uv_err_t dumpFile(const std::string &fileName)
{
	if( UV_FAILED(UVDFLIRT::getFLIRT(&g_flirt)) )
	{
		printf_error("Failed to initialize FLIRT engine\n");
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	uv_assert_ret(g_flirt);

	uv_assert_err_ret(g_flirt->dumpSigFile(fileName));
	
	return UV_ERR_OK;
}

static std::vector<std::string> g_inputFiles;
static std::vector<std::string> g_argumentArguments;
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

	if( argConfig->isNakedHandler() )
	{
		//With default options, we call this last, so action should already be settled on, but easier to do everything together
		g_argumentArguments = argumentArguments;
	}
	else if( argConfig->m_propertyForm == UVD_PROP_TARGET_FILE )
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
	uv_assert_err_ret(g_config->registerDefaultArgument(argParser, " [action specific]"));	

	uv_assert_err_ret(g_config->registerArgument(UVD_PROP_TARGET_FILE, 0, "input", "source file for data", 1, argParser, false));
	uv_assert_err_ret(g_config->registerArgument(UVD_PROP_OUTPUT_FILE, 0, "output", "dest file for data (default: stdout)", 1, argParser, false));
	//uv_assert_err_ret(g_config->registerArgument(PROP_ACTION_DECOMPRESS, 0, "decompress", "decompress input files, placing in output (default: stdout)", 0, argParser, false));
	//uv_assert_err_ret(g_config->registerArgument(PROP_ACTION_COMPRESS, 0, "compress", "compress input files, overwritting them unless output specified", 0, argParser, false));
	uv_assert_err_ret(g_config->registerArgument(PROP_ACTION_DUMP, 0, "dump", "dump input files, placing in output", 0, argParser, false));
	//uv_assert_err_ret(g_config->registerArgument(PROP_ACTION_OBJECT, 0, "object", "make best effort to convert input from .sig/.pat to ELF object", 0, argParser, false));
	//uv_assert_err_ret(g_config->registerArgument(PROP_ACTION_OBJECT, 0, "input-type", "specify input type instead of guessing by extension/magic", 0, argParser, false));

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
		for( std::vector<std::string>::iterator iter = g_argumentArguments.begin();
				iter != g_argumentArguments.end(); ++iter )
		{
			g_inputFiles.push_back(*iter);
		}

		for( std::vector<std::string>::iterator iter = g_inputFiles.begin(); iter != g_inputFiles.end(); ++iter )
		{
			std::string file = *iter;
			
			uv_assert_err_ret(dumpFile(file));
		}
	}
	else if( g_action == ACTION_OBJECT )
	{
		printf_error("object conversion unsupported\n");
		goto error;
	}
	else if( g_action == ACTION_NONE )
	{
		printf_error("target action not specified\n");
		UVDPrintHelp();
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
		printf_error("failed\n");
		return 1;
	}
	else
	{
		return 0;
	}
}

