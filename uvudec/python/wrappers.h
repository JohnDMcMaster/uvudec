/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef PYTHON_WRAPPERS_H
#define PYTHON_WRAPPERS_H

#include "uvd/all.h"

#define UVD_SWIG_ASSERT_ERR(_rcIn) \
do \
{ \
	uv_err_t _rc = _rcIn; \
	if( UV_FAILED(_rc) ) \
	{ \
		throw UVDException(_rc); \
	} \
} while( 0 ) \

class UVDException
{
public:
	UVDException(int rc)
	{
		m_rc = rc;
	}
	
public:
	int m_rc;
};


/*
XXX: The following is all test code, don't rely on it
*/

class uvd : public UVD
{
public:
	//static uv_err_t getUVDFromFileName(UVD **uvdOut, const std::string &file);
	static UVD *getUVDFromFileName(const char *fileName);
};

//Test function that always fails
uv_err_t always_return_rc(int rc);

uv_err_t returns_string(std::string &out);
uv_err_t returns_string_other(std::string &my_ret_value);
uv_err_t takes_string(const std::string &in);

#endif

