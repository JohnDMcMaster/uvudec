/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details

bin2obj entry point
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include "uvd/config/arg_property.h"
#include "uvd/config/arg_util.h"
#include "uvd/util/error.h"
#include "uvd/core/init.h"
#include "uvd/util/util.h"
#include "uvd/core/uvd.h"
#include "uvd/flirt/flirt.h"
#include "uvd/flirt/args.h"
#include "uvd/flirt/args_property.h"
#include "uvd/data/data.h"

static uv_err_t versionPrintPrefixThunk();

std::string g_inputFile;
std::string g_outputFile;

static const char *GetVersion()
{
	return UVUDEC_VER_STRING;
}

static uv_err_t argParser(const UVDArgConfig *argConfig, std::vector<std::string> argumentArguments)
{
	UVDConfig *config = NULL;
	UVDConfigFLIRT *flirtConfig = NULL;
	//If present
	std::string firstArg;
	uint32_t firstArgNum = 0;
	
	config = g_config;
	uv_assert_ret(config);
	uv_assert_ret(config->m_argv);
	uv_assert_ret(argConfig);

	flirtConfig = &g_config->m_flirt;

	if( !argumentArguments.empty() )
	{
		firstArg = argumentArguments[0];
		firstArgNum = strtol(firstArg.c_str(), NULL, 0);
	}

	if( argConfig->isNakedHandler() )
	{
		/*
		Target files
		Assume first input argument is input and second is output
		*/
		if( argumentArguments.size() >= 1 )
		{
			g_inputFile = argumentArguments[0];
		}
		if( argumentArguments.size() >= 2 )
		{
			g_outputFile = argumentArguments[1];
		}
		if( argumentArguments.size() >= 3 )
		{
			printf_error("only 2 max bare arguments, given: %d\n", argumentArguments.size());
			return UV_DEBUG(UV_ERR_GENERAL);
		}
	}
	/*
	else if( argConfig->m_propertyForm == UVD_PROP_ANALYSIS_DIR )
	{
		uv_assert_ret(!argumentArguments.empty());
		config->m_analysisDir = firstArg;
	}
	*/
	else
	{
		//return UV_DEBUG(argParserDefault(argConfig, argumentArguments));
		return UV_DEBUG(UV_ERR_GENERAL);
	}

	return UV_ERR_OK;
}

uv_err_t initProgConfig()
{
	uv_assert_err_ret(initFLIRTSharedConfig());

	//Callbacks
	g_config->versionPrintPrefixThunk = versionPrintPrefixThunk;

	uv_assert_err_ret(g_config->registerDefaultArgument(argParser, " [input object file] [output .pat file]"));	
	//uv_assert_err_ret(g_config->registerArgument(UVD_PROP_ANALYSIS_DIR, 0, "analysis-dir", "create data suitible for stored analysis", 1, argParser, false));

	return UV_ERR_OK;	
}

static uv_err_t versionPrintPrefixThunk()
{
	const char *program_name = "uvbin2obj";
	
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
	UVD *uvd = NULL;
	uv_err_t parseMainRc = UV_ERR_GENERAL;
	std::string inputFile;
	
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

	if( g_inputFile.empty() )
	{
		printf_error("Target file not specified\n");
		UVDHelp();
		uv_assert_err(UV_ERR_GENERAL);
	}
	if( g_outputFile.empty() )
	{
		g_outputFile = "/dev/stdout";
	}
	if( UV_FAILED(UVD::getUVD(&uvd, g_inputFile)) )
	{
		printf_error("Failed to initialize UVD engine\n");
		rc = UV_ERR_OK;
		goto error;
	}
	uv_assert_ret(uvd);

	//Get string output
	printf_debug_level(UVD_DEBUG_SUMMARY, "main: creating object file...\n");
	uv_assert_err_ret(uvd->analyze());
	uv_assert_err_ret(uvd->m_analyzer->m_curDb->saveData(g_outputFile));
	printf_debug_level(UVD_DEBUG_PASSES, "main: object file done\n");

	rc = UV_ERR_OK;

error:
	uv_assert_err_ret(UVDDeinit());
		
	return UV_DEBUG(rc);
}

int main(int argc, char **argv)
{
	printf_debug("main: enter\n");

	//Simple translation to keep most stuff in the framework
	uv_err_t rc = uvmain(argc, argv);
	printf_debug("main: exit\n");
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
