/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/architecture/registry.h"
#include "uvd/util/util.h"

/*
UVDRegisteredArchitecture
*/

UVDRegisteredArchitecture::UVDRegisteredArchitecture()
{
}

UVDRegisteredArchitecture::~UVDRegisteredArchitecture()
{
}

/*
UVDArchitectureRegistry
*/

UVDArchitectureRegistry::UVDArchitectureRegistry()
{
}

UVDArchitectureRegistry::~UVDArchitectureRegistry()
{
}

uv_err_t UVDArchitectureRegistry::bestMatch(const std::string &in, std::string &out)
{
	std::vector<std::string> parts = UVDSplit(in, '.', TRUE);
	UVDRegisteredArchitecture &curArchitectureRegistry = m_architectures;
	
	out = "";
	//Nothing left? Matches this as a terminal node
	if( parts.empty() )
	{
		return UV_ERR_OK;
	}
	
	for( std::vector<std::string>::iterator partsIter = parts.begin(); partsIter != parts.end(); ++partsIter )
	{
		//Partial match?
		std::string curPart = *partsIter;
		if( curArchitectureRegistry.m_architectures.find(curPart) == curArchitectureRegistry.m_architectures.end() )
		{
			//Best match
			return UV_ERR_OK;
		}
	
		//Need to recurse tree then
		curArchitectureRegistry = (*curArchitectureRegistry.m_architectures.find(curPart)).second;
	}
	
	//No match? Empty then
	return UV_ERR_OK;
}

uv_err_t UVDArchitectureRegistry::registerArchitecture(const std::string &architecture)
{
	std::vector<std::string> parts = UVDSplit(architecture, '.', TRUE);
	UVDRegisteredArchitecture &curArchitectureRegistry = m_architectures;
	
	//Can't register nothing
	uv_assert_ret(!parts.empty());
	
	for( std::vector<std::string>::iterator partsIter = parts.begin(); partsIter != parts.end(); ++partsIter )
	{
		//Partial match?
		std::string curPart = *partsIter;
		
		if( curArchitectureRegistry.m_architectures.find(curPart) == curArchitectureRegistry.m_architectures.end() )
		{
			//Create final parts then
			do
			{
				curPart = *partsIter;
				curArchitectureRegistry.m_architectures[curPart] = UVDRegisteredArchitecture();
				curArchitectureRegistry = (*curArchitectureRegistry.m_architectures.find(curPart)).second;
				++partsIter;
			} while( partsIter != parts.end() );
			return UV_ERR_OK;
		}
	
		//Need to recurse tree then
		curArchitectureRegistry = (*curArchitectureRegistry.m_architectures.find(curPart)).second;
	}
	
	//If we used up all of the parts, it was already registered
	//Don't debug report errors so we can use for less agressive function
	return UV_ERR_GENERAL;
}

uv_err_t UVDArchitectureRegistry::ensureArchitectureRegistered(const std::string &architecture)
{
	//Ignore errors
	registerArchitecture(architecture);
	
	return UV_ERR_OK;
}

