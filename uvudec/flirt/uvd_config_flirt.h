/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under terms of the three clause BSD license, see LICENSE for details
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
	uint32_t m_signatureLengthMin;
	//UVD_PROP_FLIRT_MAX_SIGNATURE_LENGTH
	uint32_t m_signatureLengthMax;
	//UVD_PROP_FLIRT_PAT_NEWLINE
	std::string m_patternFileNewline;
	//UVD_PROP_FLIRT_RELOCATION_NEARFAR
	uint32_t m_relocateNearFar;

	//Files we want to parse in
	std::vector<std::string> m_targetFiles;
	//File to write to
	std::string m_outputFile;
};

#endif
