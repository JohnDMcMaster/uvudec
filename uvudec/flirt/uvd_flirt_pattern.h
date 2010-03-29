/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#ifndef UVD_FLIRT_PATTERN_H
#define UVD_FLIRT_PATTERN_H

#include "uvd_binary_symbol.h"
#include "uvd_data.h"

/*
A library under first analysis
To generate .pat files
Architecture and source data file dependent
*/
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
	
public:
};

#endif
