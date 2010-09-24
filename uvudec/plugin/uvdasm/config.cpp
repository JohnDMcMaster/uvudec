/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvdasm/config.h"
#include "uvdasm/plugin.h"
#include "uvd_arg_property.h"
#include "uvd_language.h"

UVDAsmConfig *g_asmConfig = NULL;

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

	/*
	Startup configuration
	TODO: add a config file path var
	*/
	if( argConfig->m_propertyForm == UVD_PROP_CONFIG_LANGUAGE )
	{
		uv_assert_ret(!argumentArguments.empty());
		
		if( UV_FAILED(g_asmConfig->setConfigInterpreterLanguage(firstArg)) )
		{
			UVDHelp();
			return UV_DEBUG(UV_ERR_GENERAL);
		}
	}
	else if( argConfig->m_propertyForm == UVD_PROP_CONFIG_LANGUAGE_INTERFACE )
	{
		uv_assert_ret(argumentArguments.size() == 1);
		
		if( UV_FAILED(g_asmConfig->setConfigInterpreterLanguageInterface(firstArg)) )
		{
			UVDHelp();
			return UV_DEBUG(UV_ERR_GENERAL);
		}
	}
	else
	{
		printf_error("Property not recognized in callback: %s\n", argConfig->m_propertyForm.c_str());
		return UV_DEBUG(UV_ERR_GENERAL);
	}

	return UV_ERR_OK;
}


UVDAsmConfig::UVDAsmConfig()
{
	//m_plugin = NULL;
	m_configInterpreterLanguage = UVD_LANGUAGE_UNKNOWN;
	m_configInterpreterLanguageInterface = UVD_LANGUAGE_INTERFACE_UNKNOWN;
	g_asmConfig = this;
}

UVDAsmConfig::~UVDAsmConfig()
{
	UV_DEBUG(deinit());
}

uv_err_t UVDAsmConfig::init(UVDConfig *config)
{
	m_configInterpreterLanguage = UVD_CONFIG_INTERPRETER_LANGUAGE_DEFAULT;
	m_configInterpreterLanguageInterface = UVD_CONFIG_INTERPRETER_LANGUAGE_INTERFACE_DEFAULT;
	
	//What a mess
	uv_assert_err_ret(g_asmPlugin->registerArgument(UVD_PROP_ARCH_FILE, 0, "arch-file", "architecture/CPU module file", 1, argParser, true));
	//Config file processing
	uv_assert_err_ret(g_asmPlugin->registerArgument(UVD_PROP_CONFIG_LANGUAGE, 0, "config-language",
			"default config interpreter language (plugins may require specific)", 
			""
#ifdef USING_LUA
			"\tlua: use Lua"
#if UVD_CONFIG_INTERPRETER_LANGUAGE_DEFAULT == UVD_LANGUAGE_LUA
				" (default)"
#endif
				"\n"
#endif //USING_LUA
#ifdef USING_PYTHON
			"\tpython: use Python"
#if UVD_CONFIG_INTERPRETER_LANGUAGE_DEFAULT == UVD_LANGUAGE_PYTHON
				" (default)"
#endif
				"\n"
#endif //USING_PYTHON
#ifdef USING_JAVASCRIPT
			"\tjavascript: use javascript"
#if UVD_CONFIG_INTERPRETER_LANGUAGE_DEFAULT == UVD_LANGUAGE_JAVASCRIPT
				" (default)"
#endif
				"\n"
#endif //USING_JAVASCRIPT
			,
			1, argParser, true));
	uv_assert_err_ret(g_asmPlugin->registerArgument(UVD_PROP_CONFIG_LANGUAGE_INTERFACE, 0, "config-language-interface",
			"how to access a specifc interpreter (options as availible)", 
			"\texec: execute interpreter, parse results"
#if UVD_CONFIG_INTERPRETER_LANGUAGE_INTERFACE_DEFAULT == UVD_LANGUAGE_INTERFACE_EXEC
				" (default)"
#endif
				"\n"
			"\tAPI: use binary API to interpreter"
#if UVD_CONFIG_INTERPRETER_LANGUAGE_INTERFACE_DEFAULT == UVD_LANGUAGE_INTERFACE_API
				" (default)"
#endif
				"\n",
			1, argParser, true));
	

	return UV_ERR_OK;
}

uv_err_t UVDAsmConfig::deinit()
{
	return UV_ERR_OK;
}

uv_err_t UVDAsmConfig::setConfigInterpreterLanguage(const std::string &in)
{
	//To make selection pre-processable
	if( false )
	{
	}
#ifdef USING_LUA
	else if( in == "lua" )
	{
		m_configInterpreterLanguage = UVD_LANGUAGE_LUA;
	}
#endif //USING_LUA
#ifdef USING_PYTHON
	else if( in == "python" )
	{
		m_configInterpreterLanguage = UVD_LANGUAGE_PYTHON;
	}
#endif //USING_PYTHON
#ifdef USING_JAVASCRIPT
	else if( in == "javascript" )
	{
		m_configInterpreterLanguage = UVD_LANGUAGE_JAVASCRIPT;
	}
#endif //USING_PYTHON
	else
	{
		printf_error("unknown language: <%s>\n", in.c_str());
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	return UV_ERR_OK;
}

uv_err_t UVDAsmConfig::setConfigInterpreterLanguageInterface(const std::string &in)
{
	if( in == "exec" )
	{
		m_configInterpreterLanguageInterface = UVD_LANGUAGE_INTERFACE_EXEC;
	}
	else if( in == "api" || in == "API" )
	{
		m_configInterpreterLanguageInterface = UVD_LANGUAGE_INTERFACE_API;
	}
	else
	{
		printf_error("unknown language interface: <%s>\n", in.c_str());
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	return UV_ERR_OK;
}

