/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

/*
Fast Library Recognition Technology (FLIRT) implementation
Given a library, produce signature files that can be used to 
Based off of paper on Data Rescue's website
http://www.hex-rays.com/idapro/flirt.htm
*/

#ifndef UVD_FLIRT_H
#define UVD_FLIRT_H

#include "uvd/core/uvd.h"
#include "uvd/flirt/pat/factory.h"
#include "uvd/flirt/pat/pat.h"
#include <string>
#include <vector>

//#define FLIRT_WARNING
#ifdef FLIRT_WARNING
#define printf_flirt_warning(format, ...) do { printf("WARNING: FLIRT: " format, ## __VA_ARGS__); fflush(stdout); } while( 0 )
#else
#define printf_flirt_warning(...)
#endif

/*
Core FLIRT engine
*/
class UVDFLIRTSignatureDB;
class UVDFLIRT
{
public:
	UVDFLIRT();
	~UVDFLIRT();
	uv_err_t init();
	uv_err_t deinit();
	
	/*
	We assume obj2pat is only going to generate from a single runtime
	Old code instead instantiated a UVDFLIRT engine w/o an associated UVD and did it on demand
	However, its very important in the long time to use the object format, OS stuff etc (UVDRuntime),
	so require UVD instantiation
	*/
	//Automatically chose object format and engine
	//uv_err_t objs2patFile(const std::vector<std::string> &inputFiles, const std::string &outputFile);
	//uv_err_t objs2pat(const std::vector<std::string> &inputFiles, std::string &output);
	//Uses the object attached to our uvd instance to generate it
	uv_err_t toPatFile(const std::string &outputFile);
	uv_err_t toPat(std::string &output);

	//Input pattern files and output sig file
	uv_err_t patFiles2SigFile(const std::vector<std::string> &inputFiles, const std::string &outputFile);
	//Output signature tree is NOT checked for consistancy against system signatures
	//out should be deleted by calllee
	uv_err_t patFiles2SigDB(const std::vector<std::string> &inputFiles, UVDFLIRTSignatureDB **out);

	//Dump a sig file in a human readable form
	uv_err_t dumpSigFile(const std::string &file);
	
	//Don't do a full UVD engine init, only grab the FLIRT component
	//The UVD engine may or may not be initialized internally, do not count on it
	static uv_err_t getFLIRT(UVDFLIRT **out);
	
protected:
	//Find a capable analyzer
	uv_err_t getPatternGenerator(const std::string &file, UVDFLIRTPatternGenerator **generator);

public:
	UVD *m_uvd;
	//Pattern generation engines
	//std::vector<UVDFLIRTPatternGenerator *> m_patternGenerators;
	UVDFLIRTPatFactory m_patFactory;
};

extern UVDFLIRT *g_flirt;

#endif

