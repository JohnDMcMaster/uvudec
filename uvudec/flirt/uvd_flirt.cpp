/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#include "uvd_flirt.h"
#include "uvd_flirt_pattern.h"
#include "uvd_flirt_pattern_bfd.h"
#include "uvd_flirt_pattern_uvd.h"
#include "uvd_util.h"

UVDFLIRT *g_flirt = NULL;

UVDFLIRT::UVDFLIRT()
{
}

UVDFLIRT::~UVDFLIRT()
{
}

uv_err_t UVDFLIRT::init()
{
#ifdef UVD_FLIRT_PATTERN_UVD
	UVDFLIRTPatternGeneratorUVD *generatorUVD = NULL;
	
	uv_assert_err_ret(UVDFLIRTPatternGeneratorUVD::getPatternGenerator(&generatorUVD));
	m_patternGenerators.push_back(generatorUVD);
#endif
#ifdef UVD_FLIRT_PATTERN_BFD
	UVDFLIRTPatternGeneratorBFD *generatorBFD = NULL;
	
	uv_assert_err_ret(UVDFLIRTPatternGeneratorBFD::getPatternGenerator(&generatorBFD));
	m_patternGenerators.push_back(generatorBFD);
#endif

	return UV_ERR_OK;
}

uv_err_t UVDFLIRT::deinit()
{
	for( std::vector<UVDFLIRTPatternGenerator *>::iterator iter = m_patternGenerators.begin(); iter != m_patternGenerators.end(); ++iter )
	{
		delete *iter;
	}
	m_patternGenerators.clear();
	
	return UV_ERR_OK;
}

uv_err_t UVDFLIRT::objs2patFile(const std::vector<std::string> &inputFiles, const std::string &outputFile)
{
	std::string output;
	
	uv_assert_err_ret(objs2pat(inputFiles, output));
	uv_assert_err_ret(writeFile(outputFile, output));
	
	return UV_ERR_OK;
}

uv_err_t UVDFLIRT::objs2pat(const std::vector<std::string> &inputFiles, std::string &output)
{
	output = "";
	for( std::vector<std::string>::const_iterator iter = inputFiles.begin();
			iter != inputFiles.end(); ++iter )
	{
		std::string file = *iter;
		UVDFLIRTPatternGenerator *generator = NULL;
		std::string lastOutput;
		
		//Find a capable analyzer
		uv_assert_err_ret(getPatternGenerator(file, &generator));
		uv_assert_ret(generator);
		//And get the pat entries
		uv_assert_err_ret(generator->saveToString(file, lastOutput, (iter + 1) == inputFiles.end()));
		
		output += lastOutput;
	}
	
	return UV_ERR_OK;
}

uv_err_t UVDFLIRT::getPatternGenerator(const std::string &file, UVDFLIRTPatternGenerator **generatorOut)
{
printf("num generators %d\n", m_patternGenerators.size());
	//Iterate over all generators until one claims support
	//We might want to add priorities here later
	for( std::vector<UVDFLIRTPatternGenerator *>::iterator iter = m_patternGenerators.begin();
			iter != m_patternGenerators.end(); ++iter )
	{
		UVDFLIRTPatternGenerator *generator = *iter;
		
		uv_assert_ret(generator);
		if( UV_SUCCEEDED(generator->canGenerate(file)) )
		{
			*generatorOut = generator;
			return UV_ERR_OK;
		}
	}
	
	printf_error("Could not get pat generator\n");
	return UV_DEBUG(UV_ERR_GENERAL);
}

uv_err_t UVDFLIRT::getFLIRT(UVDFLIRT **out)
{
	UVDFLIRT *flirt = NULL;
	
	if( g_flirt )
	{
		flirt = g_flirt;
	}
	else
	{
		UVD *uvd = NULL;

		flirt = new UVDFLIRT();
		uv_assert_ret(flirt);
		uv_assert_err_ret(flirt->init());
		
		uv_assert_err_ret(UVD::getUVD(&uvd, NULL));
		flirt->m_uvd = uvd;
		uvd->m_flirt = flirt;
	}	
	*out = flirt;

	return UV_ERR_OK;
}
