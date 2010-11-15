/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/architecture/registry.h"
#include "uvd/config/config.h"
#include "uvd/util/util.h"

/*
UVDRegisteredArchitecture
*/

UVDRegisteredArchitecture::UVDRegisteredArchitecture()
{
}

UVDRegisteredArchitecture::~UVDRegisteredArchitecture()
{
	for( std::map<std::string, UVDRegisteredArchitecture *>::iterator iter = m_architectures.begin();
			iter != m_architectures.end(); ++iter )
	{
		delete (*iter).second;
	}
}

uv_err_t UVDRegisteredArchitecture::getArchitectures(const std::string &prefixIn, std::set<std::string> &out)
{
	for( std::map<std::string, UVDRegisteredArchitecture *>::iterator iter = m_architectures.begin();
			iter != m_architectures.end(); ++iter )
	{
		std::string prefix = prefixIn + (*iter).first;
		out.insert(prefix);
		uv_assert_err_ret((*iter).second->getArchitectures(prefix + ".", out));
	}
	return UV_ERR_OK;
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

uv_err_t UVDArchitectureRegistry::init()
{
	std::string dataDir;
	std::string archFileName;
	std::vector<std::string> lines;
	std::string archFileContents;

	uv_assert_ret(m_config);
	uv_assert_err_ret(m_config->getDataDir(dataDir));
	archFileName = dataDir + "/architectures.txt";
	uv_assert_err_ret(UVDReadFileByString(archFileName, archFileContents));
	lines = UVDSplitLines(archFileContents);
	for( std::vector<std::string>::size_type i = 0; i < lines.size(); ++i )
	{
		std::string lineRaw = lines[i];
		std::string line;
		
		if( uvdPreprocessLine(lineRaw, line) == UV_ERR_BLANK )
		{
			continue;
		}
		uv_assert_err_ret(registerArchitecture(line));
	}

	//uv_assert_err_ret(debugPrintArchitectures());

	return UV_ERR_OK;
}

uv_err_t UVDArchitectureRegistry::debugPrintArchitectures()
{
	std::set<std::string> architectures;
	
	uv_assert_err_ret(getArchitectures(architectures));
	printf("Architectures:\n");
	for( std::set<std::string>::iterator iter = architectures.begin(); iter != architectures.end(); ++iter )
	{
		printf("\t%s\n", (*iter).c_str());
	}

	return UV_ERR_OK;
}

uv_err_t UVDArchitectureRegistry::getArchitectures(std::set<std::string> &out)
{
	out.clear();
	return UV_DEBUG(m_architectures.getArchitectures("", out));
}

uv_err_t UVDArchitectureRegistry::bestMatch(const std::string &in, std::string &out)
{
	std::vector<std::string> parts = UVDSplit(in, '.', TRUE);
	UVDRegisteredArchitecture *curArchitectureRegistry = &m_architectures;
	
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
		if( curArchitectureRegistry->m_architectures.find(curPart) == curArchitectureRegistry->m_architectures.end() )
		{
			//Best match
			return UV_ERR_OK;
		}
	
		//Need to recurse tree then
		curArchitectureRegistry = (*curArchitectureRegistry->m_architectures.find(curPart)).second;
	}
	
	//No match? Empty then
	return UV_ERR_OK;
}

uv_err_t UVDArchitectureRegistry::registerArchitecture(const std::string &architecture)
{
	std::vector<std::string> parts = UVDSplit(architecture, '.', TRUE);
	UVDRegisteredArchitecture *curArchitectureRegistry = &m_architectures;
	
	//Can't register nothing
	uv_assert_ret(!parts.empty());
	
	for( std::vector<std::string>::iterator partsIter = parts.begin(); partsIter != parts.end(); ++partsIter )
	{
		//Partial match?
		std::string curPart = *partsIter;
		
		if( curArchitectureRegistry->m_architectures.find(curPart) == curArchitectureRegistry->m_architectures.end() )
		{
			//Create final parts then
			do
			{
				curPart = *partsIter;
				curArchitectureRegistry->m_architectures[curPart] = new UVDRegisteredArchitecture();
				curArchitectureRegistry = (*curArchitectureRegistry->m_architectures.find(curPart)).second;
				++partsIter;
			} while( partsIter != parts.end() );
			return UV_ERR_OK;
		}
	
		//Need to recurse tree then
		curArchitectureRegistry = (*curArchitectureRegistry->m_architectures.find(curPart)).second;
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

