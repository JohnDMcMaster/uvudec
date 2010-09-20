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
#include "uvd_config.h"
#include "uvd_data.h"
#include "uvd_types.h"

/*
Formats according to option specifications
Prints to global disassembly string return buffer (g_uv_disasm_ret_buff)
*/
#define UV_DISASM_DATA_NONE					0
#define UV_DISASM_DATA_REG					1
//#define UV_DISASM_DATA_MEM					2
#define UV_DISASM_DATA_OPCODE				3
/* Immediate, signed */
#define UV_DISASM_DATA_IMMS					32
#define UV_DISASM_DATA_IMMU					33
/* func like syntax used to express things like the memory type */
#define UV_DISASM_DATA_FUNC					64
/*
#define UV_DISASM_DATA_IMM_BASE				32
/ * What would be better is to do bitwise shifts * /
#define UV_DISASM_DATA_IMM8					(UV_DISASM_DATA_IMM_BASE + 8)
#define UV_DISASM_DATA_IMM16				(UV_DISASM_DATA_IMM_BASE + 16)
#define UV_DISASM_DATA_IMM32				(UV_DISASM_DATA_IMM_BASE + 32)
#define UV_DISASM_DATA_IMM64				(UV_DISASM_DATA_IMM_BASE + 64)
#define UV_DISASM_DATA_IMM128				(UV_DISASM_DATA_IMM_BASE + 128)
#define UV_DISASM_DATA_IMM_BITS(x)			(x - UV_DISASM_DATA_IMM_BASE)
*/
//There are some other weird index + offset types, need to do those


/*
An instruction with inherent data to that architecture
Replacement for uv_inst_shared_t
*/
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
class UVDIteratorCommon;
class UVDOperand
{
public:
	UVDOperand();
	virtual ~UVDOperand();
	virtual uv_err_t init();
	virtual uv_err_t deinit();

	//Will return UV_ERR_DONE to indicate incomplete parsing of the operand due to out of data
	virtual uv_err_t parseOperand(UVDIteratorCommon *uvdIter) = 0;

	//Append to given string
	virtual uv_err_t printDisassemblyOperand(std::string &out) = 0;

public:	
	//Type, name, etc
	//We do not own this
	UVDOperandShared *m_shared;
	//The instruction this operand belongs to, we do not own it, it owns us
	UVDInstruction *m_instruction;
};


//Reserved for use with instruction prefixes
class UVDPrefix
//struct uv_inst_prefix_t
{
};

#define UVD_INSTRUCTION_CLASS_UNKNOWN					0
//Assembly instruction hints
//Code that, possibly on a condition, goes to another address
#define UVD_INSTRUCTION_CLASS_JUMP						1
//Calls a subroutine
#define UVD_INSTRUCTION_CLASS_CALL						2
//Returns from a subroutine
#define UVD_INSTRUCTION_CLASS_RETURN					3
//No operation
#define UVD_INSTRUCTION_CLASS_NOP						4
//Arithmetic: add, subtract, multiply, bitwise shift, etc
#define UVD_INSTRUCTION_CLASS_ARITHMETIC				5
//Move data.  Includes between registers and memory
#define UVD_INSTRUCTION_CLASS_MOV						6

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

	virtual uv_err_t parseCurrentInstruction(UVDIteratorCommon &iterCommon) = 0;

	virtual uv_err_t print_disasm(std::string &s) = 0;
	//Give as many hints to our analyzer as possible based on what this instruction does
	virtual uv_err_t analyzeControlFlow() = 0;

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

