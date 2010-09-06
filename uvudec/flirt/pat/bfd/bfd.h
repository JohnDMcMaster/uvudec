/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_FLIRT_PATTERN_BFD_H
#define UVD_FLIRT_PATTERN_BFD_H

#ifdef UVD_FLIRT_PATTERN_BFD

#include "uvd_types.h"
#include "uvd_flirt_pattern.h"
#include <string>
#include <bfd.h>

/*
Using libbfd to get symbols and relocations
*/
class UVDFLIRTPatternGeneratorBFD : public UVDFLIRTPatternGenerator
{
public:
	UVDFLIRTPatternGeneratorBFD();
	~UVDFLIRTPatternGeneratorBFD();
	uv_err_t init();
	uv_err_t deinit();
	
	virtual uv_err_t canGenerate(const std::string &file);
	virtual uv_err_t saveToStringCore(const std::string &inputFile, std::string &output);
	
	static uv_err_t getPatternGenerator(UVDFLIRTPatternGeneratorBFD **generatorOut);

protected:
	uv_err_t generateByBFD(bfd *abfd, std::string &output);
	uv_err_t generateByFile(const std::string &fileName, std::string &output);
};

#endif

#endif
