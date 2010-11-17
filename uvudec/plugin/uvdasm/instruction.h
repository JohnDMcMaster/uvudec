/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVDASM_INSTRUCTION_H
#define UVDASM_INSTRUCTION_H

#include "uvdasm/function.h"
#include "uvdasm/operand.h"
#include "uvdasm/util.h"

/*
Formats according to option specifications
Prints to global disassembly string return buffer (g_uv_disasm_ret_buff)
FIXME: consider polymorphism instead
*/
#define UV_DISASM_DATA_NONE					0
#define UV_DISASM_DATA_REG					1
//#define UV_DISASM_DATA_MEM				2
/*
Most important example of this is an opcode
However, this also applies to constant syntax parts, such as x86's INT3
*/
#define UV_DISASM_DATA_CONSTANT				3
/* Immediate, signed */
#define UV_DISASM_DATA_IMMS					32
#define UV_DISASM_DATA_IMMU					33
/* func like syntax used to express things like the memory type */
#define UV_DISASM_DATA_FUNC					64

class UVDDisasmInstructionShared : public UVDInstructionShared
{
public:
	UVDDisasmInstructionShared();
	~UVDDisasmInstructionShared();
	
	virtual uv_err_t init();
	virtual uv_err_t deinit();

	//For doing certain analysis shortcuts
	//Means action is a single function with a single identifier as an argument
	uv_err_t isImmediateOnlyFunction();
	//identifier size in bits
	uv_err_t getImmediateOnlyFunctionAttributes(/*std::string &func,
			std::string &identifier, */uint32_t *identifierSizeBitsOut, uint32_t *immediateOffsetOut);

	virtual std::string getHumanReadableUsage();

	//Partially to rapidly support FLIRT signature creation,
	//opcodes were masked and others to allow rapid decoding of register ranges
	uv_err_t getOpcodes(std::set<uint8_t> &out) const;

	//Make ready instruction class
	uv_err_t analyzeAction();

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
	/* How many bytes the opcode is */
	uint32_t m_opcode_length;
	/* Base opcode */
	uint8_t m_opcode[MAX_OPCODE_SIZE];
	/*
	Some instructions are valid for a range of opcodes, but aren't aligned on nice bit masks 
	So, just use this
	Discontinuous opcodes should be specified in different structures
	A range of 0 means only this instruction, 1 would indicate the next one as well, etc
	*/
	uint32_t m_opcodeRangeOffset[MAX_OPCODE_SIZE];
	uint32_t m_bitmask[MAX_OPCODE_SIZE];

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
	//uint32_t m_inst_class;
	//Z80 for example has conditional jumps...so don't make bad assumptions
	//Make additional flags as needed, maybe we should just do char 
	uvd_bool_t m_isJump;
	uvd_bool_t m_isCall;
	//Instruction only executes under some given condition
	//FIXME: replace with a pointer to a conditional
	uvd_bool_t m_isConditional;
	/*
	XXX: placeholder
	Regardless of conditional result, continue to execute so many instructions after
	Intended for MIPS like where they decided its better to not flush the pipeline for a jump/call
	in exchange for more creative instruction ordering
	*/
	uint32_t m_conditionalExtraInstructions;

	/*
	head of operand linked list 
	Note that the usage information is part of the operands
	*/
	std::vector<UVDDisasmOperandShared *> m_operands;	

	/* Information needed to recongize the instruction */
	//struct uv_inst_parse_t *m_parse;

	/* Configuration file line that it came from */
	uint32_t m_config_line_syntax;
	uint32_t m_config_line_usage;
	
	uv_err_t m_isImmediateOnlyFunction;
};

class UVDDisasmInstruction : public UVDInstruction
{
public:
	UVDDisasmInstruction();
	~UVDDisasmInstruction();
	
	virtual uv_err_t init();
	virtual uv_err_t deinit();

	UVDDisasmInstructionShared *getShared();

	/* Prints a text description of the given instruction to the buffer */
	//uv_err_t print_disasm(char *buff, uint32_t buffsz);
	virtual uv_err_t print_disasm(std::string &out);
	
	virtual uv_err_t analyzeControlFlow();

	virtual uv_err_t parseCurrentInstruction(UVDIteratorCommon &iterCommon);

	static uv_err_t parseOperands(UVDIteratorCommon *uvdIter,
			std::vector<UVDDisasmOperandShared *> ops_shared, std::vector<UVDOperand *> &operands);

	//Hmm is this UVDDisasm specifc?
	virtual uv_err_t collectVariables(UVDVariableMap &environment);

	uv_err_t analyzeCall(uint32_t startPos, const UVDVariableMap &attributes);
	uv_err_t analyzeJump(uint32_t startPos, const UVDVariableMap &attributes);

public:	
	//FIXME: this should be arch pointer, not uvd
	UVD *m_uvd;
};

#endif

