/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/config/config.h"
#include "uvd/flirt/args.h"
#include "uvd/flirt/args_property.h"
#include "uvd/config/arg_property.h"
#include "uvd/config/arg_util.h"

static uv_err_t argParser(const UVDArgConfig *argConfig, std::vector<std::string> argumentArguments);

uv_err_t initFLIRTSharedConfig()
{
	uv_assert_ret(g_config);

	uv_assert_err_ret(g_config->registerArgument(UVD_PROP_FLIRT_FLAIR_COMPATIBILITY, 'f', "flair", "try to be FLAIR like", 1, argParser, true));
	uv_assert_err_ret(g_config->registerArgument(UVD_PROP_FLIRT_MIN_SIGNATURE_LENGTH, 0, "sig-min-length", "minimum (unrelocatable) signature length, default 4 bytes", 1, argParser, true));
	uv_assert_err_ret(g_config->registerArgument(UVD_PROP_FLIRT_PAT_REFERENCE_TRAILING_SPACE, 0, "pat-trailing-space", "always put a space after references (even if at line end)", 1, argParser, true));
	uv_assert_err_ret(g_config->registerArgument(UVD_PROP_FLIRT_PAT_PREFIX_UNDERSCORES, 0, "pat-prefix-underscores", "prefix underscores to symbols in certain object formats", 1, argParser, true));
	uv_assert_err_ret(g_config->registerArgument(UVD_PROP_FLIRT_PAT_FUNCTIONS_AS_MODULES, 0, "pat-functions-as-modules", "don't group functions into modules", 1, argParser, true));	
	uv_assert_err_ret(g_config->registerArgument(UVD_PROP_FLIRT_PAT_NEWLINE, 0, "pat-newline", "line termination, lf or crlf", 1, argParser, true));
	uv_assert_err_ret(g_config->registerArgument(UVD_PROP_TARGET_FILE, 0, "input", "Object library (ELF, OMF, etc)", 1, argParser, false));
	uv_assert_err_ret(g_config->registerArgument(UVD_PROP_OUTPUT_FILE, 0, "output", "pat file (default: stdout)", 1, argParser, false));

	return UV_ERR_OK;	
}

static uv_err_t argParser(const UVDArgConfig *argConfig, std::vector<std::string> argumentArguments)
{
	UVDConfig *config = NULL;
	UVDConfigFLIRT *flirtConfig = NULL;
	//If present
	std::string firstArg;
	uint32_t firstArgNum = 0;
	bool firstArgBool = true;
	
	config = g_config;
	uv_assert_ret(config);
	flirtConfig = &config->m_flirt;
	uv_assert_ret(flirtConfig);
	uv_assert_ret(config->m_argv);

	uv_assert_ret(argConfig);

	if( !argumentArguments.empty() )
	{
		firstArg = argumentArguments[0];
		firstArgNum = strtol(firstArg.c_str(), NULL, 0);
		firstArgBool = UVDArgToBool(firstArg);
	}

	if( argConfig->m_propertyForm == UVD_PROP_FLIRT_FLAIR_COMPATIBILITY )
	{
		flirtConfig->makeFLAIRCompatible(firstArgBool);
	}
	else if( argConfig->m_propertyForm == UVD_PROP_FLIRT_PAT_REFERENCE_TRAILING_SPACE )
	{
		flirtConfig->m_patternReferenceTrailingSpace = firstArgBool;
	}
	else if( argConfig->m_propertyForm == UVD_PROP_FLIRT_PAT_PREFIX_UNDERSCORES )
	{
		flirtConfig->m_prefixUnderscores = firstArgBool;
	}
	else if( argConfig->m_propertyForm == UVD_PROP_FLIRT_PAT_FUNCTIONS_AS_MODULES )
	{
		flirtConfig->m_functionsAsModules = firstArgBool;
	}
	else if( argConfig->m_propertyForm == UVD_PROP_FLIRT_PAT_NEWLINE )
	{
		uv_assert_ret(!argumentArguments.empty());

		if( firstArg == "lf" )
		{
			flirtConfig->m_patternFileNewline = "\n";
		}
		else if( firstArg == "crlf" )
		{
			flirtConfig->m_patternFileNewline = "\r\n";
		}
		//We could allow arbitrary line termination, but it seems more like a user input mistake
		else
		{
			printf_error("Unrecognized line termination: %s\n", firstArg.c_str());
			return UV_DEBUG(UV_ERR_GENERAL);
		}
	}
	else if( argConfig->m_propertyForm == UVD_PROP_FLIRT_MIN_SIGNATURE_LENGTH )
	{
		uv_assert_ret(!argumentArguments.empty());
		flirtConfig->m_patSignatureLengthMin = firstArgNum;
	}
	else if( argConfig->m_propertyForm == UVD_PROP_FLIRT_MAX_SIGNATURE_LENGTH )
	{
		uv_assert_ret(!argumentArguments.empty());
		flirtConfig->m_patSignatureLengthMax = firstArgNum;
	}
	//All of these have mult input files and one output file
	else if( argConfig->m_propertyForm == UVD_PROP_TARGET_FILE )
	{
		uv_assert_ret(!argumentArguments.empty());
		flirtConfig->m_targetFiles.push_back(firstArg);
	}
	else if( argConfig->m_propertyForm == UVD_PROP_OUTPUT_FILE )
	{
		uv_assert_ret(!argumentArguments.empty());
		flirtConfig->m_outputFile = firstArg;
	}
	//Unknown
	else
	{
		//return UV_DEBUG(argParserDefault(argConfig, argumentArguments));
		return UV_DEBUG(UV_ERR_GENERAL);
	}

	return UV_ERR_OK;
}
