/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_FLIRT_PAT_READER_H
#define UVD_FLIRT_PAT_READER_H

#include "uvd/util/types.h"
#include "uvdflirt/flirt.h"

/*
UVDPatLoaderCore
*/
class UVDFLIRTSignatureDB;
class UVDFLIRTModule;
class UVDFLIRTSignatureRawSequence;
class UVDPatLoaderCore
{
public:
	UVDPatLoaderCore(UVDFLIRTSignatureDB *db, const std::string &file = "");
	~UVDPatLoaderCore();
	
	uv_err_t fromString(const std::string &in);
	uv_err_t fileLine(const std::string &in);
	uv_err_t insert(UVDFLIRTModule *function, UVDFLIRTSignatureRawSequence &leadingBytes);

public:
	//Do not own this
	UVDFLIRTSignatureDB *m_db;
	std::string m_file;
};

#endif

