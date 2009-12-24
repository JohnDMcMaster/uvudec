/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#pragma once

#include "uvd_compiler.h"

/*
TODO: clean this up.  Its left over from C stuff
Make a formatter class (and use it...)
*/

extern unsigned int g_addr_min;
//int g_addr_max = 0;
extern unsigned int g_addr_max;
extern int g_called_sources;
extern int g_called_count;
extern int g_jumped_sources;
extern int g_jumped_count;
extern int g_addr_comment;
extern int g_addr_label;
//How many hex digits to put on addresses 
extern unsigned int g_hex_addr_print_width;
extern std::string g_mcu_name;
extern std::string g_mcu_desc;
extern std::string g_asm_imm_prefix;
extern std::string g_asm_imm_prefix_hex;
extern std::string g_asm_imm_postfix_hex;
extern std::string g_asm_imm_suffix;
extern int g_caps;
extern int g_binary;
extern int g_memoric;
extern int g_asm_instruction_info;
extern int g_print_used;
extern int g_print_string_table;
//Internal ID used to represent blocks.  Intended for debugging
extern int g_print_block_id;
extern int g_print_header;


#define UV_DISASM_RET_BUFF_SIZE			1024
#define PRINT_OPERAND_SEPERATOR			", "

/* nothing (Intel), $ (MIPS) and % (gcc) are common */
extern char reg_prefix[8];

//How many instances of the library are open
//static unsigned int g_open_instances = 0;

//This should be initialized during usage, and so not here
extern char g_uvd_ret_buff[UV_DISASM_RET_BUFF_SIZE];

void printFormatting();

class UVDFormat
{
public:
	UVDFormat();
	
	uv_err_t init();
	
	std::string formatAddress(uint32_t address);
	std::string formatRegister(const std::string &reg);
	
	void printFormatting();

public:
	//Output data has to be formatted to a given compiler
	UVDCompiler *m_compiler;
};

