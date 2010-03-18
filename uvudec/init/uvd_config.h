/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#ifndef UVD_CONFIG
#define UVD_CONFIG

#include <map>
#include <set>
#include <string>
#include <vector>
#include "uvd_instruction.h"
#include "interpreter/uvd_interpreter.h"
#include "uvd_arg.h"
#include "uvd_config_flirt.h"

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
	UVDParsedFunction();
	~UVDParsedFunction();
	uv_err_t deinit();
	
public:
	std::vector<UVDConfigValue *> m_args;
};

class UVDConfigSection
//struct uv_disasm_config_section_t
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
//struct uv_disasm_parsed_t
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
	int m_operand_type;
	int m_num_bits;
	union
	{
		int m_value;
		UVDParsedFunction *m_func;
	};
	std::string m_name;
};

#define UVD__FLOW_ANALYSIS__INVALID				0
/*
Default
Iterate through each executable instruction one at a time
*/
#define UVD__FLOW_ANALYSIS__LINEAR				1
/*
Start at each vector and trace all possible branches
May miss functions called through pointers
*/
#define UVD__FLOW_ANALYSIS__TRACE				2

/*
General configuration options
Not related to formatting of a specific compiler (language)
*/
class UVDConfig
{
public:
	UVDConfig();
	~UVDConfig();
	
	uv_err_t init();
	uv_err_t deinit();
	
	static uv_err_t uvd_read_sections(const std::string &config_file, UVDConfigSection ***sections_in, unsigned int *n_sections_in);
	static uv_err_t readSections(const std::string config_file, std::vector<UVDConfigSection> sectionsIn);

	/*
	Parse info from main to setup our configuration
	*/
	uv_err_t parseMain(int argc, char **argv); 
	uv_err_t parseMain(int argc, char **argv, char **envp); 

protected:
	// ~/.uvudec file
	//Should be called before parseMain()...move this into init()
	uv_err_t parseUserConfig();

	/*
	Called from parseMain() to process config specific options
	*/
	uv_err_t processParseMain();

public:
	//TODO: move these into a general config structure?

	//if availible
	//used to print program name for usage
	int m_argc;
	char **m_argv;
	std::vector<std::string> m_args; 

	std::string m_analysisDir;
	int m_analysisOnly;
	int m_uselessASCIIArt;
	//Which type of flow analysis to do
	int m_flowAnalysisTechnique;
	//If any are set, will only output analysis of symbols at the given addresses
	std::set<int> m_analysisOutputAddresses; 
	
	//Default interpreter to use for script files
	int m_configInterpreterLanguage;
	
	//Configuration option parsing
	//Could bet set from command line, interactive shell, or a file
	std::vector<UVDArgConfig *> m_configArgs;
	
	std::string m_sDebugFile;
	//FILE *m_pDebugFile;
	
	//g_addr_min, g_addr_max
	uint32_t m_addressMin;
	uint32_t m_addressMax;

	//Callbacks
	//Prefix the version print information
	uv_thunk_t versionPrintPrefixThunk;
	//After the usage call, meant for misc notes
	uv_thunk_t usagePrintPostfixThunk;
	
	//g_print_used
	int m_printUsed;
	//g_jumped_sources
	int m_jumpedSources;
	//g_called_sources
	int m_calledSources;
	//g_addr_comment
	int m_addressComment;
	//g_addr_label
	int m_addressLabel;

	//TODO: re-impliment this as flags
	//g_verbose
	int m_verbose;
	//g_verbose_level
	int m_verbose_level;
	//g_verbose_init
	int m_verbose_init;
	//g_verbose_processing
	int m_verbose_processing;
	//g_verbose_analysis
	int m_verbose_analysis;
	//g_verbose_printing
	int m_verbose_printing;


	//uvd_format.h
	
	//unsigned int g_addr_min;
	unsigned int m_addr_min;
	//unsigned int g_addr_max;
	unsigned int m_addr_max;
	//int g_called_sources;
	int m_called_sources;
	//int g_called_count;
	int m_called_count;
	//int g_jumped_sources;
	int m_jumped_sources;
	//int g_jumped_count;
	int m_jumped_count;
	//int g_addr_comment;
	int m_addr_comment;
	//int g_addr_label;
	int m_addr_label;
	//How many hex digits to put on addresses 
	//unsigned int g_hex_addr_print_width;
	unsigned int m_hex_addr_print_width;
	//std::string g_mcu_name;
	std::string m_mcu_name;
	//std::string g_mcu_desc;
	std::string m_mcu_desc;
	//std::string g_asm_imm_prefix;
	std::string m_asm_imm_prefix;
	//std::string g_asm_imm_prefix_hex;
	std::string m_asm_imm_prefix_hex;
	//std::string g_asm_imm_postfix_hex;
	std::string m_asm_imm_postfix_hex;
	//std::string g_asm_imm_suffix;
	std::string m_asm_imm_suffix;
	/*
	If set, output should be capitalized
	This is a pretty trivial option, originally was for something that probably
	wasn't well enough thought out and should be eliminated
	*/
	//int g_caps;
	int m_caps;
	//int g_binary;
	int m_binary;
	//int g_memoric;
	int m_memoric;
	//int g_asm_instruction_info;
	int m_asm_instruction_info;
	//int g_print_used;
	int m_print_used;
	//int g_print_string_table;
	int m_print_string_table;
	//Internal ID used to represent blocks.  Intended for debugging
	//int g_print_block_id;
	int m_print_block_id;
	//int g_print_header;
	int m_print_header;
	//nothing (Intel), $ (MIPS) and % (gcc) are common
	//char g_reg_prefix[8]
	std::string m_reg_prefix;
	
	//FLIRT related options (flirt.*)
	UVDConfigFLIRT m_flirt;
};

//Default configuration options
extern UVDConfig *g_config;

#endif
