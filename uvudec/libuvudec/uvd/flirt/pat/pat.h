/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_FLIRT_PATTERN_H
#define UVD_FLIRT_PATTERN_H

#include "uvd/assembly/symbol.h"
#include "uvd/data/data.h"

/*
A library under first analysis
To generate .pat files
Architecture and source data file dependent
*/
class UVDFLIRTFunction;
class UVDFLIRTPatternGenerator
{
public:
	UVDFLIRTPatternGenerator();
	virtual ~UVDFLIRTPatternGenerator();
	uv_err_t deinit();
	
	/*
	Save .pat to given file
	*/
	uv_err_t saveToFile(const std::string &inputFile, const std::string &file);
	//Core version providing a text dump of the .pat file
	//terminate file: add the ending ---
	uv_err_t saveToString(const std::string &inputFile, std::string &output, uint32_t terminateFile);
	//Gives only the lines, ie no ending terminator
	//Do this so we can combine several object file signatures together nicely
	virtual uv_err_t saveToStringCore(const std::string &inputFile, std::string &output) = 0;
	
	//Can we generate a .pat for the given object file?
	virtual uv_err_t canGenerate(const std::string &file) = 0;
		
	//Start analysis, chosing the best analyzer object from detected format
	//static uv_err_t getPatternGenerator(const std::string &file, UVDFLIRTPatternGenerator **generatorOut);
	
	static uv_err_t patLineToFunction(const std::string &in, UVDFLIRTFunction *out);
	static bool isValidPatBytePattern(const std::string in);
	
public:
};

#define UVD_FLIRT_PAT_TERMINATOR					"---"
#define UVD_FLIRT_PAT_RELOCATION_CHAR				'.'
#define UVD_FLIRT_PAT_PUBLIC_NAME_CHAR				':'
#define UVD_FLIRT_PAT_REFERENCED_NAME_CHAR			'^'
//appended to public name entries after offset to indicate it shouldn't be used outside of the object file
#define UVD_FLIRT_PAT_LOCAL_SYMBOL_CHAR				'@'

#define UVD_FLIRT_PAT_CRC_LEN_MAX					0xFF

#endif

