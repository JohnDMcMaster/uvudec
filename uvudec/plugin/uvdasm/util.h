/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVDASM_UTIL_H
#define UVDASM_UTIL_H

#include "uvd/util/types.h"

class UVDConfigValue;
class UVDOperand;
class UVDOperandShared;
class UVDParsedFunction
{
public:
	UVDParsedFunction();
	~UVDParsedFunction();
	uv_err_t deinit();
	
public:
	std::vector<UVDConfigValue *> m_args;
};

class UVDConfigSection
{
public:
	UVDConfigSection();
	~UVDConfigSection();

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
{
public:
	UVDConfigValue();
	~UVDConfigValue();
	uv_err_t deinit();

	/*
	a UVDConfigValue is a raw structural form of the text
	It does bulk parsing work shared by both usage and syntax
	*/
	static uv_err_t parseType(const std::string &in_real, UVDConfigValue *out);

private:
	static uv_err_t parseTypeNumber(const std::string &in, UVDConfigValue *out);

public:
	/*
	One of:
	#define UV_DISASM_DATA_REG					1
	#define UV_DISASM_DATA_OPCODE				3
	#define UV_DISASM_DATA_IMMS					32
	#define UV_DISASM_DATA_IMMU					33
	*/
	uint32_t m_operand_type;
	int m_num_bits;
	//To support ranges of values
	//If set to 0, not specified
	uint32_t m_bitmask;
	//To support ranges of values
	//If set to 0, not specified
	//Else, set to how many additional opcodes are supported
	uint32_t m_opcodeRangeOffset;
	union
	{
		int m_value;
		UVDParsedFunction *m_func;
	};
	std::string m_name;
};

class UVDAsmUtil
{
public:
	static uv_err_t uvd_read_sections(const std::string &config_file, UVDConfigSection ***sections_in, unsigned int *n_sections_in);
	static uv_err_t readSections(const std::string config_file, std::vector<UVDConfigSection> sectionsIn);

	/*
	a UVDConfigValue is a raw structural form of the text
	It does bulk parsing work shared by both usage and syntax
	*/
	static uv_err_t parseType(const std::string &in_real, UVDConfigValue *out);

private:
	static uv_err_t parseTypeNumber(const std::string &in, UVDConfigValue *out);
};

#endif

