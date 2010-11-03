/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

/*
FIXME: this file is hackish, but provides a basis for improvement as needed later
For example, error messages will be very non-intuitive
	unrecognized argument instead of config file and line
*/

#include "uvd/config/config.h"
#include "uvd/config/file.h"
#include "uvd/util/util.h"
#include "uvd/config.h"
#ifdef USING_JANSSON
#include <jansson.h>
#endif

UVDConfigFileLoader::UVDConfigFileLoader()
{
}

UVDConfigFileLoader::~UVDConfigFileLoader()
{
}

uv_err_t UVDConfigFileLoader::init(UVDConfig *config)
{
	uv_assert_ret(config);
	uv_assert_err_ret(earlyArgParse(config));
	printf_args_debug("init done\n");
	return UV_ERR_OK;
}

uv_err_t UVDConfigFileLoader::earlyArgParse(UVDConfig *config)
{
	uv_err_t rc = UV_ERR_GENERAL;
	std::string homeDir;
	std::string programRcFile;
	std::string userRcFile;

	printf_args_debug("file early arg parse\n");
	/*
	Load default configuration shipped with program
	*/
	programRcFile = g_config->m_installDir + "/config";
	rc = loadFile(programRcFile);
	printf_args_debug("finished first\n");
	if( UV_FAILED(rc) && rc != UV_ERR_NOTSUPPORTED )
	{
		printf_args_debug("abort: %d\n", rc);
		return UV_DEBUG(rc);
	}

	/*
	Load user customizations
	We load it after default config so we can ovveride stuff
	*/
	uv_assert_err_ret(UVDGetHomeDir(homeDir));
	userRcFile = homeDir + "/.uvudec/config";
	rc = loadFile(userRcFile);
	if( UV_FAILED(rc) && rc != UV_ERR_NOTSUPPORTED )
	{
		return UV_DEBUG(rc);
	}

	printf_args_debug("done arg parse\n");
	return UV_ERR_OK;
}

uv_err_t UVDConfigFileLoader::loadFile(const std::string &file)
{
	UVDConfigFile *configFile = NULL;
	uv_err_t rc = UV_ERR_GENERAL;
	
	configFile = new UVDConfigFile();
	uv_assert_ret(configFile);
	rc = configFile->init(file);
	if( UV_FAILED(rc) )
	{
		printf_args_debug("config file failed init\n");
		delete configFile;
		
		if( rc == UV_ERR_NOTSUPPORTED )
		{
			return UV_ERR_NOTSUPPORTED;
		}
		return UV_DEBUG(rc);
	}
	
	m_configFiles.insert(configFile);

	return UV_ERR_OK;
}

/*
UVDConfigFile
*/

UVDConfigFile::UVDConfigFile()
{
}

UVDConfigFile::~UVDConfigFile()
{
}

uv_err_t UVDConfigFile::init(const std::string &filename)
{

#if 0
//#ifdef USING_JANSSON
#if 0
	/*
	This validates as valid JSON on http://www.jsonlint.com/
	I'm not too familar with JSON and Jansson, but it looks like I can iterate over it to do this
	Jansson has key/value support, but I guess its not required
	How to represent default argument?  
		Is there a None/null type?
			yes it has null
			 "address": null,
		Guess I'll have to do empty string for now
	
	
	*/
	std::vector<std::string> configArgs;
	std::string fileContent;
	json_t *root = NULL;
	json_error_t error;

	printf_args_debug("config file init (JSON supported), reading %s\n", filename.c_str());
	uv_assert_err_ret(readFile(filename, fileContent));
	printf_args_debug("read file\n");
	root = json_loads(fileContent.c_str(), &error);

	if( !root )
	{
		printf_error("%s: error on line %d: %s\n", filename.c_str(), error.line, error.text);
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	printf_args_debug("got root\n");
	
	/*
	take 2
	[
		"plugin=uvdasm",
		"plugin=uvdbfd",
		"plugin2=uvdobjbin"
	]
	*/	
printf_args_debug("root type: %d\n", json_typeof(root));
	if( !json_is_array(root) )
	{
		printf_error("root is not an array\n");
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	printf_args_debug("root okay\n");
	for( unsigned int i = 0; i < json_array_size(root); ++i )
	{
		std::string kv;
		const char *value_str = NULL;
		json_t *value = NULL;
		
		value = json_array_get(root, i);
		uv_assert_ret(value);		
		uv_assert_ret(json_is_string(value));
		value_str = json_string_value(value);

		kv = std::string("--") + value_str;
		printf_args_debug("arg: %s\n", kv.c_str());
		configArgs.push_back(kv);
	}

#elif 0
	/*
	Only returned the first plugin.name, so this didn't work
	{
		"property": "value",
		"plugin.name": "uvdasm",
		"plugin.name": "uvdbfd",
		"plugin.name": "uvdobjbin"
	}
	*/
	if( !json_is_object(root) )
	{
		printf_error("root is not an object\n");
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	printf_args_debug("root okay\n");
	for( void *rootIter = json_object_iter(root);
			rootIter; rootIter = json_object_iter_next(root, rootIter) )
	{
		char buff[1024];
		std::string kv;
		const char *key_str = NULL;
		json_t *value = NULL;
		
		key_str = json_object_iter_key(rootIter);
		value = json_object_iter_value(rootIter);
		if( value )
		{
			const char *value_str = NULL;
			
			uv_assert_ret(json_is_string(value));
			value_str = json_string_value(value);

			snprintf(buff, sizeof(buff), "--%s=%s", key_str, value_str);
		}
		//Default form
		else
		{
			snprintf(buff, sizeof(buff), "--%s", key_str);
		}
		kv = buff;
		printf_args_debug("arg: %s\n", kv.c_str());
		configArgs.push_back(kv);
	}
#endif

	//Combine, skipping over program name
	g_config->m_argsEffective.insert(g_config->m_argsEffective.begin() + 1, configArgs.begin(), configArgs.end());

	json_decref(root);
	return UV_ERR_OK;
#else
	printf_args_debug("not using / compiled with JSON support\n");
	/*
	Okay I tried to play nice, now its time to get work done and go back to this later
	*/
	std::string fileContent;
	std::vector<std::string> configArgs;
	std::vector<std::string> lines;

	uv_assert_err_ret(readFile(filename, fileContent));
	lines = split(fileContent, '\n', false);
	for( std::vector<std::string>::iterator iter = lines.begin(); iter != lines.end(); ++iter )
	{
		std::string line = *iter;
		
		if( line.empty() )
		{
			continue;
		}
		configArgs.push_back(std::string("--") + line);
	}

	//Combine, skipping over program name
	g_config->m_argsEffective.insert(g_config->m_argsEffective.begin() + 1, configArgs.begin(), configArgs.end());

	return UV_ERR_OK;
#endif
}

uv_err_t preprocess(const std::string in, std::string out)
{
	/*
	TODO: I'd really like comment support
	Figure something out along those lines
	-// comment if only whitespace before it on the line
	-/ * * / style comments for similar behavior
	*/
	out = in;
	
	return UV_ERR_OK;
}

