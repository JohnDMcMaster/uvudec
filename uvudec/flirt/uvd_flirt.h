/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

/*
Fast Library Recognition Technology (FLIRT) implementation
Given a library, produce signature files that can be used to 
Based off of paper on Data Rescue's website
http://www.hex-rays.com/idapro/flirt.htm
*/

#ifndef UVD_FLIRT_H

#include "uvd.h"
#include "uvd_flirt_pattern.h"
#include "uvd.h"
#include <string>
#include <vector>

/*
Core FLIRT engine
*/
class UVDFLIRT
{
public:
	UVDFLIRT();
	~UVDFLIRT();
	uv_err_t init();
	uv_err_t deinit();
	
	//Automatically chose object format and engine
	uv_err_t objs2patFile(const std::vector<std::string> &inputFiles, const std::string &outputFile);
	uv_err_t objs2pat(const std::vector<std::string> &inputFiles, std::string &output);

	//Don't do a full UVD engine init, only grab the FLIRT component
	//Since so much would break without a basic UVD engine, for now a basic UVD engine with no opcode caps is loaded
	//The way those translations are done should prob be re-delegated anyway
	static uv_err_t getFLIRT(UVDFLIRT **out);

protected:
	//Find a capable analyzer
	uv_err_t getPatternGenerator(const std::string &file, UVDFLIRTPatternGenerator **generator);

public:
	UVD *m_uvd;
	//Pattern generation engines
	std::vector<UVDFLIRTPatternGenerator *> m_patternGenerators;
};

extern UVDFLIRT *g_flirt;

#endif
