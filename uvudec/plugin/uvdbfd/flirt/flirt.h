/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_FLIRT_PATTERN_BFD_H
#define UVD_FLIRT_PATTERN_BFD_H

//#ifdef UVD_FLIRT_PATTERN_BFD
#if 1

#include "uvd/util/types.h"
#include "uvdflirt/pat/pat.h"
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
	
	static uv_err_t canLoad(const UVDRuntime *runtime, uvd_priority_t *confidence, void *data);
	static uv_err_t tryLoad(const UVDRuntime *runtime, UVDFLIRTPatternGenerator **out, void *data);

	virtual uv_err_t saveToStringCore(UVDObject *object, std::string &output);	

protected:
	uv_err_t generateByBFD(bfd *abfd, std::string &output);
	//Not an archive
	uv_err_t generateByBFDCore(bfd *abfd, std::string &output);
	//uv_err_t generateByFile(const std::string &fileName, std::string &output);
};

#endif

#endif
