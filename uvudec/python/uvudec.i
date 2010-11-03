/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

/*
Ex relativly complicated function:
uv_err_t getUVDFromFileName(UVD **out, const std::string &file);
-uv_err_t return value needs to be translated to exception handling
-const std::string & needs to be translated to Python string
-UVD **out needs to be translated to a return value

Success codes
-UV_ERR_OK: don't do anything special
-UV_ERR_DONE: return None
	In particular, this is seen during argument parsing
	If a special wrapper is given over the parseMain() or w/e function, this issue should go away from the Python side
-UV_ERR_BLANK: return None
	This is only used in internal utility functions
	Since its not really exported from the API, this isn't so much an issue
*/

%module uvudec
%{

#include "uvd/all.h"
#include "wrappers.h"
#include <exception>

%}

// Language independent exception handler
%include exception.i       
//Lots of STL fixups
%include "std_list.i"
%include "std_map.i"
%include "std_pair.i"
%include "std_set.i"
//TODO: figure out how to type map member variables
//something like...
//%apply const std::string& {std::string* foo};
%include "std_string.i"
%include "std_vector.i"

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


/* Convert from Python --> C */
//Take the default behavior
/*
%typemap(in) uv_err_t {
    $1 = PyInt_AsLong($input);
}
*/

%include "typemaps.i"

/* Convert from C --> Python */
%typemap(out) uv_err_t {
    if( UV_FAILED($1) )
    {
	 	SWIG_exception(SWIG_RuntimeError, uv_err_str($1));	
    }
    //Maybe all of UV_SUCCEEDED()?
    //else if( $1 == UV_ERR_DONE || $1 == UV_ERR_BLANK )
    else
    {
    	//returns Py_None with proper reference counting math
    	Py_RETURN_NONE;
    }
    /*
    else
    {
	    $result = PyInt_FromLong($1);
    }
    */
}

%typemap(out) UVD ** {
    if( UV_FAILED($1) )
    {
	 	SWIG_exception(SWIG_RuntimeError, uv_err_str($1));	
    }
    $result = PyInt_FromLong($1);
}

//FIXME: figure out how to do UVD** in to UVD * ret style translations
//Currently fails due to some typemap issue
//%apply UVD **OUTPUT { UVD **out };

%include "uvd/all.h"
%include "uvd/config/config.h"
%include "uvd/core/init.h"
%include "uvd/core/uvd.h"
%include "uvd/util/types.h"
%include "wrappers.h"

%pythoncode %{
# Seems to work
class InitDeinit:
    def __init__(self):
        # print 'Calling UVDInit()'
        init()
        get_config().parseArgs()

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

