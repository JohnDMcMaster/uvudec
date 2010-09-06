/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_CONFIG_FLIRT_H
#define UVD_CONFIG_FLIRT_H

#include "uvd_types.h"
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

public:
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

	//Haven't quite figured out why it does this yet
	//Related to C++ mangling somehow?
	uint32_t m_prefixUnderscores;
};

#endif

