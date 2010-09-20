/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_CONFIG
#define UVD_CONFIG

#include <map>
#include <set>
#include <string>
#include <vector>
#include "interpreter/uvd_interpreter.h"
#include "uvd_arg.h"
#include "uvd_instruction.h"
#include "uvd_priority_list.h"
#include "flirt/config.h"
#include "plugin/plugin.h"
#include "plugin/engine.h"

//Resultant address from a call routine
#define SCRIPT_KEY_CALL				"CALL_ADDRESS"
//Resultant address from a jump
#define SCRIPT_KEY_JUMP				"JUMP_ADDRESS"
//Resultant data from arithmetic
//This requires non-static analysis
//#define SCRIPT_KEY_ARITMETIC		"ARIMETIC"
//An alternative representation of an address
#define SCRIPT_KEY_SYMBOL			"ADDRESS_SYMBOL"

/*
To control whether addresses are analyzed or not
Some more specialized types might be added later if necessary
These apply only to config passed in arguments and may not reflect the entire range of tags applied to address areas,
such as string tables discovered during analysis
XXX: it may be desirable, however, to later unify these
*/
//Invalid value
#define UVD_ADDRESS_ANALYSIS_UNKNOWN			0
//Force analysis
#define UVD_ADDRESS_ANALYSIS_INCLUDE			1
//Do not analyze
#define UVD_ADDRESS_ANALYSIS_EXCLUDE			2


class UVDPluginConfig
{
public:
	UVDPluginConfig();
	~UVDPluginConfig();

	uv_err_t init(UVDConfig *config);
	uv_err_t deinit();

	//Add a plugin to be loaded
	uv_err_t addPlugin(const std::string &pluginLibraryName);
	uv_err_t appendPluginPath(const std::string &path);
	uv_err_t prependPluginPath(const std::string &path);

public:
	//To activate at startup
	//std::vector<std::string> m_plugins;
	//renamed
	std::vector<std::string> m_toLoad;

	//All of these will be added to plugin selection lists and have their main called
	//std::vector<std::string> m_pluginDirs;
	std::vector<std::string> m_dirs;

	UVDPluginEngine m_pluginEngine;
};

class UVDConfigSymbols
{
public:
	UVDConfigSymbols();
	~UVDConfigSymbols();

	uv_err_t init();
	uv_err_t deinit();
	
	uv_err_t getSymbolTypeNamePrefix(int symbolType, std::string &ret);

public:
	//A prefix to put before every symbol generated
	//To tag this was generated from analysis here
	std::string m_autoNameUvudecPrefix;
	//Should the name of the data source be prefixed to the output symbols?
	uint32_t m_autoNameMangeledDataSource;
	//If above is set, a string to put between the generated name and the rest of the symbol
	std::string m_autoNameMangeledDataSourceDelim;
	
	//Symbol type naming
	std::string m_autoNameUnknownPrefix;
	std::string m_autoNameFunctionPrefix;
	std::string m_autoNameLabelPrefix;
	std::string m_autoNameROMPrefix;
	std::string m_autoNameVariablePrefix;
};

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
	
	/*
	Parse info from main to setup our configuration
	*/
	uv_err_t parseMain(int argc, char *const *argv); 
	uv_err_t parseMain(int argc, char *const *argv, char *const *envp); 
	
	//Include or exclude addresses from analysis
	uv_err_t addAddressInclusion(uint32_t low, uint32_t high);
	uv_err_t addAddressExclusion(uint32_t low, uint32_t high);
	//As per configuration, get a strictly increasing range of all valid analysis address ranges
	//Two adjacent ranges must have at least one non-analyzed address in between
	uv_err_t getValidAddressRanges(std::vector<UVDRangePair> &ranges);
	
	//Note these are CONFIGURATION limits, not necessarily anywhere neear whats actually allowed
	//By default this will be from 0 to UINT_MAX and the program may only be from say 0x0000 to 0xFFFF
	//If no vaddresses are valid, these should probably error
	//Currently they'd return UV_ERR_DONE
	uv_err_t getAddressMin(uint32_t *addr);
	uv_err_t getAddressMax(uint32_t *addr);
	
	//The following two should be used to construct blocks valid for analysis in alternating fashion
	//based on the configuration settings here
	//Including the given value as a canidate, return the next address valid for analysis
	//If no more addresses are valid, returns the success code UV_ERR_DONE
	uv_err_t nextValidAddress(uint32_t start, uint32_t *ret);
	//Including the given value as a canidate, return the next address invalid for analysis
	//If no more addresses are invalid, returns the success code UV_ERR_DONE
	uv_err_t nextInvalidAddress(uint32_t start, uint32_t *ret);
	//Extend rules above, but going in reverse
	uv_err_t lastValidAddress(uint32_t start, uint32_t *ret);
	uv_err_t lastInvalidAddress(uint32_t start, uint32_t *ret);

	//Are any of the verbose (debug) flags set?
	bool anyVerboseActive();
	//Activate all verbose flags
	void setVerboseAll();
	void clearVerboseAll();

	uv_err_t setConfigInterpreterLanguageInterface(const std::string &in);
	uv_err_t setConfigInterpreterLanguage(const std::string &in);

	uv_err_t registerArgument(const std::string &propertyForm,
			char shortForm, std::string longForm, 
			std::string helpMessage,
			uint32_t numberExpectedValues,
			UVDArgConfigHandler handler,
			bool hasDefault);
	uv_err_t registerArgument(const std::string &propertyForm,
			char shortForm, std::string longForm, 
			std::string helpMessage,
			std::string helpMessageExtra,
			uint32_t numberExpectedValues,
			UVDArgConfigHandler handler,
			bool hasDefault);

