/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_H
#define UVD_H

#include <stdint.h>
#include <map>
#include <list>
#include <set>
#include <string>
#include <vector>
#include "uvd/assembly/cpu.h"
#include "uvd/util/error.h"
#include "uvd/core/analyzer.h"
#include "uvd/data/data.h"
#include "uvd/language/format.h"
#include "uvd/assembly/instruction.h"
#include "uvd/assembly/register.h"
#include "uvd/util/types.h"
#include "uvd/architecture/architecture.h"

//From linux/stringify.h
#define __stringify_1(x)	#x
#define __stringify(x)		__stringify_1(x)

#define UVUDEC_VER_STRING 		__stringify(UVUDEC_VER_MAJOR) "." __stringify(UVUDEC_VER_MINOR) "." __stringify(UVUDEC_VER_PATCH)

#if 0
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
#endif

/*
Options
*/

#if 0
uv_err_t uvd_init_equiv_mem(void);

/* Global symbol management */
uv_err_t uvd_set_sym(const std::string key, 
		struct uvd_sym_t *value, struct uvd_sym_t **old_value);
uv_err_t uvd_get_sym(const std::string key, struct uvd_sym_t **value);
#endif

#if 0
//nasm style. Uses % in front of registers.  mov %src_reg, %dest_reg
#define UV_DISASM_STYLE_ATT						1
//masm style?.   MOV DEST_REG, SRC_REG
#define UV_DISASM_STYLE_INTEL					2
//is MIPS its own style?  Seems MIPS asm is standardized
//extern int g_asm_style;
//Print how instructions are derrived?
//extern int g_deriv;
#endif

#if 0
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
//extern int g_memoric;
#endif

/*
Language specific options
*/
/*
Assembly language specific
*/
//Use AT&T style assembly if not set, otherwise Intel
//#define UV_DISASM_SUBACTION_ASM_STYLE			0x0100

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

#include "uvd/core/iterator.h"

/*
UV Decompiler engine
Primary end user object
*/
class UVDEventEngine;
class UVDArchitecture;
class UVDObject;
class UVDRuntime;
class UVD
{
public:
	UVD();
	~UVD();
	
	//Factory function for construction
	static uv_err_t getUVDFromFileName(UVD **out, const std::string &file);
	static uv_err_t getUVDFromData(UVD **out, UVDData *data);
	//Without a specific assembly implementation loaded
	//static uv_err_t getUVD(UVD **uvdIn);

	/*
	file: data file to target
	object: hint about the file format
	architecture: hint about what we are trying to disassemble
	*/
	uv_err_t initFromFileName(const std::string &file, const UVDRuntimeHints &hints = UVDRuntimeHints());
	uv_err_t initFromData(UVDData *data, const UVDRuntimeHints &hints = UVDRuntimeHints());
	uv_err_t initEarly();
	//uv_err_t initPlugins();
	uv_err_t initObject(UVDData *data, const UVDRuntimeHints &hints, UVDObject **out);
	uv_err_t initArchitecture(UVDObject *object, const UVDRuntimeHints &hints, UVDArchitecture **out);
	//Core version with a pre-supplied object and architecture
	uv_err_t init(UVDObject *object, UVDArchitecture *architecture);
	uv_err_t deinit();

	/*
	Iterator functions
	*/
	//Printing related
	//These try to guess the primary executable address space
	UVDPrintIterator begin();
	uv_err_t begin(UVDPrintIterator &iter);
	
	UVDPrintIterator begin(uv_addr_t offset);
	uv_err_t begin(UVDAddress address, UVDPrintIterator &iter);
	//UVDPrintIterator begin(UVDData *data);
	UVDPrintIterator end();
	uv_err_t end(UVDPrintIterator &iter);
	//UVDPrintIterator end(UVDData *data);
	//Analysis related
	UVDInstructionIterator instructionBegin();
	uv_err_t instructionBegin(UVDInstructionIterator &iter);
	uv_err_t instructionBeginByAddress(UVDAddress address, UVDInstructionIterator &iter);
	UVDInstructionIterator instructionEnd();
	uv_err_t instructionEnd(UVDInstructionIterator &iter);

	/*
	High level generation functions
	*/
	/*
	Object files and such
	for now, outputDir is ignored and controlled through config
	*/
	//uv_err_t createAnalysisDir(const std::string &file, const std::string &outputDir);
	//Use config options
	//uv_err_t createAnalysisDir();
	/*
	Disassemble binary file to output string
	*/
	uv_err_t disassemble(std::string &output);
	uv_err_t disassembleByCallback(uvd_string_callback_t callback, void *user);
	/*
	Given file, generate best representation possible in specified langauge to output file
	Using what created the engine init
	*/
	uv_err_t decompile(std::string &output);
	uv_err_t decompileByCallback(uvd_string_callback_t callback, void *user);
	//Intended for things like printing a function
	uv_err_t printRange(uv_addr_t start, uv_addr_t end, uint32_t destinationLanguage, std::string &output);
	//iterEnd is not inclusive
	uv_err_t printRangeCore(UVDPrintIterator iterBegin, UVDPrintIterator iterEnd, uvd_string_callback_t callback, void *user);
	//What we will try to output when printing
	//Used to format assembly output and such
	uv_err_t setDestinationLanguage(uint32_t destinationLanguage);
		
