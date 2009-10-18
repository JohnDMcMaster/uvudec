/*
Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under the terms of the BSD license.  See LICENSE for details.
*/

#pragma once

#include <map>
#include <string>
#include <vector>
#include "uvd_instruction.h"
#include "interpreter/uvd_interpreter.h"

//Resultant address from a call routine
#define SCRIPT_KEY_CALL				"CALL_ADDRESS"
//Resultant address from a jump
#define SCRIPT_KEY_JUMP				"JUMP_ADDRESS"
//Resultant data from arithmetic
//This requires non-static analysis
//#define SCRIPT_KEY_ARITMETIC		"ARIMETIC"
//An alternative representation of an address
#define SCRIPT_KEY_SYMBOL			"ADDRESS_SYMBOL"

class UVDConfigValue;
class UVDOperand;
class UVDOperandShared;
class UVDParsedFunction
//struct uv_disasm_parsed_func_t
{
public:
	std::vector<UVDConfigValue *> m_args;
};

class UVDConfigSection
//struct uv_disasm_config_section_t
{
public:
	UVDConfigSection();

public:
	std::string m_name;
	std::vector<std::string> m_lines;
	//TODO: these are here more of as a placeholder
	//Should replace these vectors with an entry ConfigEntry to track this properly
	//This would allow automatic removal of duplicate entries and maybe even hash map lookups
	std::vector<uint32_t> m_lineNumbers;
	int m_line;
};

/*
An actual value parsed from a configuration file

Types
Hex number
	0x21
Octal number
	012
Decimal number
	23
Unsigned 8 bit
	u8_0
Signed 16 bit
	s16_0
*/
class UVDConfigValue
//struct uv_disasm_parsed_t
{
public:
	UVDConfigValue();
	/*
	a UVDConfigValue is a raw structural form of the text
	It does bulk parsing work shared by both usage and syntax
	*/
	static uv_err_t parseType(const std::string &in_real, UVDConfigValue *out);

public:
	/*
	One of:
	#define UV_DISASM_DATA_REG					1
	#define UV_DISASM_DATA_OPCODE				3
	#define UV_DISASM_DATA_IMMS					32
	#define UV_DISASM_DATA_IMMU					33
	*/
	int m_operand_type;
	int m_num_bits;
	union
	{
		int m_value;
		UVDParsedFunction *m_func;
	};
	std::string m_name;
};

/*
General configuration options
Not related to formatting of a specific compiler (language)
*/
class UVDConfig
{
public:
	UVDConfig();
	
	static uv_err_t uvd_read_sections(const std::string &config_file, UVDConfigSection ***sections_in, unsigned int *n_sections_in);
	static uv_err_t readSections(const std::string config_file, std::vector<UVDConfigSection> sectionsIn);

public:
	std::string m_analysisDir;
	
	std::vector<std::string> m_args; 
	//Default interpreter to use for script files
	int m_configInterpreterLanguage;
};

//Default configuration options
extern UVDConfig *g_config;
