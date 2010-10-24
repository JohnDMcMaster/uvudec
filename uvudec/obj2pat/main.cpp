/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details

obj2pat entry point
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <string>
#include "uvd/config/arg_property.h"
#include "uvd/config/arg_util.h"
#include "uvd/util/error.h"
#include "uvd/util/log.h"
#include "uvd/core/init.h"
#include "uvd/util/util.h"
#include "uvd/core/uvd.h"
#include "uvd/flirt/flirt.h"
#include "uvd/flirt/args.h"
#include "uvd/flirt/args_property.h"
#include "uvd/data/data.h"
#include "uvd/language/format.h"
#include "uvd/assembly/address.h"
#include "uvd/language/language.h"

static uv_err_t versionPrintPrefixThunk();

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
		We could also do funky guess stuff
		Think we should let pat2sig deal with combining files
		*/
		if( argumentArguments.size() >= 1 )
		{
			//These may not have suffixes (and many different of them), so don't bother checking
			flirtConfig->m_targetFiles.push_back(argumentArguments[0]);
		}
		if( argumentArguments.size() >= 2 )
		{
			flirtConfig->m_outputFile = argumentArguments[1];
			if( flirtConfig->m_outputFile.find(".pat") == std::string::npos )
			{
				flirtConfig->m_outputFile += ".pat";
			}
		}
		if( argumentArguments.size() >= 3 )
		{
			printf_error("only 2 max bare arguments, given: %d\n", argumentArguments.size());
			return UV_DEBUG(UV_ERR_GENERAL);
		}
	}
	else if( argConfig->m_propertyForm == UVD_PROP_FLIRT_PAT_FUNCTIONS_AS_MODULES )
	{
		if( argumentArguments.empty() )
		{
			flirtConfig->m_functionsAsModules = true;
		}
		else
		{
			flirtConfig->m_functionsAsModules = UVDArgToBool(firstArg);
		}
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
	uv_assert_err_ret(initFLIRTSharedConfig());

	//Callbacks
	g_config->versionPrintPrefixThunk = versionPrintPrefixThunk;

	uv_assert_err_ret(g_config->registerDefaultArgument(argParser, " [input object file] [output .pat file]"));	
	uv_assert_err_ret(g_config->registerArgument(UVD_PROP_FLIRT_PAT_FUNCTIONS_AS_MODULES, 0, "functions-as-modules", "functions will not be grouped into object modules", 1, argParser, true));

	return UV_ERR_OK;	
}

static uv_err_t versionPrintPrefixThunk()
{
	const char *program_name = "uvobj2pat";
	
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
	UVDConfigFLIRT *flirtConfig = NULL;
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

	flirtConfig = &g_config->m_flirt;;
	uv_assert_ret(flirtConfig);

	if( flirtConfig->m_targetFiles.empty() )
	{
		printf_error("Target file(s) not specified\n");
		UVDHelp();
		uv_assert_err(UV_ERR_GENERAL);
	}
	if( flirtConfig->m_targetFiles.size() != 1 )
	{
		printf_error("requires exactly 1 target file, got %d\n", flirtConfig->m_targetFiles.size());
		UVDHelp();
		uv_assert_err(UV_ERR_GENERAL);
	}
	inputFile = flirtConfig->m_targetFiles[0];
	if( UV_FAILED(UVD::getUVD(&uvd, inputFile)) )
	{
		printf_error("Failed to initialize FLIRT engine\n");
		rc = UV_ERR_OK;
		goto error;
	}
	uv_assert_ret(uvd);

	//Get string output
	printf_debug_level(UVD_DEBUG_SUMMARY, "main: creating pat file...\n");
	uv_assert_err_ret(uvd->m_flirt->toPatFile(flirtConfig->m_outputFile));
	printf_debug_level(UVD_DEBUG_PASSES, "main: pat done\n");


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
