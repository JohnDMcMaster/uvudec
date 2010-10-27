/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_FLIRT_SIG_READER_H
#define UVD_FLIRT_SIG_READER_H

#include "uvd/flirt/function.h"
#include "uvd/util/types.h"

/*
UVDPatLoaderCore
*/
class UVDFLIRTSignatureDB;
class UVDFLIRTSigReader
{
public:
	UVDFLIRTSigReader(UVDFLIRTSignatureDB *db);
	~UVDFLIRTSigReader();
	
	uv_err_t preparse();
	uv_err_t decompress();
	uv_err_t advance(int bytes);
	int read_byte();
	int read16();
	int bitshift_read();
	int read_relocation_bitmask();
	uv_err_t load(const std::string &in);
	uv_err_t parse_header();
	uv_err_t parse_tree();

public:
	//Do not own this
	UVDFLIRTSignatureDB *m_db;
	std::string m_file;
	uint32_t m_file_pos;	
	char *m_file_contents;
	char *m_cur_ptr;
	unsigned int m_file_size;
	//the module working copy we use to insert into the tree
	UVDFLIRTModule m_module;
};

#endif

