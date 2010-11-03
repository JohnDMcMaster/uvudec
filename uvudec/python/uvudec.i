/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

%module uvudec
%{

#include "uvd/all.h"
#include "wrappers.h"
#include <exception>

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

//stdint recreated...
typedef int int32_t;
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef char int8_t;
typedef unsigned char uint8_t;

//Also, it seems that these aren't getting properly defined either
typedef int32_t uv_err_t;
typedef int32_t uvd_tri_t;
typedef uv_err_t (*uv_thunk_t)();
typedef uint32_t uv_addr_t;

%include "uvd/all.h"
%include "uvd/util/types.h"
%include "uvd/core/init.h"
%include "uvd/core/uvd.h"
%include "wrappers.h"

%pythoncode %{
# Seems to work
class InitDeinit:
    def __init__(self):
        # print 'Calling UVDInit()'
        init()

    def __del__(self):
        # print 'Calling UVDDeinit()'
        deinit()
# Dummy instance to get created and destroyed
# We could get init to be executed globally...but I don't know about deinit
obj = InitDeinit()

%}

%{
#include "wrappers.cpp"
%}

