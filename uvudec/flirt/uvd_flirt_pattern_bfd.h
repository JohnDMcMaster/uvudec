#ifndef UVD_FLIRT_PATTERN_BFD_H
#define UVD_FLIRT_PATTERN_BFD_H

#ifdef UVD_FLIRT_PATTERN_BFD

#include "uvd_flirt_pattern.h"

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
};

#endif

#endif
