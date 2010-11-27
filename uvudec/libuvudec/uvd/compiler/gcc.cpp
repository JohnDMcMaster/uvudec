/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/compiler/gcc.h"
#include "uvd/util/util.h"
#ifdef USING_BINUTILS
#include "bfd.h"
#endif
#include <cxxabi.h>

UVDCompilerGCC::UVDCompilerGCC()
{
}

UVDCompilerGCC::~UVDCompilerGCC()
{
}

uv_err_t UVDCompilerGCC::demangle(const std::string &in, std::string &out)
{
	/*
	http://www.kegel.com/mangle.html
	good overview of demangling for variety of situations 
	*/

	uv_assert_err_ret(demangleByABI(in, out));
	return UV_ERR_OK;
}

uv_err_t UVDCompilerGCC::demangleByABI(const std::string &in, std::string &out)
{
	/*
	http://gcc.gnu.org/onlinedocs/libstdc++/latest-doxygen/a00855.html
	char * __cxxabiv1::__cxa_demangle (const char *__mangled_name, char *__output_buffer, size_t *__length, int *__status)
	http://idlebox.net/2008/0901-stacktrace-demangled/cxa_demangle.htt
	Has better description

	status
		*status is set to one of the following values:

		    * 0: The demangling operation succeeded.
		    * -1: A memory allocation failure occurred.
		    * -2: mangled_name is not a valid name under the C++ ABI mangling rules.
		    * -3: One of the arguments is invalid.

	*/
	//Will be allocated for us if we don't
	char *buff = NULL;
	//See above
	int status = 0;
	
	buff = abi::__cxa_demangle(in.c_str(), buff, 0, &status);
	if( status == -2 )
	{
		return UV_ERR_ARGS;
	}
	uv_assert_ret(status == 0);
	uv_assert_ret(buff);
	
	out = buff;
	free(buff);
	
	return UV_ERR_OK;
}

uv_err_t UVDCompilerGCC::demangleByCppfilt(const std::string &in, std::string &out)
{
	/*
	[mcmaster@gespenst bin]$ c++filt _ZN9UVDConfig9parseMainEiPKPc
	UVDConfig::parseMain(int, char* const*)
	*/
	std::string stdOut;
	int rcProcess = 0;
	std::vector<std::string> args;

	args.push_back(in);
	
	//Should return not supported if not present
	uv_assert_err_ret(UVDExecuteToText("c++filt",
			args,
			rcProcess,
			&stdOut,
			NULL));
	uv_assert_ret(rcProcess == 0);
	out = trimString(stdOut);
	
	return UV_ERR_OK;
}

uv_err_t UVDCompilerGCC::demangleByBinutils(const std::string &in, std::string &out)
{
	/*
	//Hmm older versions didn't require a bfd pointer?
	char *bfd_demangle (bfd *, const char *, int);
	*/
#ifdef USING_BINUTILS
	char *temp = NULL;
	
	//Needs a bfd for arch probably
	//will it give reasonable default if we NULL it?  Or crash...
	temp = bfd_demangle(NULL, in.c_str(), DMGL_PARAMS | DMGL_ANSI);
	//Only returned if it liked it (thought it was g++ compatible)
	if( temp )
	{
		out = temp;
		free(temp);
	}
	else
	{
		out = in;
	}
#else
	return UV_ERR_OK;
#endif
}

uv_err_t UVDCompilerGCC::demangleByAlgorithm(const std::string &in, std::string &out)
{
	//It would be a good idea to eventually ipmlement our own engine
	return UV_ERR_NOTSUPPORTED;
}

/* 
   char *cplus_demangle (const char *mangled, int options)

   If MANGLED is a mangled function name produced by GNU C++, then
   a pointer to a malloced string giving a C++ representation
   of the name will be returned; otherwise NULL will be returned.
   It is the caller's responsibility to free the string which
   is returned.

   The OPTIONS arg may contain one or more of the following bits:

   	DMGL_ANSI	ANSI qualifiers such as `const' and `void' are
			included.
	DMGL_PARAMS	Function parameters are included.
*/

uv_err_t UVDCompilerGCC::demangleApproximation(const std::string &in, std::string &out)
{
	UVDCompilerGCC temp;
	
	uv_assert_err_ret(temp.demangle(in, out));
	return UV_ERR_OK;
}

