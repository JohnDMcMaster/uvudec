/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_ARG_H
#define UVD_ARG_H

#include "uvd_types.h"
#include <string>

#define UVD_OPTION_FILE_STDIN		"-"
#define UVD_OPTION_FILE_STDOUT		"/dev/stdout"
#define UVD_OPTION_FILE_STDERR		"/dev/stderr"

/*
Function to parse an argument
val: trailing argument value, if arg->m_expectedValues == 1
	ex usually from one of 
		--startOffset 0		
	would yield the string "0"
*/
class UVDArgConfig;
typedef uv_err_t (*UVDArgConfigHandler)(const UVDArgConfig *UVDArgConfig, std::vector<std::string> argumentArgument);

/*
Argument parsing for startup
Should be parsable from a config file or from command line

--key=val is preferred form
--defauledKey will, for example, assume you left off the = for a default and not check the next val
*/
class UVDArgConfig
{
public:
	UVDArgConfig();
	UVDArgConfig(const std::string &propertyForm,
			char shortForm, std::string longForm, 
			std::string helpMessage,
			uint32_t numberExpectedValues,
			UVDArgConfigHandler handler,
			bool hasDefault);
	UVDArgConfig(const std::string &propertyForm,
			char shortForm, std::string longForm, 
			std::string helpMessage,
			std::string helpMessageExtra,
			uint32_t numberExpectedValues,
			UVDArgConfigHandler handler,
			bool hasDefault);
	~UVDArgConfig();

	bool isNakedHandler() const;
	//bool operator ==(const std::string &r) const;

	/*
	Do a main style parse, eliminating used args
	-abc or -a -b -c
		if num args == 0 for each
	-d arg or -darg
		if num args == 1
	--arg=val or --arg val
	*/
	static uv_err_t process(const std::vector<UVDArgConfig *> &argConfig, std::vector<std::string> &args);

public:
	/*
	The argument property we are looking for in full form
	ex: format.displayWidth
	must contain at least one . to distinguish it from short form
		and is good for organizational purposes anyways
	if blank, will take args with no - prefixes or setup
	*/
	std::string m_propertyForm;
	//Single letter -X form, without -
	char m_shortForm;
	//Longer letters --my-arg form, without --
	std::string m_longForm;
	//Briefly what the arg does
	std::string m_helpMessage;
	//If it has enum vals, we need to print those as well.  One new line and expected to be newline terminated
	std::string m_helpMessageExtra;
	//Number trailing arguments, prob should be only 0 or 1
	uint32_t m_numberExpectedValues;
	//What to call when found
	UVDArgConfigHandler m_handler;
	//If set, an argument of the form --key will accept even if required arguments is greater
	//callback should expect this and create the appropriete default itself
	bool m_hasDefault;
};

#define UVD_ARG_FORM_UNKNOWN			0
#define UVD_ARG_FORM_LONG				1
#define UVD_ARG_FORM_SHORT				2
//--stuff.other
#define UVD_ARG_FORM_PROPERTY			3
//Not a -'d argument, just by self
#define UVD_ARG_FORM_NAKED				4

/*
An argument we've parsed and are trying to decode
*/
class UVDParsedArg
{
public:
	UVDParsedArg()
	{
		m_keyForm = UVD_ARG_FORM_UNKNOWN;
		m_embeddedValPresent = false;
	}

public:
	//Where we got it from
	std::string m_raw;
	//The UVD_ARG_FORM_* value
	uint32_t m_keyForm;
	//Our key.  Should not have any -'s
	std::string m_key;
	//If we have a --key=val form
	bool m_embeddedValPresent;
	//The val part of above
	std::string m_embeddedVal;
};

//uv_err_t argParserDefault(const UVDArgConfig *argConfig, std::vector<std::string> argumentArguments);
uv_err_t UVDInitConfigEarly();
uv_err_t UVDInitConfig();
void UVDHelp();

#endif
