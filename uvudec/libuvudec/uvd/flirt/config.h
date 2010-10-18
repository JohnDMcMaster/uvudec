/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_CONFIG_FLIRT_H
#define UVD_CONFIG_FLIRT_H

#include "uvd/util/types.h"
#include <string>

/*
Configuration entries related to FLIRT
flirt.* entries
*/
class UVDConfigFLIRT
{
public:
	UVDConfigFLIRT();
	~UVDConfigFLIRT();
	uv_err_t deinit();
	
	//Set parameters that FLAIR cannot set to what FLAIR would use
	uv_err_t makeFLAIRCompatible(bool makeCompatible = true);

public:
	//UVD_PROP_FLIRT_FLAIR_COMPATIBILITY
	uint32_t m_FLAIRCompatibility;
	//UVD_PROP_FLIRT_MIN_SIGNATURE_LENGTH
	uint32_t m_patSignatureLengthMin;
	//UVD_PROP_FLIRT_MAX_SIGNATURE_LENGTH
	uint32_t m_patSignatureLengthMax;
	//UVD_PROP_FLIRT_PAT_NEWLINE
	std::string m_patternFileNewline;
	//UVD_PROP_FLIRT_RELOCATION_NEARFAR
	uint32_t m_relocateNearFar;
	//UVD_PROP_FLIRT_PAT_LEADING_LENGTH
	uint32_t m_patLeadingLength;
	//UVD_PROP_FLIRT_PAT_MODULE_LENGTH_MAX_DEFAULT
	uint32_t m_patModuleLengthMax;
	//UVD_PROP_FLIRT_PAT_FUNCTIONS_AS_MODULES
	uint32_t m_functionsAsModules;
	
	/*
	Triming heuristics
	*/
	/*
	On object files, we may not have a parser availibe to do recursive descent
	Instead, we may have to employ some heuristics to remove padding
	Other common values are 0xFF and 0x7F, so we may consider adding a list of trim values instead
	*/
	uint32_t m_trimZeros; 
	
	//Files we want to parse in
	std::vector<std::string> m_targetFiles;
	//File to write to
	std::string m_outputFile;

	//When debug dumping, what to append for an indent
	std::string m_debugDumpTab;

	//Haven't quite figured out why it does this yet
	//Related to C++ mangling somehow?
	//UVD_PROP_FLIRT_PAT_PREFIX_UNDERSCORES
	uint32_t m_prefixUnderscores;
};

#endif

