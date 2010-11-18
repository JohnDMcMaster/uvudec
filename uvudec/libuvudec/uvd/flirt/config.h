/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_CONFIG_FLIRT_H
#define UVD_CONFIG_FLIRT_H

#include "uvd/util/types.h"
#include <set>
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
	//UVD_PROP_FLIRT_PAT_MODULE_LENGTH_MAX
	uint32_t m_patModuleLengthMax;
	//UVD_PROP_FLIRT_PAT_FUNCTIONS_AS_MODULES
	uint32_t m_functionsAsModules;
	//UVD_PROP_FLIRT_PAT_REFERENCE_TRAILING_SPACE
	uint32_t m_patternReferenceTrailingSpace;
	//UVD_PROP_FLIRT_PAT_PUBLIC_NAME_LENGTH_MIN
	uint32_t m_patternPublicNameLengthMin;

	/*
	Signature creation
	*/
	//UVD_PROP_FLIRT_SIG_LIB_NAME
	std::string m_libName;
	//UVD_PROP_FLIRT_SIG_VERSION
	uint32_t m_sigVersion;
	//UVD_PROP_FLIRT_SIG_FEATURES
	uint32_t m_sigFeatures;
	//UVD_PROP_FLIRT_SIG_PAD
	uint32_t m_sigPad;
	//UVD_PROP_FLIRT_SIG_PROCESSOR_ID
	uint32_t m_sigProcessorID;
	//UVD_PROP_FLIRT_SIG_OS_TYPES
	uint32_t m_sigOSTypes;
	//UVD_PROP_FLIRT_SIG_APP_TYPES
	uint32_t m_sigAppTypes;
	//UVD_PROP_FLIRT_SIG_FILE_TYPES
	uint32_t m_sigFileTypes;
	
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
	//Related to C++ mangling somehow?  Archive packing somehow I guess
	//Listed as a general option since I might write files without them and need to parse them correctly
	//UVD_PROP_FLIRT_PAT_PREFIX_UNDERSCORES
	uint32_t m_prefixUnderscores;
	
	//For obj2pat, but needs visibility from core engine
	//Would have to think about what to do about this
	//Don't skip these, even if we think they should be skipped (ie not a function symbol)
	std::set<std::string> m_forcePatternSymbols;
};

#endif

