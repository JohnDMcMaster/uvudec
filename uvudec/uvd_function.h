/*
Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under the terms of the BSD license.  See LICENSE for details.
*/

#pragma once

#include "uvd_types.h"
#include "uvd_compiler.h"
#include "uvd_data.h"
#include "uvd_version.h"
#include <string>
#include <vector>

/*
In practical terms, a function is loaded from a config archive by the following:
-Text file (*.func) describing what the function is called and other common information
-Entries for each representation
	-*.func file will have reference to binary (*.bin)

So at a minimum we need/should have:
-A .func file
-A .bin file to describe the function

Ex for archive SDCC_CRT.tar.bz2

printf.func
	# Common info
	NAME=printf
	DESC=Print a formatted string.
	
	# Implementation entry
	IMPL=printf__version_1_2__optimized.bin
	# Missing version IDs means "all"
	# Min version is inclusive
	VER_MIN=1
	# Max version is exclusive
	VER_MAX=2
printf__version_1_2__optimized.bin
	 


*/

//A specific implementation of function recorded from analysis 
class UVDBinaryFunctionCodeShared
{
public:
	UVDBinaryFunctionCodeShared();
	
	//Gets default hash
	//Will compute it if needed
	uv_err_t getHash(std::string &hash);
	//Force hash compute
	uv_err_t computeHash();
		
public:
	//Giving code for langauge, compiler needed (used) to produce binary 
	UVDCompiler *m_compiler;
	//Version ranges that will produce this
	UVDSupportedVersions m_versions;
	//Project specific options needed to generate code (ex: -O3)
	UVDCompilerOptions *m_compilerOptions;
	//Binary version stored in DB
	UVDDataChunk *m_dataChunk;

	//UV_DISASM_LANG
	//Programming langauge code representation is in (assembly, C, etc)
	//Assembly may sound useless, but many MCU functions are well commented assembly
	int m_langauge;
	//Langauge specific code representation
	std::string m_code;
	//Description of where this code came from (ie a specific product)
	std::string m_origin;
	//Misc notes
	std::string m_notes;
	//Hash based distribution allows quicker compare and avoids copyright issues
	std::string m_MD5;
};

//As would be queried or written to analysis DB
class UVDBinaryFunctionShared
{
public:
	UVDBinaryFunctionShared();

public:
	//name as called in a program
	std::string m_name;
	//Human readable description of what the function does ("converts a character to upper case")
	std::string m_description;
	//Assembly/C/fortran, varients
	std::vector<UVDBinaryFunctionCodeShared *> m_representations;
};

//A function as found in an executable (binary) as opposed to a config function
//Intended for current analysis
class UVD;
class UVDBinaryFunction
{
public:
	UVDBinaryFunction();

public:
	//Local binary representation of function
	//May be different than DB version for small techincal reasons (nop padding?)
	UVDDataChunk *m_dataChunk;
	UVD *m_uvd;
	UVDBinaryFunctionShared *m_shared;
};

