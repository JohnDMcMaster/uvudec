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
		Py_RETURN_NONE;
	}
	catch(...)
	{
	 	SWIG_exception(SWIG_UnknownError, "Unknown exception");
		Py_RETURN_NONE;
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
    	//FIXME: this is causing return type conversions in some areas
    	//Honestly, I'm not sure why it WORKS sometimes
    	//returns Py_None with proper reference counting math
    	//#define Py_RETURN_NONE return Py_INCREF(Py_None), Py_None
    	//printf("pre return none\n"); fflush(stdout);
    	Py_RETURN_NONE;
    	//Eh this isn't proper, but this is statically allocated and probably won't lose ref count anyway
    	//$result = Py_None;
    	//printf("post return none\n"); fflush(stdout);
		//$result = $result;
    }
    /*
    else
    {
	    $result = PyInt_FromLong($1);
    }
    */
}

/*
FIXME: figure out how to do UVD** in to UVD * ret style translations
Currently fails due to some typemap issue
All of the ** types used
find -mindepth 3 -name '*.h' -exec fgrep '**' {} ';' |sed 's/^.*[(]//g' |sed 's/[)].*$//g' |awk -F ',' '{ for(i=1;i<=NF;++i) print $i  }' |fgrep '**' |fgrep -v '***' |tr -d '[:blank:]' |grep -v '^$' |sort -u |wc -l
71
Unique UVD only types
find -mindepth 3 -name '*.h' -exec fgrep '**' {} ';' |sed 's/^.*[(]//g' |sed 's/[)].*$//g' |awk -F ',' '{ for(i=1;i<=NF;++i) print $i  }' |fgrep '**' |fgrep -v '***' |tr -d '[:blank:]' |grep -v '^$' |fgrep UVD |awk -F '**' '{ print $1 }' |sort -u |wc -l
43
*/
%typemap(in, numinputs=0) UVD **out (UVD *temp) {
   $1 = &temp;
}

%typemap(argout) (UVD **) {
	PyObject *to_add = SWIG_NewPointerObj($1, $descriptor(UVD *), SWIG_POINTER_OWN);
	$result = SWIG_AppendOutput($result, to_add);
}

//Anytime we have non-const std::string &, its a return type
//Thought swig would translate this, but it doesn't...guess not common enough?
%typemap(in, numinputs=0) std::string &out (std::string temp)
{
	$1 = &temp;
}
 
%typemap(argout) (std::string &out)
{
	resultobj = Py_None;
	PyObject *to_add = PyString_FromString((*$1).c_str());
	$result = SWIG_AppendOutput($result, to_add);
}
 
%include "uvd/all.h"
%include "uvd/config/config.h"
%include "uvd/core/init.h"
%include "uvd/core/uvd.h"
%include "uvd/util/types.h"
%include "uvd/util/error.h"
%include "wrappers.h"

%pythoncode %{
# Seems to work
class InitDeinit:
    def __init__(self):
        print 'Calling UVDInit()'
        UVDInit()
        get_config().parseArgs()

    def __del__(self):
        print 'Calling UVDDeinit()'
        UVDDeinit()
        
# Dummy instance to get created and destroyed
# We could get init to be executed globally...but I don't know about deinit
print 'Constructing InitDeinit'
obj = InitDeinit()

%}

%{
#include "wrappers.cpp"
%}

