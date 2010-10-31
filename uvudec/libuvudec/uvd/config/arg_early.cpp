/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/config/arg.h"
#include "uvd/config/arg_util.h"
#include "uvd/config/arg_property.h"
#include "uvd/config/config.h"

static uv_err_t argParser(const UVDArgConfig *argConfig, std::vector<std::string> argumentArguments);

static uv_err_t argParser(const UVDArgConfig *argConfig, std::vector<std::string> argumentArguments)
{
	UVDConfig *config = NULL;
	//If present
	std::string firstArg;
	uint32_t firstArgNum = 0;
	bool firstArgBool = true;

	config = g_config;
	uv_assert_ret(config);
	uv_assert_ret(config->m_argv);

	uv_assert_ret(argConfig);

	if( !argumentArguments.empty() )
	{
		firstArg = argumentArguments[0];
		firstArgNum = strtol(firstArg.c_str(), NULL, 0);
		firstArgBool = UVDArgToBool(firstArg);
	}

	if( argConfig->isNakedHandler() )
	{
		//skip...let primary handler deal with it
	}
	/*
	Debug
	*/
	else if( argConfig->m_propertyForm == UVD_PROP_DEBUG_IGNORE_ERRORS )
	{
		config->m_ignoreErrors = firstArgBool;
	}
	else if( argConfig->m_propertyForm == UVD_PROP_DEBUG_SUPPRESS_ERRORS )
	{
		config->m_suppressErrors = firstArgBool;
	}
	else if( argConfig->m_propertyForm == UVD_PROP_DEBUG_LEVEL )
	{
		//If they didn't set any flags, assume its a general state across the program
		if( !config->anyVerboseActive() )
		{
			//config->setVerboseAll();
			UVDSetDebugFlag(UVD_DEBUG_TYPE_GENERAL, true);
		}
	
		//Did we specify or want default?
		if( argumentArguments.empty() )
		{
			config->m_debugLevel = UVD_DEBUG_VERBOSE;
		}
		else
		{
			config->m_debugLevel = firstArgNum;
		}
	}
	/*
	This looks unused, was used for printing out the binary as we go along during disassembly
	else if( argConfig->m_propertyForm == "debug.print_binary" )
	{
		g_binary = TRUE;
	}
	*/
	else if( argConfig->m_propertyForm == UVD_PROP_DEBUG_FILE )
	{
		uv_assert_ret(!argumentArguments.empty());
		config->m_sDebugFile = firstArg;
	}
	//Plugins
	else if( argConfig->m_propertyForm == UVD_PROP_PLUGIN_FILE )
	{
		uv_assert_ret(!argumentArguments.empty());
		uv_assert_err_ret(config->m_plugin.addToLoad(firstArg));
	}
	else if( argConfig->m_propertyForm == UVD_PROP_PLUGIN_NAME )
	{
		uv_assert_ret(!argumentArguments.empty());
		uv_assert_err_ret(config->m_plugin.addToInitialize(firstArg));
	}
	else if( argConfig->m_propertyForm == UVD_PROP_PLUGIN_APPEND_PATH )
	{
		uv_assert_ret(!argumentArguments.empty());
		uv_assert_err_ret(config->m_plugin.appendPluginPath(firstArg));
	}
	else if( argConfig->m_propertyForm == UVD_PROP_PLUGIN_PREPEND_PATH )
	{
		uv_assert_ret(!argumentArguments.empty());
		uv_assert_err_ret(config->m_plugin.prependPluginPath(firstArg));
	}
	//Do not error on unrecognized args, they will be handled in the main case

	return UV_ERR_OK;
}

uv_err_t UVDPluginConfig::earlyArgParse(UVDConfig *config)
{
	std::string mainPluginDir;
	
	//Plugin
	uv_assert_err_ret(g_config->registerArgument(UVD_PROP_PLUGIN_NAME, 0, "plugin", "load given library name as plugin", 1, argParser, false, "", true));
	uv_assert_err_ret(g_config->registerArgument(UVD_PROP_PLUGIN_APPEND_PATH, 0, "plugin-path", "append dir to plugin search path", 1, argParser, false, "", true));
	uv_assert_err_ret(g_config->registerArgument(UVD_PROP_PLUGIN_PREPEND_PATH, 0, "plugin-path-prepend", "prepend dir to plugin search path", 1, argParser, false, "", true));
	
	//Debug
	//NOTE: type debug flags are automaticlly registered along with the type
	uv_assert_err_ret(g_config->registerArgument(UVD_PROP_DEBUG_IGNORE_ERRORS, 0, "ignore-errors", "ignore nonfatal errors", 1, argParser, true, "", true));
	uv_assert_err_ret(g_config->registerArgument(UVD_PROP_DEBUG_SUPPRESS_ERRORS, 0, "suppress-errors", "suprress printing of warnings and nonfatal errors", 1, argParser, true, "", true));
	uv_assert_err_ret(g_config->registerArgument(UVD_PROP_DEBUG_LEVEL, 0, "verbose", "debug verbosity level", 1, argParser, true, "", true));
	uv_assert_err_ret(g_config->registerArgument(UVD_PROP_DEBUG_FILE, 0, "debug-file", "debug output (default: stdout)", 1, argParser, true, "", true));

	//So we can ignore arg meant for later
	uv_assert_err_ret(g_config->registerDefaultArgument(argParser, "", 0, true, true, true));

	//FIXME XXX TODO: hack until I can have startup config file variables
	mainPluginDir = config->m_installDir + "/lib/plugin";
	m_dirs.push_back(mainPluginDir);

	return UV_ERR_OK;
}

