/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_ARG_H
#define UVD_ARG_H

#include "uvd/util/types.h"
#include <set>
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
typedef uv_err_t (*UVDArgConfigHandler)(const UVDArgConfig *UVDArgConfig, std::vector<std::string> argumentArgument, void *user);

//Map of property to arg config
//Naked handler has empty property
class UVDArgConfig;

/*
A group of UVDArgConfigs that represents a logical zone where an argument is valid
This class should not have any knowledge of UVD engine specifics
It should be as if it was in a separate argument parsing library
*/
class UVDArgConfigs
{
public:
	typedef std::map<std::string, UVDArgConfig *> ArgConfigs;

public:
	UVDArgConfigs();
	~UVDArgConfigs();
	
	uv_err_t registerDefaultArgument(UVDArgConfigHandler handler,
			const std::string &helpMessage,
			uint32_t minRequired,
			bool combine,
			bool alwaysCall,
			void *user);

	uv_err_t registerArgument(const std::string &propertyForm,
			char shortForm, std::string longForm, 
			std::string helpMessage,
			std::string helpMessageExtra,
			uint32_t numberExpectedValues,
			UVDArgConfigHandler handler,
			bool hasDefault,
			void *user);

	uv_err_t printUsage(const std::string &indent = "");

public:
	ArgConfigs m_argConfigs;
};

/*
Collection of all of the arg configs
Allows arguments to be cleanly tracked even if they need to be parsed in multiple phases
*/
class UVDArgRegistry
{
public:
	UVDArgRegistry();
	//Convenience for simple programs that only have one arg pass
	UVDArgRegistry(UVDArgConfigs *args);
	~UVDArgRegistry();
	
	uv_err_t printUsage(const std::string &indent = "");
	uv_err_t newArgConfgs(UVDArgConfigs **out = NULL);
	uv_err_t processMain(int argc, char *const *argv);
	uv_err_t processMainWithRemove(int argc, char **argv);
	uv_err_t processStringVector(const std::vector<std::string> &args);
	//Remove processed args
	//Will not return error on unknown args, instead !args.empty()
	uv_err_t processStringVectorWithRemove(std::vector<std::string> &args);
	
public:
	//TODO: define ordering on this, probably by property
	std::set<UVDArgConfigs *> m_argConfigsSet;
};

/*
Argument parsing for startup
Should be parsable from a config file or from command line

--key=val is preferred form
--defauledKey will, for example, assume you left off the = for a default and not check the next val

FIXME: we need to subclass this or something
I think we only deal with pointers to it anyway
*/
class UVDArgConfig
{
public:
	UVDArgConfig();
	//For naked argumentss
	//Of course, set min required to 0 for no requirement
	UVDArgConfig(UVDArgConfigHandler handler,
			const std::string &helpMessage,
			uint32_t minRequired, 
			bool combine, 
			bool alwaysCall,
			void *user);
	UVDArgConfig(const std::string &propertyForm,
			char shortForm, std::string longForm, 
			std::string helpMessage,
			std::string helpMessageExtra,
			uint32_t numberExpectedValues,
			UVDArgConfigHandler handler,
			bool hasDefault,
			void *user);
	~UVDArgConfig();

	bool isNakedHandler() const;
	//bool operator ==(const std::string &r) const;

	uv_err_t print(const std::string &indent) const;

	/*
	Do a main style parse, eliminating used args
	-abc or -a -b -c
		if num args == 0 for each
	-d arg or -darg
		if num args == 1
	--arg=val or --arg val
	*/
	static uv_err_t process(const UVDArgConfigs &argConfig, std::vector<std::string> &args,
			bool printErrors = true, UVDArgConfigs *ignoredArgs = NULL);
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
	union
	{
		char m_shortForm;
		bool m_alwaysCall;
	};
	//Longer letters --my-arg form, without --
	std::string m_longForm;
	//Briefly what the arg does
	std::string m_helpMessage;
	//If it has enum vals, we need to print those as well.  One new line and expected to be newline terminated
	std::string m_helpMessageExtra;
	//Number trailing arguments, prob should be only 0 or 1
	//Minimum number of arguments for naked arguments, does not have to be exactly equal
	uint32_t m_numberExpectedValues;
	//What to call when found
	UVDArgConfigHandler m_handler;
	union
	{
		//If set, an argument of the form --key will accept even if required arguments is greater
		//callback should expect this and create the appropriete default itself
		bool m_hasDefault;
		//Spit out args as we go or give single vector at end?
		//Former is more control, but latter is what is usually needed and more convenient
		bool m_combine;
	};
	void *m_user;
};

/*
A --arg, -a, --adsf.adsf=val, etc type args
*/
class UVDPropertyArgConfig : public UVDArgConfig
{
public:
};

/*
Handler policy for arguments with no - prefix
We might make this more advanced to handle positional parameters, but right now is singleton
*/
class UVDNakedArgConfig : public UVDArgConfig
{
public:
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
	//Also if this is a naked arg, we store this here since they are somewhat indistinguishable
	std::string m_key;
	//If we have a --key=val form
	bool m_embeddedValPresent;
	//The val part of above
	std::string m_embeddedVal;
};

//uv_err_t UVDInitArgConfig();
void UVDPrintVersion();

#endif

