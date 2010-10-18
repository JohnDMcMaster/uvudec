/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVDASM_OPCODE_TABLE_H
#define UVDASM_OPCODE_TABLE_H

#include "uvdasm/instruction.h"
#include "uvdasm/interpreter.h"
#include "uvd/util/types.h"

#include <string>

/*
Byte based lookup table
Should be ultimatly loaded from an opcode file
*/

class UVDDisasmOpcodeLookupTable
{
public:
	UVDDisasmOpcodeLookupTable();
	~UVDDisasmOpcodeLookupTable();
	
	/*
	Register a lookup for opcode to return newElement
	
	If old opcode is present, it will be assinged to oldElement if non-null
	Otherwise, old opcode will be deleted
	
	A value of NULL indicates an non-coding opcode.  
	It will be an error to lookup an opcode with such a value
	*/
	uv_err_t registerOpcode(unsigned char opcode, UVDDisasmInstructionShared newElement, UVDDisasmInstructionShared **oldElement = NULL);

	/*
	Show statistics about how much of the table is active
	*/
	void usageStats(void);
	/*
	Show statistic about how much the table was used for the last program
	*/
	void usedStats(void);

	/*
	Opcodes are not allowed here as opposed to uvd_parse_syntax
	This function also recursivly calls itself to handle function arguments
	*/
	uv_err_t uvd_parse_syntax_operand(UVDDisasmOperandShared **op_shared_in, const std::string cur);

	/* 
	Syntax
	How the instruction looks during assembly
	-Should not 
		-imply anything about how much actual space they use
		-Have opcodes
	-Should
		-Have immediates
			-Must have name matching USAGE, as such will have size implied
			-Does not imply size because it is possible for examle to have ADD 0x10 be a single opcode
		-Have registers
		-Have memory usage
		-Not have to be in the same order as the USAGE
		-Setup operand structures as all of them appear during syntax
	*/
	uv_err_t uvd_parse_syntax(UVDDisasmInstructionShared *inst_shared, const std::string value_syntax);

	uv_err_t uvd_match_syntax_usage_core(std::vector<UVDDisasmOperandShared *>op_shareds,
			UVDConfigValue *parsed_type, 
			UVDDisasmOperandShared **op_shared_out);

	uv_err_t uvd_match_syntax_usage(UVDDisasmInstructionShared *inst_shared,
			UVDConfigValue *parsed_type, 
			UVDDisasmOperandShared **op_shared_out);


	/* 
	USAGE field
	How the instruction looks in binary
	The SYNTAX field must have already been parsed before this to setup the operand structs
	The main job of this field is to
	-Map immediates to arguments as seen in an assembler
	-Figure out how long the entire instruction is so the next instruction class can be deciphered
	*/
	uv_err_t uvd_parse_usage(UVDDisasmInstructionShared *inst_shared, const std::string value_usage);

	uv_err_t init(UVDConfigSection *op_section);
	uv_err_t deinit(void);
	uv_err_t init_opcode(UVDConfigSection *op_section);
	uv_err_t getElement(unsigned int index, UVDDisasmInstructionShared **element);
	
	
public:
	//Change this to a map?
	UVDDisasmInstructionShared *m_lookupTable[0x100];
	unsigned int m_lookupTableHits[0x100];

	UVDConfigExpressionInterpreter *m_interpreter;
};

extern int g_error_opcode_repeat;

#endif

