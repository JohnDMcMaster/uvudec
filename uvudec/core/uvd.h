/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#ifndef UV_DISASM_H
#define UV_DISASM_H

#include <stdint.h>
#include <map>
#include <list>
#include <set>
#include <string>
#include <vector>
#include "uvd_error.h"
#include "uvd_analyzer.h"
#include "uvd_data.h"
#include "uvd_format.h"
#include "uvd_instruction.h"
#include "uvd_opcode.h"
#include "uvd_register.h"
#include "uvd_config_symbol.h"
#include "uvd_types.h"

//From linux/stringify.h
#define __stringify_1(x)	#x
#define __stringify(x)		__stringify_1(x)

#define UVUDEC_VER_STRING 		__stringify(UVUDEC_VER_MAJOR) "." __stringify(UVUDEC_VER_MINOR) "." __stringify(UVUDEC_VER_PATCH) "." __stringify(UVUDEC_VER_BUILD)

/* Used for opcode processing funcs */
//static uvd_func opcode_map[256];
/* on 8051, its a fairly starightforward map from numbers to opcode descriptions */
//extern struct uv_inst_shared_t *opcode_structs[256];


/*
Primary actions
*/
//If not set, UV_ERROR_OK for each instruction, UV_ERROR_DONE when no more
#define UV_DISASM_ACTION_ALL					0
//Rather than disassembling just one instruction, do as many as possible
//Instead of returing a const std::string , will return a std::string  that must be free'd
#define UV_DISASM_ACTION_SINGLE					1
//Don't actually output, just see if we like the assembly we see
#define UV_DISASM_ACTION_ERROR_CHECK			2
/*
Try to figure out the primary language of the executable
Often times compiler is coded into the files or certain symbols (such as STL
references) can be used to figure things out
Note that every func can be, in theory, written in a different language
*/
#define UV_DISASM_ACTION_DETECT_Language		3

/*
Options
*/

/*
If set, output should be capitalized
This is a pretty trivial option, originally was for something that probably
wasn't well enough thought out and should be eliminated
*/
extern int g_caps;

uv_err_t uvd_init_equiv_mem(void);

/* Global symbol management */
uv_err_t uvd_set_sym(const std::string key, 
		struct uvd_sym_t *value, struct uvd_sym_t **old_value);
uv_err_t uvd_get_sym(const std::string key, struct uvd_sym_t **value);

//nasm style. Uses % in front of registers.  mov %src_reg, %dest_reg
#define UV_DISASM_STYLE_ATT						1
//masm style?.   MOV DEST_REG, SRC_REG
#define UV_DISASM_STYLE_INTEL					2
//is MIPS its own style?  Seems MIPS asm is standardized
//extern int g_asm_style;
//Print how instructions are derrived?
//extern int g_deriv;

/*
Options common to many different things we could do
*/
//Show how instructions are derrived
//Instead of "xor %ebp,%ebp", use XOR (0x31) /r, r = 0xED (ebp)
#define UV_DISASM_OPTION_DERRIVED				0x0001
//Print out file addresses during disassembly
#define UV_DISASM_SUBACTION_FILE				0x0002
//Print out memory address during disassembly
#define UV_DISASM_SUBACTION_MEMORY				0x0004
//Print out the actual hex data
#define UV_DISASM_SUBACTION_RAW_DATA			0x0008
/*
Try to figure out if there are any major logical divisions between funcs
that may indicate they come from different source files
Primarily, look for C++ classes and perhaps groups of funcs that start with
same name
Note that templated classes will likely turn out plain ugly
Maybe try to identify them and filter them out
*/
#define UV_DISASM_SUBACTION_TEXT_LOWERCASE		0x0010
//Don't break on possibly recoverable errors?
#define UV_DISASM_SUBACTION_RECOVER				0x0020
//Decode what the memoric stands for
//Ex: ASCII Adjust after Addition (AAA)
#define UV_DISASM_SUBACTION_NO_MEMORIC			0x0040
//Use memorics, default 1
extern int g_memoric;

/*
Language specific options
*/
/*
Assembly language specific
*/
//Use AT&T style assembly if not set, otherwise Intel
#define UV_DISASM_SUBACTION_ASM_STYLE			0x0100

/*
buffer: instruction buffer
buffer_size: length of instruction buffer
offset: current buffer offset, updated to next position to be parsed.  When done, should be equal to buffer size
flags: what to do with the buffer
in: additional input parameters
ret: returned data structure, type specified by action flag

This combines all of the funcs together to save on spreading out code for
individual instructions
*/

/*
Used for keeping track of position when outputting files
Position is suppose to be the offset in a file
positionIndex is for when multiple details are on that line
Could keep internal or external cache of what is to come on that line
and generate all indexes at once
*/
class UVD;
class UVDIterator
{
public:
	//UVDIterator();
	UVDIterator(UVD *disassembler = NULL, unsigned int position = g_addr_min, unsigned int index = 0);
	uv_err_t init(UVD *disassembler, unsigned int position = g_addr_min, unsigned int index = 0);

	unsigned int getPosition();

	uv_err_t parseOperands(UVDInstruction *instruction);
	UVDIterator operator++();
	uv_err_t next();
	uv_err_t nextCore();
	uv_err_t nextInstruction(UVDInstruction &instructionIn);
	bool operator==(const UVDIterator &other) const;
	bool operator!=(const UVDIterator &other) const;
	std::string operator*();
	void debugPrint() const;

