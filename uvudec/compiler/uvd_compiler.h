/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#pragma once

#include "uvd_types.h"
#include <string>

/*
These defines will need reworking once this becomes more important
Its not too uncommon for a compiler to do C and C++
Consider it a C++ compiler only?
*/

#define UVD_COMPILER_UNKNOWN		0x0000
/*
gcc
Many embedded compilers are retargetted gcc/g++ varieties
*/
#define UVD_COMPILER_GCC			0x0001
//g++
#define UVD_COMPILER_GXX			0x0002
//sdcc
#define UVD_COMPILER_SDCC			0x0003
//Keil compiler
#define UVD_COMPILER_KEIL			0x0004
//Microchip C18
#define UVD_COMPILER_C18			0x0005
//Intel C compiler
#define UVD_COMPILER_INTEL_C		0x0101
//visual C++
#define UVD_COMPILER_VC				0x0201
//visual basic (yes, I've had to "fix" VB apps before)
#define UVD_COMPILER_VB				0x0202
//...and may more to come
//PLD stuff?  One day...
//#define UVD_COMPILER_ISE/XILINX or something  0x10000

//Used on a specific peice of code
class UVDCompilerOptions
{
public:
};

//A compiler, such as GCC
/*
class UVDCompilerShared
{
public:
	UVDCompilerShared();
	
public:
};
*/

//A specific version of a compiler
//Primary purpose is to format output data
//Possibly also to compile the code (for verification of decompilation)
class UVDCompiler
{
public:
	UVDCompiler();
	virtual ~UVDCompiler();

	//Given input, produce it as an output suitable for comments
	//Input may be multi-line
	//If it contains illegal sequences (ex: ANSI C compiler asked to comment string "*/"),
	//Will return error
	virtual uv_err_t comment(const std::string &in, std::string &out);
	//Even if it has to do something odd to print the data, it will output it
	//If not possible, will drop some data
	//If not implemented, will drop comments
	//Should return error only on internal errors
	virtual uv_err_t commentAggressive(const std::string &in, std::string &out);
	
	static uv_err_t getCompiler(int compilerType, const std::string &version, UVDCompiler **compilerOut);
	static uv_err_t getCompilerFromFile(const std::string &file, UVDCompiler **compilerOut);
	
public:
	//A compiler source tree specific version such as 3.4.1
	std::string m_version;

	//Compiler define identifying type
	int m_compiler;
	//Human readable name.  Ex: gcc-mips
	std::string m_name;
	//Additional version information identifying a special branch or such.  Ex: "Sourcery ARM"
	std::string m_typeExtra;
};

