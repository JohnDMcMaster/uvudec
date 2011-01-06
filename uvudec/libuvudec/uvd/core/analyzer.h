/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_ANALYZER_H
#define UVD_ANALYZER_H

#include <set>
#include <stdint.h>
//#include "uvd/core/analysis_db.h"
#include "uvd/assembly/address.h"
#include "uvd/data/data.h"
#include "uvd/assembly/symbol.h"

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

typedef std::map<uint32_t, UVDMemoryReference *> UVDAnalyzedMemoryRangeReferences;

/*
Each memory location must keep track of references to itself to know what sort of branching we should expect
*/
class UVDAnalyzedMemoryRange : public UVDAddressRange
{
public:
	UVDAnalyzedMemoryRange();
	UVDAnalyzedMemoryRange(unsigned int min_addr);
	UVDAnalyzedMemoryRange(unsigned int min_addr, unsigned int max_addr,
			UVDAddressSpace *space = NULL);
	//UVDAnalyzedMemoryRange(uint32_t referenceCount);
	~UVDAnalyzedMemoryRange();
	uv_err_t deinit();

	uv_err_t insertReference(uint32_t from, uint32_t type);
	uint32_t getReferenceCount();
	//Query bitmask of reference types
	uint32_t getReferenceTypes();
	
	uv_err_t getReferences(UVDAnalyzedMemoryRangeReferences &references, uint32_t type);
	
public:
	//How many times a reference has been made to this location
	UVDAnalyzedMemoryRangeReferences m_references;
	//uint32_t m_referenceCount;
};

class UVDAnalyzedCode
{
public:
	UVDAnalyzedCode();
	~UVDAnalyzedCode();
	uv_err_t deinit();

public:
	//Language: C, C++, assembly, etc.  Assume C by default since overwhelmingly popular on embedded
	unsigned int m_language;

	//A copy of the code, we own it
	UVDDataChunk *m_dataChunk;
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
	~UVDAnalyzedBlock();
	uv_err_t deinit();
	
	//Get the actual code representation of this block
	uv_err_t getDataChunk(UVDDataChunk **dataChunk);
	
	uv_err_t getMinAddress(uint32_t &address);
	uv_err_t getMaxAddress(uint32_t &address);
	uv_err_t getSize(uint32_t &address);
	
public:
	//Both of following can be set
	//m_code should always indicate the range and subblocks, if present, also indicated
	//If it contains code
	//We own this
	UVDAnalyzedCode *m_code;
	//If it contains blocks, usually should be more than one
	std::vector<UVDAnalyzedBlock *> m_blocks;
	UVDAddressSpace *m_addressSpace;
};

//No inherent reason why same function can't be at multiple locations, 
//other reasons why this may happen
//Can store analysis in the archive file is main reason, however
class UVDAnalyzedFunction
{
public:
	UVDAnalyzedFunction();
	~UVDAnalyzedFunction();
	uv_err_t deinit();

public:
	//Name of the function
	std::string m_sName;
	//Language specific human readable syntax
	std::string m_sSyntax;
	//Calling convention.  Architecture specific
	unsigned int m_callingConvention;
	//We own this
	UVDAnalyzedCode *m_code;
};

//Map between an address and information regarding it
//TODO: make a compare method to key to UVDAddressRange instead 
typedef std::map<uint32_t, UVDAnalyzedMemoryRange *> UVDAnalyzedMemorySpace;
typedef std::vector<UVDAnalyzedMemoryRange *> UVDAnalyzedMemoryRanges;
class UVDBinaryFunctionShared;
class UVDStringEngine;
class UVDBinaryFunctionInstance;
class UVD;
class UVDInstruction;
//Holds data that resulted from binary analysis or advanced hints from user
class UVDAnalyzer
{
public:
	UVDAnalyzer();
	~UVDAnalyzer();
	uv_err_t init(UVD *uvd);
	uv_err_t deinit();

	uv_err_t insertReference(uint32_t targetAddress, uint32_t from, uint32_t type);
	//For destinations, not sources
	uv_err_t insertCallReference(uv_addr_t targetAddress, uv_addr_t from);
	uv_err_t insertJumpReference(uv_addr_t targetAddress, uv_addr_t from);
	
	uv_err_t getAddresses(UVDAnalyzedMemorySpace &addresses, uint32_t type = UVD_MEMORY_REFERENCE_NONE);
	uv_err_t getAddresses(UVDAnalyzedMemoryRanges &locations, uint32_t type = UVD_MEMORY_REFERENCE_NONE);

	//For destinations, not sources
	uv_err_t getCalledAddresses(UVDAnalyzedMemorySpace &calledAddresses);
	uv_err_t getJumpedAddresses(UVDAnalyzedMemorySpace &calledAddresses);
	
	//Register a newly analyzed function
	//Will reflect the analyzedProgramDB to reflect the newly found function instance
	//Note that the function's shared data will be from current analysis and must be free'd here
	uv_err_t loadFunction(UVDBinaryFunction *function);

	uv_err_t mapSymbols();

	//After interpreting these instructions, add to analysis DB info based on mined attributes
	//uv_err_t analyzeCall(UVDInstruction *instruction, uint32_t startPos, const UVDVariableMap &attributes);
	//uv_err_t analyzeJump(UVDInstruction *instruction, uint32_t startPos, const UVDVariableMap &attributes);

	uv_err_t assignDefaultSymbolNames();
	uv_err_t identifyKnownFunctions();

public:
	//Superblock for block representation of program
	UVDAnalyzedBlock *m_block;

	//Keeps track of jumped to and called addresses
	UVDAnalyzedMemorySpace m_referencedAddresses;
	
	//A location where any change in code flow can occur
	//Branch target/source, call target/source
	UVDAnalyzedMemorySpace m_branchPoints;
	
	//ROM data addresses, not including string addresses
	//Provide utility func later if need conglomeration
	UVDAnalyzedMemorySpace m_ROMADataddresses;
	
	//List of functions found during analysis
	//XXX: should this get replaced by the symbol DB?
	std::set<UVDBinaryFunction *> m_functions;
	//All of the symbols discovered during this analysis
	//m_functions should be contained in this as well
	UVDBinarySymbolManager m_symbolManager;
	
	UVDStringEngine *m_stringEngine;

	UVD *m_uvd;
};

#endif //UVD_ANALYZER_H
