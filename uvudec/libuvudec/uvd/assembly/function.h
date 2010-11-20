/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

/*
FIXME
These data structures are somewhat messed up
XXX war time lets roll

Too much abstraction without enough knowledge about what was useful to abstract
It has some artifacts from not strongly seperating current analysis from the analysis DB
	and from playing around with some stuff near the beginning, such as the MD5s
	these should not be stored in the function objects, but rather computed as needed if we want to dump the func db files
It can probably be fixed now, but I haven't gotten around to it
Mostly, once FLIRT integration is more advanced, it will become clearer how this should be laid out
*/

/*
Binary functions as opposed those for processing config directives
(the config directive functions actually came first)
The naming in this file should admittedly be cleaned up

Ex, add 3 can be represented as

UVDBinaryFunction
	raw data recovered from analysis
	uvd
	UVDBinaryFunctionShared *m_shared
		function name: add3
		m_representations
			UVDBinaryFunctionInstance
				symbol: add3_quick
					or for an unknown function...
					symbol: uvudec__candela_PLTL_1__0FA6
				code:
					inc
					inc
					inc
			UVDBinaryFunctionInstance
				symbol: add3_short
				code:
					add 3
*/

#ifndef UVD_FUNCTION_H
#define UVD_FUNCTION_H

#include "uvd/util/types.h"
#include "uvd/compiler/compiler.h"
#include "uvd/data/data.h"
#include "uvd/util/version.h"
#include "uvd/relocation/relocation.h"
#include "uvd/assembly/symbol.h"
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

#if 0
//A specific implementation of function recorded from analysis 
//This has replaced UVDFunctionBinarySymbol
class UVDElf;
class UVDBinaryFunctionInstance : public UVDBinarySymbol
{
public:
	UVDBinaryFunctionInstance();
	virtual ~UVDBinaryFunctionInstance();
	uv_err_t init();
	uv_err_t deinit();
	static uv_err_t getUVDBinaryFunctionInstance(UVDBinaryFunctionInstance **out);
	
	//Gets default hash
	//Will compute it if needed
	//uv_err_t getHash(std::string &out);
	//On the relocatable version
	//uv_err_t getRelocatableHash(std::string &out);

	//Get binary representations of relocatable and raw forms
	uv_err_t getRawDataBinary(UVDData **data);
	//uv_err_t getRelocatableDataBinary(UVDData **data);

	//Get an ELF file relocatable
	//It is callee responsibilty to free
	uv_err_t toUVDElf(UVDElf **out);
	//Factory function to create from ELF file
	//It is callee responsibilty to free
	static uv_err_t getFromUVDElf(const UVDElf *in, UVDBinaryFunctionInstance **out);

	//Add setData() in a sec
	uv_err_t setData(UVDData *data);
	uv_err_t transferData(UVDData *data);
	UVDData *getData();
	
private:
	//Force hash compute
	//uv_err_t computeHash();
	//Relocatable version
	//uv_err_t computeRelocatableHash();
		
public:
	//Giving code for Language, compiler needed (used) to produce binary
	//Prob shouldn't be owner...but are we?
	UVDCompiler *m_compiler;
	//Version ranges that will produce this
	UVDSupportedVersions m_versions;
	//Project specific options needed to generate code (ex: -O3)
	//Prob shouldn't be owner...but are we?
	UVDCompilerOptions *m_compilerOptions;
private:
	//Binary version stored in DB
	//This has been take over by the m_data in the UVDBinarySymbol class
	//UVDData *m_data;
public:
	//The relocations for this function
	//Stuff like where g_debug is used
	//Also stored is a relocatable version of the function
	//Now in UVDBinarySymbol
	//UVDRelocatableData *m_relocatableData;
	//UV_DISASM_LANG
	//Programming Language code representation is in (assembly, C, etc)
	//Assembly may sound useless, but many MCU functions are well commented assembly
	int m_language;
	//Language specific code representation
	std::string m_code;
	//Description of where this code came from (ie a specific product)
	std::string m_origin;
	//Misc notes
	std::string m_notes;
	//Hash based distribution allows quicker compare and avoids copyright issues
	//std::string m_MD5;
	//std::string m_relocatableMD5;
	/*
	The real name may not be known, but the symbol can be arbitrarily defined to a value to link
	For unknown symbols, this is reccomended to be the value that came from the original program
	
	Note that while the real function name is shared between various implementations,
	the symbol name is specific to a particular implementation
	*/
	//Superceded by def in UVDBinarySymbol
	//std::string m_symbolName;
};
#endif

/*
As would be queried or written to analysis DB
Represents a group of representations of a function
*/
#if 0
class UVDBinaryFunctionShared
{
public:
	UVDBinaryFunctionShared();
	~UVDBinaryFunctionShared();
	uv_err_t deinit();

public:
	//The real name as called in a program
	//If the real name is unknown, this should be empty
	std::string m_name;
	//Human readable description of what the function does ("converts a character to upper case")
	//For unknown functions, this may be blank or a description of how it was generated
	std::string m_description;
	//Assembly/C/fortran, varients
	std::vector<UVDBinaryFunctionInstance *> m_representations;
};
#endif

//A function as found in an executable (binary) as opposed to a config function
//Intended for current analysis
class UVD;
class UVDElf;
class UVDBinaryFunction : public UVDBinarySymbol
{
public:
	UVDBinaryFunction();
	~UVDBinaryFunction();
	uv_err_t deinit();
 
 	//Locations in the source file
	uv_err_t getMin(uint32_t *out);
	uv_err_t getMax(uint32_t *out);

	//One and only one should be associated with this object
	//UVDBinaryFunctionInstance *getFunctionInstance();
	//One and only one should be associated with this object
	//void setFunctionInstance(UVDBinaryFunctionInstance *instance);

	uv_err_t toUVDElf(UVDElf **out);
	//Factory function to create from ELF file
	//It is callee responsibilty to free
	static uv_err_t getFromUVDElf(const UVDElf *in, UVDBinaryFunction **out);
	static uv_err_t getUVDBinaryFunctionInstance(UVDBinaryFunction **out);

	uv_err_t setData(UVDData *data);
	uv_err_t transferData(UVDData *data);
	UVDData *getData();

public:
	//Raw binary representation of function as it was found in the program
	//This version should not be processed for relocatables
	//We own this
	//UVDData *m_data;
	//Analysis engine generated by
	UVD *m_uvd;
	//The generated function info
	//m_shared should have a single m_representations
	//maybe later we link into a db could have multiple?
	//UVDBinaryFunctionShared *m_shared;
	//Offset that was found at in original data (m_uvd->m_data)
	//FIXME: this should instead be uv_addr_t m_address;
	//Size can be queried from m_data
	uint32_t m_offset;

private:
	//Cache of the instance we are using so we don't have to query it from the DB
	//UVDBinaryFunctionInstance *m_instance;
};

#endif //UVD_FUNCTION_H

