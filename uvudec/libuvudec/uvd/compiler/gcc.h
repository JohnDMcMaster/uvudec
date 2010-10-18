/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_COMPILER_GCC_H
#define UVD_COMPILER_GCC_H

#include "uvd/util/types.h"
#include "uvd/compiler/compiler.h"

class UVDCompilerGCC : UVDCompiler
{
public:
	UVDCompilerGCC();
	~UVDCompilerGCC();
	
	//If we end up supporting more than one of these, add a config to select the engine to use
	uv_err_t demangle(const std::string &in, std::string &out);
	//Use c++ runtime to demangle. 
	//Easy to implement, but depends on using same compiler and open problem if all cases will be handeled properly
	uv_err_t demangleByABI(const std::string &in, std::string &out);
	//Command line program.  Would this be useful?
	uv_err_t demangleByCppfilt(const std::string &in, std::string &out);
	//Instead of using libiberity indirectly through c++filt, link to it directly
	uv_err_t demangleByBinutils(const std::string &in, std::string &out);
	//Self-contained implemetnation 
	uv_err_t demangleByAlgorithm(const std::string &in, std::string &out);
	//Less proper, but for general use okay since we don't need instance
	static uv_err_t demangleApproximation(const std::string &in, std::string &out);
};

#endif