	//Print a (tabbed) list of memory locations for current address based on type given
	uv_err_t printReferenceList(UVDAnalyzedMemoryLocation *memLoc, uint32_t type);

protected:
	//Force printing of header information
	uv_err_t initialPrint();

public:
	//Source of data to disassemble
	//Usually equal to m_uvd.m_data, but may be a smaller segment for finer analysis
	UVDData *m_data;

private:	
	//Object we are iterating on
	UVD *m_uvd;
	//Next position in file to check
	unsigned int m_nextPosition;
	//Next index to check on generated lines
	unsigned int m_positionIndex;
	//Lines to come from current index
	std::vector<std::string> m_indexBuffer;
	//Printed startup information yet?
	int m_printedInformation;
};


/*
UV Decompiler engine
Primary end user object
*/
class UVD
{
public:
	UVD();
	
	//Factory function for construction
	static uv_err_t getUVD(UVD **uvdIn, UVDData *data);

	/*
	file: data file to target
	architecture: hint about what we are trying to disassemble
	*/
	uv_err_t init(const std::string &file, int architecture = 0);
	uv_err_t init(UVDData *data, int architecture = 0);
	//Initialize the opcode tables
	uv_err_t init_config(const std::string &confFile);
	//uv_err_t opcodeDeinit();
	uv_err_t init_misc(UVDConfigSection *misc_section);
	uv_err_t init_memory(UVDConfigSection *mem_section);
	uv_err_t init_reg(UVDConfigSection *reg_section);
	uv_err_t init_prefix(UVDConfigSection *pre_section);
	

	UVDIterator begin();
	UVDIterator begin(UVDData *data);
	UVDIterator end();
	UVDIterator end(UVDData *data);

	uv_err_t disassemble(std::string file, std::string &output);
	uv_err_t decompile(std::string file, int destinationLanguage, std::string &output);
	uv_err_t decompilePrint(std::string &output);
	
	std::string analyzedSymbolName(std::string dataSource, uint32_t functionAddress);
	
	//Given a function location, do analysis on the section to do actual decompiling
	uv_err_t analyzeNewFunction(const UVDAnalyzedMemoryLocation *memLoc, UVDAnalyzedFunction &analyzedFunction);
	//uv_err_t analyzeFunction(UVDAnalyzedFunction &analyzedFunction);
	//Since not all code is in a function, base implementation is on raw code
	//uv_err_t analyzeCode(UVDAnalyzedMemoryLocation memLoc, UVDAnalyzedCode &analyzedCode);
	//Core code analysis function
	//Structure should be pre-set with data before entry
	uv_err_t analyzeCode(UVDAnalyzedCodeShared &UVDAnalyzedCodeShared);
	uv_err_t analyzeConstData();
	uv_err_t analyzeStrings();
	uv_err_t constructBlock(unsigned int minAddr, unsigned int maxAddr, UVDAnalyzedBlock **blockOut);
	uv_err_t constructBlocks();
	uv_err_t constructFunctionBlocks(UVDAnalyzedBlock *superblock);
	uv_err_t constructJumpBlocks(UVDAnalyzedBlock *superblock, UVDAnalyzedMemoryLocations &superblockLocations, UVDAnalyzedMemoryLocations::iterator &iterSuperblock);
	uv_err_t analyzeControlFlow();
	//Analyze control structures: if, else, etc
	uv_err_t analyzeBlock(UVDAnalyzedBlock *block);
	//Create output suitible for building analysis database
	uv_err_t generateAnalysisDir();
	uv_err_t analyze();
	
	//Convert a block (should be UVDDataChunk?) suspected to be a function to a skeleton analyzed function structure
	uv_err_t blockToFunction(UVDAnalyzedBlock *functionBlock, UVDBinaryFunction **functionIn);
	
	uv_err_t changeConfig(UVDConfig *config);
	
	/*
	Convert an instruction to a disassembled string list using the policy of this disassembler object
	Since ideally UVDInustruction knows nothing about this engine, so better here than a toString'ish method there
	*/
	uv_err_t stringListAppend(UVDInstruction *inst, std::vector<std::string> &list);


	uv_err_t analyzeFunction(UVDBinaryFunctionShared *functionShared);
	uv_err_t analyzeFunctionRelocatables(UVDBinaryFunctionInstance *binaryFunctionCodeShared);

public:
	//Source of data to disassemble
	UVDData *m_data;
	//Lookup table for opcodes
	//This in theory could be shared between multiple engines for the same arch
	//But not much of an issue since only one instance is expected per run
	UVDOpcodeLookupTable *m_opcodeTable;
	//For special modifiers mostly for now (functions)
	//Allows special mapping of addresses and others
	UVDSymbolMap *m_symMap;
	
	//Used for advanced analysis
	UVDConfigExpressionInterpreter *m_interpreter;
		
	/*
	Architecture hint
	Necessary for raw binary images
	If an ELF file or similar that contains architecture info in it is given, 
	it may be possible to determine this information dynamically
	*/
	int m_architecture;

	UVDAnalyzer *m_analyzer;
	
	//ROM data or other things that make these addresses non-disassemblable
	//This should probably be moved to the analyzer
	std::vector<UVDMemoryLocation> m_noncodingAddresses;
	//Registers, mapped by name
	std::map<std::string, UVDRegisterShared *> m_registers;

	//General configuration
	UVDConfig *m_config;
	//How to print data
	UVDFormat *m_format;
};

#ifdef __cplusplus
extern "C"
{
#endif /* ifdef __cplusplus */

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif // ifdef UV_DISASM_H