	//Given a function location, do analysis on the section to do actual decompiling
	//FIXME: this code is nearly dead and should probably be removed
	//uv_err_t analyzeNewFunction(const UVDAnalyzedMemoryRange *memLoc, UVDAnalyzedFunction &analyzedFunction);
	//uv_err_t analyzeFunction(UVDAnalyzedFunction &analyzedFunction);
	//Since not all code is in a function, base implementation is on raw code
	//uv_err_t analyzeCode(UVDAnalyzedMemoryRange memLoc, UVDAnalyzedCode &analyzedCode);
	//Core code analysis function
	//Structure should be pre-set with data before entry
	uv_err_t analyzeConstData();
	uv_err_t analyzeStrings();
	
	uv_err_t mapSymbols();
#if 0
	uv_err_t constructBlock(UVDAddressRange addressRange, UVDAnalyzedBlock **blockOut);
	uv_err_t constructBlocks();
	uv_err_t constructFunctionBlocks(UVDAnalyzedBlock *superblock);
	uv_err_t constructJumpBlocks(UVDAnalyzedBlock *superblock, UVDAnalyzedMemoryRanges &superblockLocations, UVDAnalyzedMemoryRanges::iterator &iterSuperblock);
	//Analyze control structures: if, else, etc
	uv_err_t analyzeBlock(UVDAnalyzedBlock *block);
#endif
	uv_err_t analyzeControlFlow();
		
	//Create output suitible for building analysis database
	uv_err_t analyze();
	
	//Convert a block (should be UVDDataChunk?) suspected to be a function to a skeleton analyzed function structure
	//uv_err_t blockToFunction(UVDAnalyzedBlock *functionBlock, UVDBinaryFunction **out);
	
	//uv_err_t changeConfig(UVDConfig *config);
	
	/*
	Convert an instruction to a disassembled string list using the policy of this disassembler object
	Since ideally UVDInustruction knows nothing about this engine, so better here than a toString'ish method there
	*/
	uv_err_t stringListAppend(UVDInstruction *inst, std::vector<std::string> &list);


	//uv_err_t analyzeFunction(UVDBinaryFunctionShared *functionShared);
	//uv_err_t analyzeFunctionRelocatables(UVDBinaryFunctionInstance *binaryFunctionCodeShared);

	//Change data to correspond to given file
	//uv_err_t setFile(const std::string &file);	
	//uv_err_t setData(UVData *data);
	//I think this was implemented to fix a bug of some sort...in truth is does nothing since it doesn't do standard error checking
	//UVDData *getData();
	/*
	In more complicated architectures, data may not be simply from 0 to end
	Preparations for later analysis allowing virtual addresses and such
	Each segment contains a peice of data at a certain virtual memory offset
	FIXME: this is a placeholder and just returns a simple version for now
	*/
	//uv_err_t getDataSegments(std::vector<UVDMemorySegment *> &segments);

	//transer ownership of formatting object to us
	//For outputting disassembly and decompiled formatting
	uv_err_t setOutputFormatting(UVDFormat *format);

protected:
	//Vector at start, take each instruction, one at a time, then compute branches/calls
	uv_err_t analyzeControlFlowLinear();
	//Start at all (valid) vectors and find all branch points
	uv_err_t analyzeControlFlowTrace();

	uv_err_t suspectValidInstruction(uint32_t address, int *isValid);

public:
	//Object format, architecture, debuggers, etc
	UVDRuntime *m_runtime;
	
	UVDAnalyzer *m_analyzer;
	
	//hmm should be moved to UVDAnalyzer
	//ROM data or other things that make these addresses non-disassemblable
	//This should probably be moved to the analyzer
	//actually these are currently never added to
	//std::vector<UVDAddressRange> m_noncodingAddresses;

	//General configuration
	//This must be a pointer as its intialized before we initialize our UVD object
	UVDConfig *m_config;
	//How to print data
	//This is active code as opposed to UVDConfig that just defines formatting parameters
	UVDFormat *m_format;

	//Library recognition engine
	//UVDFLIRT *m_flirt;
	
	//For notifying plugins and such of analysis events
	//We own this
	UVDEventEngine *m_eventEngine;
	
	//NOTE: plugin is part of config because it must have early init
	//we put it here for convenience, we do not own it
	UVDPluginEngine *m_pluginEngine;
};

#ifndef SWIG
//TODO: this is a hack, needs to be fixed
//Deprecated
extern UVD *g_uvd;
#endif
//For internal use only
//We may not always be singleton
UVD *UVDGetUVD();

#endif

