/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd_arg.h"
#include "uvd_arg_util.h"
#include <vector>
#include <string.h>

uv_err_t parseFileOption(const std::string optionFileIn, FILE **pOptionFileIn)
{
	std::string optionFile = optionFileIn;
	FILE *pOptionFile = NULL;

//printf("translating FILE * from file %s\n", optionFileIn.c_str());

	//Assume blank means discard
	if( optionFile == "" )
	{
		optionFile = "/dev/null";
	}

	//Translate file into a I/O structure
	if( optionFile == UVD_OPTION_FILE_STDOUT )
	{
		pOptionFile = stdout;
	}
	else if( optionFile == UVD_OPTION_FILE_STDERR )
	{
		pOptionFile = stderr;
	}
	else
	{
		pOptionFile = fopen(optionFile.c_str(), "w");
	}
	uv_assert_ret(pOptionFile);

	//Save the file pointer we got
	uv_assert_ret(pOptionFileIn);
	*pOptionFileIn = pOptionFile;

	return UV_ERR_OK;
}

bool UVDArgToBool(const std::string &sArg)
{
	if( strcmp(sArg.c_str(), "true") == 0 )
	{
		return true;
	}
	else if( strcmp(sArg.c_str(), "false") == 0 )
	{
		return false;
	}
	else if( sArg == "0" )
	{
		return false;
	}
	else if( sArg == "1" )
	{
		return true;
	}
	else
	{
		//Default...should error?
		return true;
	}
}

/*
short forms return all short args as the key
*/
uv_err_t processArgCore(const std::string &arg, UVDParsedArg &parsedArg)
{
	parsedArg.m_keyForm = UVD_ARG_FORM_UNKNOWN;
	parsedArg.m_embeddedValPresent = false;
	parsedArg.m_raw = arg;

	//See if we can ID what form it is
	if( arg[0] != '-' )
	{
		parsedArg.m_keyForm = UVD_ARG_FORM_NAKED;
		parsedArg.m_key = arg;
	}
	//We have at least a starting -
	else
	{
		//Do we have a long form?
		if( arg[1] == '-' )
		{
			parsedArg.m_keyForm = UVD_ARG_FORM_LONG;
			//a --key=val form?
			std::string::size_type pos = arg.find('=');
			if( pos != std::string::npos )
			{
				parsedArg.m_embeddedVal = arg.substr(pos + 1);
				/*
				-	-	k	e	y	=	v	a	l
				0	1	2	3	4	5	6	7	8
									pos
				*/
				parsedArg.m_key = arg.substr(2, pos - 2);
				parsedArg.m_embeddedValPresent = true;
			}
			//--key
			else
			{
				parsedArg.m_key = arg.substr(2);
			}
			
			//But is it a true long form or is it a full property name?
			if( parsedArg.m_key.find('.') != std::string::npos )
			{
				parsedArg.m_keyForm = UVD_ARG_FORM_PROPERTY;
			}
		}
		//Short form then
		//Return all short chars
		else
		{
			parsedArg.m_key = arg.substr(1);
		}
	}

	return UV_ERR_OK;
}

uv_err_t processArg(const std::string &arg, std::vector<UVDParsedArg> &parsedArgs)
{
	UVDParsedArg parsedArg;
	uv_assert_err_ret(processArgCore(arg, parsedArg));
	
	parsedArgs.clear();
	//Do we need to split this up due to packed short args?
	if( parsedArg.m_keyForm == UVD_ARG_FORM_SHORT && parsedArg.m_key.size() > 1 )
	{
		for( std::string::size_type i = 0; i < parsedArg.m_key.size(); ++i )
		{
			UVDParsedArg parsedArgTemp = parsedArg;
			//Make single char to reduce to single arg
			parsedArg.m_key = parsedArg.m_key[i];
			parsedArgs.push_back(parsedArgTemp);
		}
	}
	else
	{
		parsedArgs.push_back(parsedArg);
	}
	
	return UV_ERR_OK;
}

uv_err_t matchArgConfig(const std::vector<UVDArgConfig *> &argConfigs, UVDParsedArg &arg, UVDArgConfig const**matchedArgConfig)
{
	uv_assert_ret(matchedArgConfig);
	for( std::vector<UVDArgConfig>::size_type argConfigsIndex = 0; 
			argConfigsIndex < argConfigs.size(); ++argConfigsIndex )
	{
		const UVDArgConfig *argConfig = argConfigs[argConfigsIndex];
		uv_assert_ret(argConfig);
		
		switch( arg.m_keyForm )
		{
		case UVD_ARG_FORM_LONG:
			if( arg.m_key == argConfig->m_longForm )
			{
				*matchedArgConfig = argConfig;
				return UV_ERR_OK;
			}
			break;
		case UVD_ARG_FORM_SHORT:
			if( arg.m_key[0] == argConfig->m_shortForm )
			{
				*matchedArgConfig = argConfig;
				return UV_ERR_OK;
			}
			break;
		case UVD_ARG_FORM_PROPERTY:
			if( arg.m_key == argConfig->m_propertyForm )
			{
				*matchedArgConfig = argConfig;
				return UV_ERR_OK;
			}
			break;
		case UVD_ARG_FORM_NAKED:
			if( argConfig->isNakedHandler() )
			{
				*matchedArgConfig = argConfig;
				return UV_ERR_OK;
			}
			break;
		default:
			return UV_DEBUG(UV_ERR_GENERAL);
		}
	}
	
	//This is fatal since an unknown arg could have any number of required extra params
	//ex: should we assume --arg param or just --arg?  param could be a default option such as our target file
	printf_error("Unknown option: <%s>\n", arg.m_raw.c_str());
	UVDHelp();
	return UV_DEBUG(UV_ERR_GENERAL);
}

