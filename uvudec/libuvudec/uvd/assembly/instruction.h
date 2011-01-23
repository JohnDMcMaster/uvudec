/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

/*
TODO:
This is an ancient file in the history of this project
The code may be a bit archaic and needs some cleanup
*/

#ifndef UVD_INSTRUCTION_H
#define UVD_INSTRUCTION_H

#include <vector>
#include <string>
#include "uvd/config/config.h"
#include "uvd/data/data.h"
#include "uvd/util/types.h"

/*
Operand to an instruction 
Linked list node
I've never observed additional operands after an immediate, so don't support them
	If it does occur, treat as an instruction postfix and not the true opcode
Where should instruction prefix be handled?
	Maybe shouldn't worry until x86 support is resumed

Operands come from the SYNTAX field, no longer the USAGE field

One of these should be created for each non-opcode operand from each SYNTAX field part 
Additionally, any immediate should map to something in the USAGE field#define uvd_INST_ADD		1

Operands do not map to entire instruction bytes
Ex: 8051 MUL assumes that B and A will be multiplied, storing the result in A
	MUL A, B
*/
class UVDConfigValue;
class UVDOperandShared
{
public:
	UVDOperandShared();
	~UVDOperandShared();
	virtual uv_err_t init();
	virtual uv_err_t deinit();

	//Returns error if it isn't an immediate
	//virtual uv_err_t getImmediateSize(uint32_t *immediateSizeOut) = 0;
};


/*
An instruction as parsed from a file
Hmm actually maybe we can keep this class non-virtual and do all virtual process in the shared class
Keep the void *data member
*/
class UVD;
class UVDInstruction;
class UVDInstructionIterator;
class UVDOperand
{
public:
	UVDOperand();
	virtual ~UVDOperand();
	virtual uv_err_t init();
	virtual uv_err_t deinit();

	//Will return UV_ERR_DONE to indicate incomplete parsing of the operand due to out of data
	virtual uv_err_t parseOperand(UVDInstructionIterator *out) = 0;

	//Append to given string
	virtual uv_err_t printDisassemblyOperand(std::string &out) = 0;

public:	
	//Type, name, etc
	//We do not own this
	UVDOperandShared *m_shared;
	//The instruction this operand belongs to, we do not own it, it owns us
	//UVDInstruction *m_instruction;
};


//Reserved for use with instruction prefixes
class UVDPrefix
//struct uv_inst_prefix_t
{
};

/*
Shared data among all instances of some given instruction
Information that would be contained in datasheet
Ex: "ADD" and "Adds two numbers together"
This information should be read from the config file
Note that fields are not always valid for prefixes and extensions
*/
class UVDInstructionShared
//struct uv_inst_shared_t
{
public:
	UVDInstructionShared();
	virtual ~UVDInstructionShared();
	virtual uv_err_t init();
	virtual uv_err_t deinit();

	//Like would be in developer manual
	virtual std::string getHumanReadableUsage();
};

/*
For querying specifics of what the instruction does
Returned as an object since this is computed, not intrinsic
Add functions to the instruction if this turns out to not be a good idea
*/
class UVDInstructionAnalysis
{
public:
	UVDInstructionAnalysis();
	~UVDInstructionAnalysis();
	
public:
	//if tristate is unknown, means we do not know the target

	uvd_tri_t m_isJump;
	uv_addr_t m_jumpTarget;
	
	uvd_tri_t m_isCall;
	uv_addr_t m_callTarget;
	
	uvd_bool_t m_isConditional;
	
	//Should put this as a fallback case?
	//std::map<std::string, std::string> m_misc;
};

/*
An instruction as parsed from a data source
*/
class UVDInstruction
//struct uv_inst_t
{
public:
	UVDInstruction();
	virtual ~UVDInstruction();
	//eh why is there no init?
	virtual uv_err_t init();
	virtual uv_err_t deinit();

	virtual uv_err_t parseCurrentInstruction(UVDInstructionIterator &out) = 0;

	virtual uv_err_t print_disasm(std::string &out) = 0;
	//Give as many hints to our analyzer as possible based on what this instruction does
	//If out is given, also fill in details to returned structure
	virtual uv_err_t analyzeControlFlow(UVDInstructionAnalysis *out = NULL) = 0;

public:
	/* Shared information for the primary instruction part such as a general description */
	UVDInstructionShared *m_shared;
	/* 
	Head of operand linked list.  
	Stored in Intel format, left to right
	ie destination followed by first operand, second, etc
	*/
	std::vector<UVDOperand *> m_operands;	
	
	/* offset in the source file */
	uint32_t m_offset;
	/* The raw instruction */
	uint32_t m_inst_size;
	/* With prefixes and such, this can be much longer than just a plain opcode */
	char m_inst[MAX_INST_SIZE];
	
	/* 8051 does not use prefixes */
	std::vector<UVDPrefix> m_prefixes;
};


const char *uvd_data_str(int uvd_data);

#endif

