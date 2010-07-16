/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

/*
For working with .sig files
*/
class UVDFLIRTSignature
{
public:
	UVDFLIRTSignature();
	~UVDFLIRTSignature();
	uv_err_t deinit();
	
	/*
	Using signatures
	*/
	uv_err_t sig2pat(UVDFLIRTPatternAnalysis *pat);
	
	/*
	Save .sig to given file
	*/
	uv_err_t saveToFile(const std::string &file);

public:
	//The UVD FLIRT engine
	UVDFLIRT *m_flirt;

	/*
	Our .sig file, if we created through file read
	We own it
	*/
	UVDData *m_data;
};
