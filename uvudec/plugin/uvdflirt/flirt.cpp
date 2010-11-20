/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvdflirt/flirt.h"
#include "uvdflirt/pat/pat.h"
#include "uvdflirt/sig/sig.h"
#include "uvd/util/util.h"
#include "uvd/core/runtime.h"

UVDFLIRT *g_flirt = NULL;

UVDFLIRT::UVDFLIRT()
{
	m_uvd = NULL;
}

UVDFLIRT::~UVDFLIRT()
{
}

uv_err_t UVDFLIRT::init()
{
	printf_flirt_debug("initializing FLIRT engine\n");
	//Use to have engine specific init stuff, but got moved to plugins

	return UV_ERR_OK;
}

uv_err_t UVDFLIRT::deinit()
{
	/*
	for( std::vector<UVDFLIRTPatternGenerator *>::iterator iter = m_patternGenerators.begin(); iter != m_patternGenerators.end(); ++iter )
	{
		delete *iter;
	}
	m_patternGenerators.clear();
	*/
	
	return UV_ERR_OK;
}

#if 0
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

#endif

uv_err_t UVDFLIRT::toPatFile(const std::string &outputFile)
{
	std::string output;
	
	uv_assert_err_ret(toPat(output));
	uv_assert_err_ret(writeFile(outputFile, output));

	return UV_ERR_OK;
}

uv_err_t UVDFLIRT::toPat(std::string &output)
{
	UVDFLIRTPatternGenerator *generator = NULL;
	
	uv_assert_ret(m_uvd);
	uv_assert_ret(m_uvd->m_runtime);
	uv_assert_ret(m_uvd->m_runtime->m_object);
	uv_assert_err_ret(m_patFactory.tryLoad(m_uvd->m_runtime, &generator));
	uv_assert_ret(generator);
	uv_assert_err_ret(generator->saveToString(m_uvd->m_runtime->m_object, output, true));

	return UV_ERR_OK;
}

uv_err_t UVDFLIRT::patFiles2SigFile(const std::vector<std::string> &inputFiles, const std::string &outputFile)
{
	/*
	TODO: check if we have relocation at end of line how this is treated since usually '.' padded
	*/
	UVDFLIRTSignatureDB *db = NULL;
	
	uv_assert_err_ret(patFiles2SigDB(inputFiles, &db));
	uv_assert_err_ret(db->debugDumpTree());
	uv_assert_ret(db);
	printf_flirt_debug("\n\n\nWriting to file\n");
	uv_assert_err_ret(db->writeToFile(outputFile));
	delete db;
	
	return UV_ERR_OK;
}

uv_err_t UVDFLIRT::patFiles2SigDB(const std::vector<std::string> &inputFiles, UVDFLIRTSignatureDB **out)
{
	UVDFLIRTSignatureDB *db = NULL;
	
	db = new UVDFLIRTSignatureDB();
	uv_assert_ret(db);
	uv_assert_err_ret(db->init());
	
	for( uint32_t i = 0; i < inputFiles.size(); ++i )
	{
		std::string curFile = inputFiles[i];
		
		uv_assert_ret(!curFile.empty());
		uv_assert_err_ret(db->loadFromPatFile(curFile));
	}
	uv_assert_ret(out);
	*out = db;
	return UV_ERR_OK;
}

uv_err_t UVDFLIRT::dumpSigFile(const std::string &file)
{
	UVDFLIRTSignatureDB *db = NULL;
	
	db = new UVDFLIRTSignatureDB();
	uv_assert_ret(db);
	uv_assert_err_ret(db->init());

	uv_assert_err_ret(db->loadSigFile(file));
	uv_assert_err_ret(db->dump());

	delete db;
	
	return UV_ERR_OK;
}

uv_err_t UVDFLIRT::getPatternGenerator(const std::string &file, UVDFLIRTPatternGenerator **generatorOut)
{
	uv_assert_ret(m_uvd);
	return UV_DEBUG(m_patFactory.tryLoad(m_uvd->m_runtime, generatorOut));
#if 0
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
#endif
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
		//UVD *uvd = NULL;

		flirt = new UVDFLIRT();
		uv_assert_ret(flirt);
		uv_assert_err_ret(flirt->init());
		
		//uv_assert_err_ret(UVD::getUVD(&uvd, NULL));
		//flirt->m_uvd = uvd;
		//uvd->m_flirt = flirt;
	}	
	*out = flirt;

	return UV_ERR_OK;
}

