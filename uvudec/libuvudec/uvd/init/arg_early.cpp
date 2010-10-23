/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/init/arg.h"
#include "uvd/init/arg_util.h"
#include "uvd/init/arg_property.h"
#include "uvd/init/config.h"

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
	*/
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
	else if( argConfig->m_propertyForm == UVD_PROP_DEBUG_ARGS )
	{
		config->m_verbose_args = firstArgBool;
		UVDSetDebugFlag(UVD_DEBUG_TYPE_ARGS, firstArgBool);
		uv_assert_err_ret(config->ensureDebugLevel(UVD_DEBUG_TEMP));
	}
	else if( argConfig->m_propertyForm == UVD_PROP_DEBUG_INIT )
	{
		config->m_verbose_init = firstArgBool;
		UVDSetDebugFlag(UVD_DEBUG_TYPE_INIT, firstArgBool);
		uv_assert_err_ret(config->ensureDebugLevel(UVD_DEBUG_TEMP));
	}
	else if( argConfig->m_propertyForm == UVD_PROP_DEBUG_PLUGIN )
	{
printf("plugin debug flag set: %d\n", UVD_DEBUG_TYPE_PLUGIN);
		UVDSetDebugFlag(UVD_DEBUG_TYPE_PLUGIN, firstArgBool);
		uv_assert_err_ret(config->ensureDebugLevel(UVD_DEBUG_TEMP));
	}
	else if( argConfig->m_propertyForm == UVD_PROP_DEBUG_PROCESSING )
	{
		config->m_verbose_processing = firstArgBool;
		UVDSetDebugFlag(UVD_DEBUG_TYPE_PROCESSING, firstArgBool);
		uv_assert_err_ret(config->ensureDebugLevel(UVD_DEBUG_TEMP));
	}
	else if( argConfig->m_propertyForm == UVD_PROP_DEBUG_ANALYSIS )
	{
		config->m_verbose_analysis = firstArgBool;
		UVDSetDebugFlag(UVD_DEBUG_TYPE_ANALYSIS, firstArgBool);
		uv_assert_err_ret(config->ensureDebugLevel(UVD_DEBUG_TEMP));
	}
	else if( argConfig->m_propertyForm == UVD_PROP_DEBUG_PRINTING )
	{
		config->m_verbose_printing = firstArgBool;
		UVDSetDebugFlag(UVD_DEBUG_TYPE_PRINT, firstArgBool);
		uv_assert_err_ret(config->ensureDebugLevel(UVD_DEBUG_TEMP));
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
	else if( argConfig->m_propertyForm == UVD_PROP_PLUGIN_NAME )
	{
		uv_assert_ret(!argumentArguments.empty());
		uv_assert_err_ret(config->m_plugin.addPlugin(firstArg));
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
	//Plugin
	uv_assert_err_ret(g_config->registerArgument(UVD_PROP_PLUGIN_NAME, 0, "plugin", "load given library name as plugin", 1, argParser, false, "", true));
	uv_assert_err_ret(g_config->registerArgument(UVD_PROP_PLUGIN_APPEND_PATH, 0, "plugin-path", "append dir to plugin search path", 1, argParser, false, "", true));
	uv_assert_err_ret(g_config->registerArgument(UVD_PROP_PLUGIN_PREPEND_PATH, 0, "plugin-path-prepend", "prepend dir to plugin search path", 1, argParser, false, "", true));
	
	//Debug
	uv_assert_err_ret(g_config->registerArgument(UVD_PROP_DEBUG_LEVEL, 0, "verbose", "debug verbosity level", 1, argParser, true, "", true));
	uv_assert_err_ret(g_config->registerArgument(UVD_PROP_DEBUG_ANALYSIS, 0, "verbose-analysis", "selectivly debug analysis", 1, argParser, true, "", true));
	uv_assert_err_ret(g_config->registerArgument(UVD_PROP_DEBUG_ARGS, 0, "verbose-args", "selectivly debug argument parsing", 1, argParser, true, "", true));
	uv_assert_err_ret(g_config->registerArgument(UVD_PROP_DEBUG_INIT, 0, "verbose-init", "selectivly debug initialization", 1, argParser, true, "", true));
	uv_assert_err_ret(g_config->registerArgument(UVD_PROP_DEBUG_PLUGIN, 0, "verbose-plugin", "selectivly debug plugin engine", 1, argParser, true, "", true));
	uv_assert_err_ret(g_config->registerArgument(UVD_PROP_DEBUG_PRINTING, 0, "verbose-printing", "selectivly debug printing", 1, argParser, true, "", true));
	uv_assert_err_ret(g_config->registerArgument(UVD_PROP_DEBUG_PROCESSING, 0, "verbose-processing", "selectivly debug post-analysis", 1, argParser, true, "", true));
	uv_assert_err_ret(g_config->registerArgument(UVD_PROP_DEBUG_FILE, 0, "debug-file", "debug output (default: stdout)", 1, argParser, true, "", true));

	//So we can ignore arg meant for later
	uv_assert_err_ret(g_config->registerDefaultArgument(argParser, "", 0, true, true, true));

	//Early config parsing for debugging, especially during plugin loading/initialization
	UVDArgConfig::process(m_earlyConfigArgs, config->m_args, false);

	//FIXME XXX TODO: hack until I can have startup config files
	m_dirs.push_back("../lib/plugin");
	m_toLoad.push_back("uvdasm");
	m_toLoad.push_back("uvdbfd");
	m_toLoad.push_back("uvdobjbin");

	return UV_ERR_OK;
}
