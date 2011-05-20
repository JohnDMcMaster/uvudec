/*
UVNet Universal Decompiler (uvudec)
Copyright 2011 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_RAW_ARG_H
#define UVD_RAW_ARG_H

#include <string>
#include <vector>
#include "uvd/util/types.h"


//Nothing was specified
//#define UVD_ARG_FROM_NONE				1
//Default value was given, is there really a difference between this and above?
//Config values should be light weight enough that none should be default
#define UVD_ARG_FROM_DEFAULT			2
#define UVD_ARG_FROM_CONFIG				3
#define UVD_ARG_FROM_COMMAND_LINE		4
//#define UVD_ARG_FROM_UNKNOWN			5


/*
From command line, file, etc
Gives traceability
*/
class UVDRawArg {
public:
	UVDRawArg();
	~UVDRawArg();
	
	//Get human readable description of where this came from
	//virtual uv_err_t getSourceStr(std::string &out) = 0;
	//Return an integer identifier describing the general idea
	//UVD_ARG_FROM_*
	virtual unsigned int getSource() = 0;

public:
	std::string m_token;
};

class UVDRawArgs;
class UVDCommandLineArg : public UVDRawArg {
public:
	UVDCommandLineArg();
	virtual ~UVDCommandLineArg();
	
	//virtual uv_err_t getSource(std::string &out);
	virtual unsigned int getSource();

	//set ignore first for argc/argv based
	//sets all to have getSource() return UVD_ARG_FROM_COMMAND_LINE
	//Appends, not clobbers
	static uv_err_t fromStringVector( const std::vector<std::string> in, UVDRawArgs &out, bool skipFirst = true );
};

class UVDConfigFileArg : public UVDRawArg {
public:
	UVDConfigFileArg();
	virtual ~UVDConfigFileArg();
	
	//virtual uv_err_t getSource(std::string &out);
	virtual unsigned int getSource();

public:	
	//File this came from
	std::string m_fileName;
	unsigned int m_fileLine;
};

class UVDRawArgs {
public:
	UVDRawArgs();
	~UVDRawArgs();
		
public:
	std::vector<UVDRawArg *> m_args;
};

#endif

