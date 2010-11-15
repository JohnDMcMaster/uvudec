/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

/*
This file tries to provide a universal architecture description so that it can
be translated to various sub-libraries
It DOES aim to be an exaustive set, as long as there is justifiable reason why the ISA is different than another
Although this is a hard problem, it is thought this can be useful in many situations even if not completly accurate
If the system has multiple ISA, they must be analyzed separatly

8051.8052
*/

#ifndef UVD_CORE_ARCHITECTURE_H
#define UVD_CORE_ARCHITECTURE_H

#include "uvd/util/types.h"
#include <set>
#include <string>

/*
Each "dot" level has one of these
Note the base case has an empty map
*/
class UVDRegisteredArchitecture
{
public:
	UVDRegisteredArchitecture();
	~UVDRegisteredArchitecture();
		
	uv_err_t getArchitectures(const std::string &prefix, std::set<std::string> &out);

public:
	std::map<std::string, UVDRegisteredArchitecture *> m_architectures;
};

class UVDConfig;
class UVDArchitectureRegistry
{	
public:
	UVDArchitectureRegistry();
	~UVDArchitectureRegistry();
	
	uv_err_t init();
	
	//If nothing even at the base level matches, return UV_ERR_NOTFOUND
	uv_err_t bestMatch(const std::string &in, std::string &out);
	//If already registered, report error
	uv_err_t registerArchitecture(const std::string &architecture);
	//Don't return error if already present
	uv_err_t ensureArchitectureRegistered(const std::string &architecture);
	uv_err_t getArchitectures(std::set<std::string> &out);
	uv_err_t debugPrintArchitectures();
	
	std::string normalizeArchitecture(const std::string &in);

public:
	UVDRegisteredArchitecture m_architectures;
	UVDConfig *m_config;
};

#endif

