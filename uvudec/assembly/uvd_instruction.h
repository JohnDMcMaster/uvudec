/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

/*
TODO:
This is an ancient file in the history of this project
The code may be a bit archaic and needs some cleanup
*/

#pragma once

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

//These are part of operands, which already have a name
//Thus certain object such as that are uncessary
class UVDOperandShared;
class UVDFunctionShared
//struct uv_disasm_func_shared_t
{
public:
	UVDFunctionShared();
	~UVDFunctionShared();
	uv_err_t deinit();

public:
	std::vector<UVDOperandShared *> m_args;
};

class UVDOperand;
class UVDFunction
//struct uv_disasm_func_t
{
public:
	UVDFunction();
	~UVDFunction();
	uv_err_t deinit();

public:
	std::vector<UVDOperand *> m_args;
};

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
	uv_err_t deinit();

	static uv_err_t uvd_parsed2opshared(const UVDConfigValue *parsed_type, UVDOperandShared **op_shared_in);

	//Returns error if it isn't an immediate
	uv_err_t getImmediateSize(uint32_t *immediateSizeOut);

public:
	/* Register, memory, immediate */
	int m_type;

	/*
	Register: malloc'd register name
	Immediate: single integer (not a pointer) indicating size in bits
		Try to phase out immediate defines
	Memory:
	*/
	union
	{
		void *m_type_specific;
		int m_immediate_size;
		UVDFunctionShared *m_func;
		/* struct uvd_reg_shared_t *m_reg; */
	};

	/* Symbolic name as defined in the .op file's USAGE field */
	std::string m_name;
	
	//UVDOperandShared *m_next;

};

/*
An instruction as parsed from a file
Replacement for uv_inst_operand_t
*/
class UVD;
class UVDInstruction;
class UVDOperand
//struct uv_inst_operand_t
{
public:
	UVDOperand();
	~UVDOperand();
	uv_err_t deinit();

	uv_err_t parseOperand(uint32_t &nextPosition);

	uv_err_t uvd_parsed2opshared(const struct uvd_parsed_t *parsed_type, UVDOperandShared **op_shared_in);
	//Append to given string
	uv_err_t printDisassemblyOperand(std::string &out);
	uv_err_t print_disasm_operand(char *buff, unsigned int buffsz, unsigned int *buff_used_in);

	//Get a variable mapping suitable for scripting
	//If name returns empty, is not applicable
	uv_err_t getVariable(std::string &name, UVDVarient &value);

	//Does not have to be original size
	//Must match signedness
	uv_err_t getUI32Representation(uint32_t &i);
	uv_err_t getI32Representation(int32_t &i);

	//Created to give representations that might allow for binary to pratical value translations
	//Ex: relative jump contains binary 3, but really is PC + 3 = say 123
	uv_err_t getUI32RepresentationAdjusted(uint32_t &i);
	uv_err_t getI32RepresentationAdjusted(int32_t &i);

public:

	/* Decomposition is hard, don't deal with for now
	//<imm32=>0x23456789
	const std::string loc_decomp_pre;
	//%ebp<(r=5)>
	const std::string loc_decomp_post;
	*/
	
	/* Type, name, etc */
	UVDOperandShared *m_shared;
	//The instruction this operand belongs to, we do not own it, it owns us
	UVDInstruction *m_instruction;
	
private:
	/* 
	Additional information for this operand, such as a value larger than simple field supports 
	Interpretation is operand specific and determined by m_shared
	
	REG: std::string  to register name
	IMMEDIATE: larger than sizeof(value): pointer to raw byte stream
		Support for such data isn't planned in near future
	*/
	union
	{
		void *m_extra;
		
		UVDFunction *m_func;

		/* Many operands are immediates of some time or another */
		uint8_t m_ui8;
		int8_t m_i8;

		uint16_t m_ui16;
		int16_t m_i16;

		uint32_t m_ui32;
		int32_t m_i32;
	};
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
	~UVDInstructionShared();
	uv_err_t deinit();

	//Like would be in developer manual
	std::string getHumanReadableUsage();
	//Make ready instruction class
	uv_err_t analyzeAction();
	//For doing certain analysis shortcuts
	//Means action is a single function with a single identifier as an argument
	uv_err_t isImmediateOnlyFunction();
	//identifier size in bits
	uv_err_t getImmediateOnlyFunctionAttributes(/*std::string &func,
			std::string &identifier, */uint32_t *identifierSizeBitsOut, uint32_t *immediateOffsetOut);

private:
	uv_err_t isImmediateOnlyFunctionCore();

public:

	//Primary descrpition of what it does
	//imm32=<0x23456789> or <ESP>
	/* The memoric */
	std::string m_memoric;
 	/* A longer text description */
	std::string m_desc;
 	/* Action script command */
	std::string m_action;
	
	/* The following two will likely have to be redesigned, probably better to store one byte at a time */
	/* Base opcode */
	uint8_t m_opcode[MAX_OPCODE_SIZE];
	/* How many bytes the opcode is */
	uint32_t m_opcode_length;
	/*
	Including immediates, prefix, etc, how long 
	Not valid for the following:
		-Prefixes
		-Extensions
		We'll see if something else comes up
	This will likely server as an estimate only
	*/
	uint32_t m_total_length;
	/*
	Some instructions are valid for a range of opcodes, but aren't aligned on nice bit masks 
	So, just use this
	Discontinuous opcodes should be specified in different structures
	A range of 0 means only this instruction, 1 would indicate the next one as well, etc
	*/
	uint32_t m_opcode_range_offset;

	/* 
	This can be trickey.
	Ex: branch not taken: 2 cycles, branch taken: 3 cycles
	However, even an approx num for now could be useful
	Use cpi hi and low to address the corner cases
	*/
	uint32_t m_cpi;
	uint32_t m_cpi_low;
	uint32_t m_cpi_hi;
	
	/* used to parse out from binary, head of usage linked list */
	/*struct uv_inst_usage_t *usage;*/

	/*
	Instruction class 
	If class is "prefix" or "extension", many of these fields do not hold meaningful values
	*/
	uint32_t m_inst_class;

	/*
	head of operand linked list 
	Note that the usage information is part of the operands
	*/
	std::vector<UVDOperandShared *> m_operands;	

	/* Information needed to recongize the instruction */
	//struct uv_inst_parse_t *m_parse;

	/* Configuration file line that it came from */
	uint32_t m_config_line_syntax;
	uint32_t m_config_line_usage;
	
	uv_err_t m_isImmediateOnlyFunction;
};


/*
An instruction as parsed from a data source
*/
class UVDInstruction
//struct uv_inst_t
{
public:
	UVDInstruction();
	~UVDInstruction();
	uv_err_t deinit();

	uv_err_t collectVariables(UVDVariableMap &environment);

	uv_err_t parseOperands(uint32_t &nextPosition, std::vector<UVDOperandShared *> ops_shared, std::vector<UVDOperand *> &operands);

public:

	/* Prints a text description of the given instruction to the buffer */
	uv_err_t print_disasm(char *buff, uint32_t buffsz);
	uv_err_t print_disasm(std::string &s);
	
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
	
	UVD *m_uvd;
};

const char *uvd_data_str(int uvd_data);
