/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under terms of the three clause BSD license, see LICENSE for details

obj2pat entry point
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
#include "uvd_flirt.h"
#include "uvd_flirt_args.h"
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

uv_err_t versionPrintPrefixThunk();

static const char *GetVersion()
{
	return UVUDEC_VER_STRING;
}

static uv_err_t doConvert()
{
	uv_err_t rc = UV_ERR_GENERAL;
	std::string output;
	UVDConfigFLIRT *flirtConfig = NULL;
		
	flirtConfig = &g_uvd->m_config->m_flirt;
	uv_assert_ret(flirtConfig);

	//Get string output
	printf_debug_level(UVD_DEBUG_SUMMARY, "main: creating pat file...\n");
	uv_assert_err_ret(g_flirt->objs2patFile(flirtConfig->m_targetFiles, flirtConfig->m_outputFile));
	printf_debug_level(UVD_DEBUG_PASSES, "main: pat done\n");

	rc = UV_ERR_OK;
	
	return rc;
}

#if 0
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

	if( false )
	{
	}
	else
	{
		//return UV_DEBUG(argParserDefault(argConfig, argumentArguments));
		return UV_DEBUG(UV_ERR_GENERAL);
	}

	return UV_ERR_OK;
}
#endif

uv_err_t initProgConfig()
{
	uv_assert_err_ret(initFLIRTSharedConfig());

	//Callbacks
	g_config->versionPrintPrefixThunk = versionPrintPrefixThunk;

	return UV_ERR_OK;	
}

uv_err_t versionPrintPrefixThunk()
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


	//Create a doConvertr engine active on that input
	printf_debug_level(UVD_DEBUG_SUMMARY, "doConvert: initializing FLIRT engine...\n");
	if( UV_FAILED(UVDFLIRT::getFLIRT(&g_flirt)) )
	{
		printf_error("Failed to initialize FLIRT engine\n");
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	uv_assert_ret(g_flirt);

	uv_assert_ret(g_uvd);
	uv_assert_ret(g_uvd->m_config);
	flirtConfig = &g_uvd->m_config->m_flirt;
	uv_assert_ret(flirtConfig);

	if( flirtConfig->m_targetFiles.empty() )
	{
		printf_error("Target file(s) not specified\n");
		UVDHelp();
		uv_assert_err(UV_ERR_GENERAL);
	}
	
	if( UV_FAILED(doConvert()) )
	{
		printf_error("Top level doConvert failed\n");
		uv_assert(UV_ERR_GENERAL);
	}	

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
		return 1;
	}
	else
	{
		return 0;
	}
}
