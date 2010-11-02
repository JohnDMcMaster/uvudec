/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

%module uvudec
%{

#include "uvd/all.h"
#include <exception>

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

#define UVD_SWIG_ASSERT_ERR(rcIn) \
do \
{ \
	uv_err_t rc = rcIn; \
	if( UV_FAILED(rc) ) \
	{ \
		throw UVDException(rc); \
	} \
} while( 0 ) \

%}

// Language independent exception handler
%include exception.i       

%exception
{
	/*
	SWIG_MemoryError
	SWIG_IOError
	SWIG_RuntimeError
	SWIG_IndexError
	SWIG_TypeError
	SWIG_DivisionByZero
	SWIG_OverflowError
	SWIG_SyntaxError
	SWIG_ValueError
	SWIG_SystemError
	SWIG_UnknownError
	*/
	try
	{
		$action
	}
	catch(UVDException e)
	{
	 	SWIG_exception(SWIG_RuntimeError, uv_err_str(e.m_rc));
		return NULL;
	}
	catch(...)
	{
	 	SWIG_exception(SWIG_UnknownError, "Unknown exception");
		return NULL;
	}
}

%{
static void init()
{
	UVD_SWIG_ASSERT_ERR(UVDInit());
}
static void deinit()
{
	UVD_SWIG_ASSERT_ERR(UVDDeinit());
}

%}
//extern double My_variable;
//extern int fact(int n);
//extern int my_mod(int x, int y);
//extern char *get_time();

//stdint recreated...
typedef int int32_t;
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef char int8_t;
typedef unsigned char uint8_t;

typedef int32_t uv_err_t;
typedef int32_t uvd_tri_t;
typedef uv_err_t (*uv_thunk_t)();
typedef uint32_t uv_addr_t;
%include "uvd/all.h"
%include "uvd/util/types.h"
%include "uvd/core/init.h"
%include "uvd/core/uvd.h"

void init();
void deinit();
//extern int UVDInit();
//extern int UVDDeinit();