protected:
	// ~/.uvudec file
	//Should be called before parseMain()...move this into init()
	uv_err_t parseUserConfig();

	/*
	Called from parseMain() to process config specific options
	*/
	uv_err_t processParseMain();

	uv_err_t nextAddressState(uint32_t start, uint32_t *ret, uint32_t targetState);
	uv_err_t lastAddressState(uint32_t start, uint32_t *ret, uint32_t targetState);

public:
	//TODO: move these into a general config structure?

	//if availible
	//used to print program name for usage
	int m_argc;
	char *const *m_argv;
	std::vector<std::string> m_args;

	//The binary we are analyzing, it from a file
	//The primary source of this information should be uvd's UVDData and this is more for init purposes
	std::string m_targetFileName;
	//The file that will be used to load the CPU module and such
	std::string m_architectureFileName;

	//Canonical name where our install was to
	std::string m_installDir;
	//Canonical name where the arch files are stored
	//TODO: make this a vector of search paths, either absolute or relative to current dir
	std::string m_archDir;

	std::string m_analysisDir;
	int m_analysisOnly;
	int m_uselessASCIIArt;
	//Which type of flow analysis to do
	int m_flowAnalysisTechnique;
	//If any are set, will only output analysis of symbols at the given addresses
	std::set<int> m_analysisOutputAddresses;
	
	std::string m_rawFileSuffix;
	std::string m_relocatableFileSuffix;
	std::string m_elfFileSuffix;

	//Default interpreter to use for script files
	uint32_t m_configInterpreterLanguage;
	uint32_t m_configInterpreterLanguageInterface;
	
	//Configuration option parsing
	//Could bet set from command line, interactive shell, or a file
	std::vector<UVDArgConfig *> m_configArgs;
	
	std::string m_sDebugFile;
	//FILE *m_pDebugFile;
	
	//Callbacks
	//Prefix the version print information
	uv_thunk_t versionPrintPrefixThunk;
	//After the usage call, meant for misc notes
	uv_thunk_t usagePrintPostfixThunk;
	
	//g_print_used
	int m_printUsed;
	//g_jumped_sources
	int m_jumpedSources;
	int m_jumpedCount;
	//g_called_sources
	int m_calledSources;
	int m_calledCount;
	//g_addr_comment
	int m_addressComment;
	//g_addr_label
	int m_addressLabel;

	//TODO: re-impliment this as flags
	int m_verbose;
	int m_verbose_level;
	//Program sections
	int m_verbose_args;
	int m_verbose_init;
	int m_verbose_processing;
	int m_verbose_analysis;
	int m_verbose_printing;

	//Different areas of code (modules: engines, plugins, etc)
	//map of dedicated flags and strings used to print them, currently for debugging purposes
	std::map<uint32_t, std::string> m_modulePrefixes;

	//The following will place comments and try the best of their abilities to continue
	//if they are told to ignore errors
	//Should we error if we don't have enough data for an instruction?
	int m_haltOnTruncatedInstruction;
	//Should we error if we don't recognize an opcode?
	int m_haltOnInvalidOpcode;

	//uvd_format.h
	
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
	

	//uvd_analysis_db.cpp declared
	//Raw function MD5
	uint32_t m_computeFunctionMD5;
	//Relocatable function version MD5
	//Relocatable parts should be 0'd
	uint32_t m_computeFunctionRelocatableMD5;

	//Write a .bin file exactly as the function was found
	uint32_t m_writeRawBinary;
	//Write a .bin file with default relocatable values (MD5 should match config MD5)
	//Implies writting out a complimentary file describing in text the relocations
	uint32_t m_writeRelocatableBinary;
	//Write an ELF format relocatable data
	uint32_t m_writeElfFile;
	//When analysis is written, a summary file is written
	//Idea was to make IDA .pat style file for storing function analysis
	std::string m_functionIndexFilename;


	//Automatic symbol naming
	UVDConfigSymbols m_symbols;
	//FLIRT related options (flirt.*)
	UVDConfigFLIRT m_flirt;
	//The address ranges that should/shouldn't be analyzed
	//Later might add in some other stuff like differentiating between addresses skipped for analysis and actually not present
	UVDUint32RangePriorityList m_addressRangeValidity;

	UVDPluginConfig m_plugin;
};

//The following are internal use only
uv_err_t UVDInitConfig();
uv_err_t UVDInitConfigEarly();

//Default configuration options
//Deprecated, this will be removed as an exported symbol in the future
extern UVDConfig *g_config;
//Internal use only
//Returns the singleton config instance
//In future, we may (although unlikely for some time) allow multiple engines to be loaded with separate configs
UVDConfig *UVDGetConfig();

#endif
