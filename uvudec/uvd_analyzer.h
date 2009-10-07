/*
Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under the terms of the BSD license.  See LICENSE for details.
*/

#ifndef UVD_ANALYZER_H
#define UVD_ANALYZER_H

#include <set>
#include <stdint.h>
#include "uvd_address.h"
#include "uvd_data.h"
#include "uvd_analysis_db.h"

/*
Ways that memory locations are used (referenced)
Originally used for branch and jump analysis
*/
//No attributes
//Note if passed in this can often mean ALL because unspecified will match everything
#define UVD_MEMORY_REFERENCE_NONE						0x00
//In X1234: CALL 0x5678, target address would be 0x1234
#define UVD_MEMORY_REFERENCE_CALL_SOURCE				0x01
//In X1234: CALL 0x5678, target address would be 0x5678
#define UVD_MEMORY_REFERENCE_CALL_DEST					0x02
//In X1234: JMP 0x5678, target address would be 0x1234
#define UVD_MEMORY_REFERENCE_JUMP_SOURCE				0x04
//In X1234: JMP 0x5678, target address would be 0x5678
#define UVD_MEMORY_REFERENCE_JUMP_DEST					0x08
//Interrupt vector entry point
#define UVD_MEMORY_REFERENCE_ISR_ENTRY					0x10
//String value (constant C only?)
//Should imply constant type
#define UVD_MEMORY_REFERENCE_STRING						0x20
//Constant
#define UVD_MEMORY_REFERENCE_CONSTANT					0x40
//Variable
#define UVD_MEMORY_REFERENCE_VAR						0x80

//A single memory reference used for code flow analysis
class UVDMemoryReference
{
public:
	UVDMemoryReference();
	
public:
	//Flags for types of references
	uint32_t m_types;
	uint32_t m_from;
};

typedef std::map<uint32_t, UVDMemoryReference *> UVDAnalyzedMemoryLocationReferences;

/*
Each memory location must keep track of references to itself to know what sort of branching we should expect
*/
class UVDAnalyzedMemoryLocation : public UVDMemoryLocation
{
public:
	UVDAnalyzedMemoryLocation();
	UVDAnalyzedMemoryLocation(unsigned int min_addr);
	UVDAnalyzedMemoryLocation(unsigned int min_addr, unsigned int max_addr,
			UVDMemoryShared *space = NULL);
	//UVDAnalyzedMemoryLocation(uint32_t referenceCount);

	uv_err_t insertReference(uint32_t from, uint32_t type);
	uint32_t getReferenceCount();
	//Query bitmask of reference types
	uint32_t getReferenceTypes();
	
	uv_err_t getReferences(UVDAnalyzedMemoryLocationReferences &references, uint32_t type);
	
public:
	//How many times a reference has been made to this location
	UVDAnalyzedMemoryLocationReferences m_references;
	//uint32_t m_referenceCount;
};

class UVDAnalyzedCodeShared
{
public:
	UVDAnalyzedCodeShared();

public:
	//Language: C, C++, assembly, etc.  Assume C by default since overwhelmingly popular on embedded
	unsigned int m_language;

	//A copy of the code
	UVDDataChunk *m_dataChunk;
};

class UVDAnalyzedCode
{
public:
	UVDAnalyzedCode();

public:
	UVDAnalyzedCodeShared *m_shared;
};

/*
A block of code
	May contain nested blocks and/or actual code
Highest level block should be the entire program
	Next level is functions
	The lowest level blocks should be non-branching segments
*/
class UVDAnalyzedBlock
{
public:
	UVDAnalyzedBlock();
	
	//Get the actual code representation of this block
	uv_err_t getDataChunk(UVDDataChunk **dataChunk);
	
	uv_err_t getMinAddress(uint32_t &address);
	uv_err_t getMaxAddress(uint32_t &address);
	uv_err_t getSize(uint32_t &address);
	
public:
	//Both of following can be set
	//m_code should always indicate the range and subblocks, if present, also indicated
	//If it contains code
	UVDAnalyzedCode *m_code;
	//If it contains blocks, usually should be more than one
	std::vector<UVDAnalyzedBlock *> m_blocks;
};

class UVDAnalyzedFunctionShared
{
public:
	UVDAnalyzedFunctionShared();

public:
	//Name of the function
	std::string m_sName;
	//Langauge specific human readable syntax
	std::string m_sSyntax;
	//Calling convention.  Architecture specific
	unsigned int m_callingConvention;
	//Shared code
	UVDAnalyzedCodeShared *m_code;
};

//No inherent reason why same function can't be at multiple locations, 
//other reasons why this may happen
//Can store analysis in the archive file is main reason, however
class UVDAnalyzedFunction
{
public:
	UVDAnalyzedFunction();

public:
	UVDAnalyzedCode *m_code;
	UVDAnalyzedFunctionShared *m_shared;
};

//Map between an address and information regarding it
//TODO: make a compare method to key to UVDMemoryLocation instead 
typedef std::map<uint32_t, UVDAnalyzedMemoryLocation *> UVDAnalyzedMemorySpace;
typedef std::vector<UVDAnalyzedMemoryLocation *> UVDAnalyzedMemoryLocations;
//Holds data that resulted from binary analysis or advanced hints from user
class UVDAnalyzer
{
public:
	UVDAnalyzer();
	uv_err_t init();

	uv_err_t insertReference(uint32_t targetAddress, uint32_t from, uint32_t type);
	//For destinations, not sources
	uv_err_t insertCallReference(uint32_t targetAddress, uint32_t from);
	uv_err_t insertJumpReference(uint32_t targetAddress, uint32_t from);
	
	void updateCache(uint32_t address, const UVDVariableMap &analysisResult);
	uv_err_t readCache(uint32_t address, UVDVariableMap &analysisResult);

	uv_err_t getAddresses(UVDAnalyzedMemorySpace &addresses, uint32_t type = UVD_MEMORY_REFERENCE_NONE);
	uv_err_t getAddresses(UVDAnalyzedMemoryLocations &locations, uint32_t type = UVD_MEMORY_REFERENCE_NONE);

	//For destinations, not sources
	uv_err_t getCalledAddresses(UVDAnalyzedMemorySpace &calledAddresses);
	uv_err_t getJumpedAddresses(UVDAnalyzedMemorySpace &calledAddresses);
	
	//DB of currently analyzed program
	uv_err_t getAnalyzedProgramDB(UVDAnalysisDBArchive **db);
	
public:
	//Superblock for block representation of program
	UVDAnalyzedBlock *m_block;

	UVDAnalyzedMemorySpace m_referencedAddresses;
	//Deprecated by above
	//UVDAnalyzedMemorySpace m_calledAddresses;
	//UVDAnalyzedMemorySpace m_jumpedAddresses;
	
	//A location where any change in code flow can occur
	//Branch target/source, call target/source
	UVDAnalyzedMemorySpace m_branchPoints;
	
	//ROM data addresses, not including string addresses
	//Provide utility func later if need conglomeration
	UVDAnalyzedMemorySpace m_ROMADataddresses;
	//String data addresses
	UVDAnalyzedMemorySpace m_stringAddresses;
	
	std::map<uint32_t, UVDVariableMap> m_analysisCache;

	//Database of previous analysis
	UVDAnalysisDBConcentrator *m_db;
	//Database constructed from currnet program
	//May be listed in m_db
	UVDAnalysisDBArchive *m_curDb;
	
	//List of functions found
	std::vector<UVDBinaryFunction *> m_functions;
};

#endif //UVD_ANALYZER_H
