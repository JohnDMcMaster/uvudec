/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#include "config.h"
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
//What we need...IS SOME ROPE!
#if USING_ROPE
#include <ext/rope>
#endif //USING_ROPE
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>
#include <sys/stat.h>
#include <vector>
#include <algorithm>
#include "uvd_debug.h"
#include "uvd_error.h"
#include "uvd_log.h"
#include "uvd_util.h"
#include "uvd.h"
#include "uvd_address.h"
#include "uvd_analysis.h"
#include "uvd_benchmark.h"
#include "uvd_config_symbol.h"
#include "uvd_data.h"
#include "uvd_format.h"
#include "uvd_instruction.h"
#include "uvd_language.h"
#include "uvd_log.h"
#include "uvd_types.h"

UVD *g_uvd = NULL;

/*
TODO: there was a paradigm shift between how to analyze functions
Moving towards DB based storage
Each of these accomplish essentially same thing, but different way
*/

//Called when function first discovered
uv_err_t UVD::analyzeNewFunction(const UVDAnalyzedMemoryLocation *memLoc, UVDAnalyzedFunction &analyzedFunction)
{
	uv_err_t rc = UV_ERR_GENERAL;
	UVDAnalyzedCode *analyzedCode = NULL;
	UVDDataChunk *dataChunk = NULL;
	
	uv_assert(memLoc);
	
	//analyzedFunction = new UVDAnalyzedFunction();
	
	analyzedCode = new UVDAnalyzedCode();
	analyzedFunction.m_code = analyzedCode;
	
	dataChunk = new UVDDataChunk();
	if( UV_FAILED(dataChunk->init(m_data, memLoc->m_min_addr, memLoc->m_max_addr)) )
	{
		UV_DEBUG(rc);
		goto error;
	}
	analyzedCode->m_dataChunk = dataChunk;
	
	//Ready, perform analysis
	if( UV_FAILED(analyzeCode(*analyzedCode)) )
	{
		UV_DEBUG(rc);
		goto error;
	}
	
	/*
	Block analysis is not yet done yet, which would make this hard/duplicate work
	//Additional analysis: start from end and look for code that doesn't either exit func or continue it somehow
	if( g_filterPostRet )
	{
	}
	*/

	rc = UV_ERR_OK;

error:
	return UV_DEBUG(rc);
}

static std::string mangleFileToSymbol(const std::string &sIn)
{
	/*
	There are a lot of cases not covered for now
	*/

	//Start by getting the basename
	//basename may modify string, make a copy
	char buff[512];
	strcpy(buff, sIn.c_str());
	char *szBasename = basename(buff);
	if( !szBasename )
	{
		return "";
	}
	std::string sBasename = szBasename;
	
	//remove everything after . (assume extension)
	for( ;; )
	{
		std::string::size_type pos = sBasename.rfind('.');
		if( pos == std::string::npos )
		{
			break;
		}
		//Get substring
		sBasename = sBasename.substr(0, pos);
	}
	
	//All other stuff replace with _
	
	return sBasename;
}

std::string UVD::analyzedSymbolName(uint32_t symbolAddress, int symbolType)
{
	if( m_data )
	{
		std::string dataSource = uv_basename(m_data->getSource());
		return analyzedSymbolName(dataSource, symbolAddress, symbolType);
	}
	return "";
}

/*
For generating unknown symbol stuff
*/
static const char *getSymbolTypeNamePrefix(int symbolType)
{
	switch( symbolType )
	{
	case UVD__SYMBOL_TYPE__UNKNOWN:
		return "unknown";
	case UVD__SYMBOL_TYPE__FUNCTION:
		return "sub";
	case UVD__SYMBOL_TYPE__LABEL:
		return "lab";
	case UVD__SYMBOL_TYPE__ROM:
		return "const";
	case UVD__SYMBOL_TYPE__VARIABLE:
		return "var";
	default:
		return "error";
	}
}

std::string UVD::analyzedSymbolName(std::string dataSource, uint32_t symbolAddress, int symbolType)
{
	/*
	Might be nice to add on something about these being unknown symbol rather than known
	Ex:
	uvd_unknown__candela_rev_3__3242
	*/
	char buff[512];
	const char *typePrefix = getSymbolTypeNamePrefix(symbolType);
	//make this optional?
	const char *uvudecPrefix = "uvudec__";

	//file + address
	//Do mangling to make sure we don't have dots and such
	std::string mangeledDataSource = mangleFileToSymbol(dataSource);
	snprintf(buff, 512, "%s%s__%s_%.4X", uvudecPrefix, mangeledDataSource.c_str(), typePrefix, symbolAddress);
	return std::string(buff);
}

//Second pass used to create for DB storage
//Block contains analyzed code
uv_err_t UVD::blockToFunction(UVDAnalyzedBlock *functionBlock, UVDBinaryFunction **functionIn)
{
	UVDBinaryFunction *function = NULL;
	UVDBinaryFunctionShared *functionShared = NULL;
	//Since ownership will be xferred to object, we need to map it twice
	UVDDataChunk *functionBlockDataChunk = NULL;
	//UVDDataChunk *functionDataChunk = NULL;
	//UVDDataChunk *functionInstanceDataChunk = NULL;
	UVDBinaryFunctionInstance *functionInstance = NULL;

	uv_assert_ret(functionBlock);
	uv_assert_ret(functionIn);
	
	uv_assert_err_ret(functionBlock->getDataChunk(&functionBlockDataChunk));
	uv_assert_ret(functionBlockDataChunk)
	//uv_assert_err_ret(functionBlock->getDataChunk(&functionDataChunk));
	//uv_assert_err_ret(functionBlock->getDataChunk(&functionInstanceDataChunk));
	//uv_assert_ret(functionDataChunk);
	//uv_assert_ret(functionInstanceDataChunk);
	//uv_assert_ret(functionDataChunk->m_data);
	//uv_assert_ret(functionInstanceDataChunk->m_data);

	function = new UVDBinaryFunction();
	uv_assert_ret(function);
	//uv_assert_err_ret(function->init());

	functionShared = new UVDBinaryFunctionShared();
	printf_debug("new UVDBinaryFunctionShared: 0x%.8X\n", (unsigned int)function);	
	uv_assert_ret(functionShared);

	uint32_t minAddress = 0;
	uv_assert_err_ret(functionBlock->getMinAddress(minAddress));
	function->m_offset = minAddress;
	
	//The true name of the function is unknown
	//This name is stored as a way to figure out the real name vs the current symbol name 
	functionShared->m_name = "";
	functionShared->m_description = "Automatically generated.";	

	//Create the single known instance of this function
	uv_assert_err_ret(UVDBinaryFunctionInstance::getUVDBinaryFunctionInstance(&functionInstance));
	uv_assert_ret(functionInstance);
	functionInstance->m_symbolAddress = UVDRelocatableElement(minAddress);
	//Only specific instances get symbol designations
	functionInstance->setSymbolName(analyzedSymbolName(minAddress));
	
	/*
	Skip for now:
	UVDCompiler *m_compiler;
	UVDSupportedVersions m_versions;
	UVDCompilerOptions *m_compilerOptions;
	int m_language;
	std::string m_code;
	std::string m_origin;
	std::string m_notes;
	*/
	//This will perform copy
	uv_assert_err_ret(functionInstance->setData(functionBlockDataChunk));
	//And register it to that particular function
	functionShared->m_representations.push_back(functionInstance);
	
	function->m_shared = functionShared;
	function->m_uvd = this;
	uv_assert_err_ret(functionBlockDataChunk->deepCopy(&function->m_data));
	
	*functionIn = function;

	return UV_ERR_OK;
}

uv_err_t UVD::analyzeCode(UVDAnalyzedCode &analyzedCode)
{
	return UV_ERR_OK;
}

uv_err_t UVD::analyzeStrings()
{
	/*
	Do strings(3) like processing to find ROM/string data
	For some formats like ELF, it is possible to get this string table more directly
	*/

	uv_err_t rc = UV_ERR_GENERAL;
/*
	//An individual ASCII chunk, does not account terminating nulls
	unsigned int minStringSize = 3;
	//How far apart clusters should be to collect them
	unsigned int maxClusterSpacing = 3;
	UVDAnalyzedMemoryLocations startCluster;
	*/
	
	UV_ENTER();

	printf_debug_level(UVD_DEBUG_PASSES, "uvd: analyzing strings...\n");
	UVDBenchmark stringAnalysisBenchmark;
	stringAnalysisBenchmark.start();

	/*
	//For now just detect locations
	//Eventually would like to get hit count

	//Look for string data, it should have a high ASCII hit count
	//Then cluster data for total count
	for( unsigned int i = 0; i < m_data->size(); )
	{
		for( unsigned int j = 0; j < m_data->size(); )
		{
			int readRaw = 0;
			char c = 0;
			
			readRaw = m_data->read(j);
			++j;
			
			if( readRaw < 0 )
			{
				UV_DEBUG(rc);
				goto error;
			}
			c = (char)readRaw;
			if( !isprint(c) )
			{
				break;
			}
		}
		
		unsigned int endAddr = j - 1;
		//Goes one beyond locations
		if( endAddr - i >= minClusterSize )
		{
			startCluster.push_back(UVDAnalyzedMemoryLocation(i, endAddr));
		}
		
		i = j;
	}
	*/
	
	//For now assume non-coding addresses are ROM data since thats primary motiviation to exclude them
	for( unsigned int i = 0; i < m_noncodingAddresses.size(); ++i )
	{
		UVDMemoryLocation memLoc = m_noncodingAddresses[i];
		//Do a C string table analysis
		for( unsigned int i = memLoc.m_min_addr; i < memLoc.m_max_addr; )
		{
			unsigned int j = 0;
			for( j = i; j <= memLoc.m_max_addr; )
			{
				int readRaw = 0; 
				char c = 0;
				
				readRaw = m_data->read(j);
				++j;
				
				if( readRaw < 0 )
				{
					UV_DEBUG(rc);
					goto error;
				}
				c = (char)readRaw;
				//null terminator
				if( c == 0 )
				{
					//Add the string
					unsigned int endAddr = j - 1;

					//Just insert one reference to each string for now
					//A map
					m_analyzer->m_stringAddresses[i] = new UVDAnalyzedMemoryLocation(i, endAddr);
					m_analyzer->m_stringAddresses[i]->insertReference(i, UVD_MEMORY_REFERENCE_CONSTANT | UVD_MEMORY_REFERENCE_STRING);
					break;
				}
			}
			
			//Start is where we left off			
			i = j;
		}
	}
	
	stringAnalysisBenchmark.stop();
	printf_debug_level(UVD_DEBUG_PASSES, "string analysis time: %s\n", stringAnalysisBenchmark.toString().c_str());
	

	rc = UV_ERR_OK;
	
error:
	return UV_DEBUG(rc);
}

uv_err_t UVD::constructBlock(unsigned int minAddr, unsigned int maxAddr, UVDAnalyzedBlock **blockOut)
{
	UVDAnalyzedBlock *block = NULL;
	UVDAnalyzedCode *analyzedCode = NULL;
	UVDDataChunk *dataChunk = NULL;
	//uint32_t dataSize = 0;
	
	uv_assert_ret(m_config);
	uv_assert_ret(blockOut);
	
	printf_debug("Constructing block 0x%.8X:0x%.8X\n", minAddr, maxAddr);
	uv_assert_ret(m_data);

	uv_assert_ret(minAddr <= maxAddr);
	//uv_assert_ret(maxAddr <= m_config->m_addr_max);
	//dataSize = maxAddr - minAddr;

	block = new UVDAnalyzedBlock();
	uv_assert_ret(block);
	
	analyzedCode = new UVDAnalyzedCode();
	uv_assert_ret(analyzedCode);
	block->m_code = analyzedCode;

	dataChunk = new UVDDataChunk();
	uv_assert_ret(dataChunk);
	uv_assert_ret(m_data);
	uv_assert_err_ret(dataChunk->init(m_data, minAddr, maxAddr));
	uv_assert_ret(dataChunk->m_data);
	analyzedCode->m_dataChunk = dataChunk;

	*blockOut = block;

	return UV_ERR_OK;
}

/*
static int UVDAnalyzedMemorySpaceSorter(UVDAnalyzedMemoryLocation *l, UVDAnalyzedMemoryLocation *r)
{
	if( l == r )
	{
		return 0;
	}
	if( l == NULL )
	{
		return -1;
	}
	if( r == NULL )
	{
		return 1;
	}
	
	//printf("Comparing: 0x.%.8X to 0x%.8X\n", r->m_min_addr, l-> m_min_addr);
	
	//Both different and non-null: standard comapre
	return r->m_min_addr - l-> m_min_addr;
}
*/

uv_err_t memorySpaceToMemoryLocations(const UVDAnalyzedMemorySpace &space, UVDAnalyzedMemoryLocations& ret)
{
	uv_err_t rc = UV_ERR_GENERAL;

	ret.clear();
	
	//Some way to make insertion copy, but can't remember off of top of head
	//std::copy(ret.begin(), space.begin(), space.end());
	for( UVDAnalyzedMemorySpace::const_iterator iter = space.begin(); iter != space.end(); ++iter )
	{
		UVDAnalyzedMemoryLocation *memoryLocation = (*iter).second;
		uv_assert(memoryLocation);
		ret.push_back(memoryLocation);
	}
	//Why does this seg fault?
	//std::sort(ret.begin(), ret.end(), UVDAnalyzedMemorySpaceSorter);
	//printf("Sorted\n");
	//fflush(stdout);

	rc = UV_ERR_OK;
	
error:
	return UV_DEBUG(rc);
}

uv_err_t UVD::constructJumpBlocks(UVDAnalyzedBlock *superblock, UVDAnalyzedMemoryLocations &superblockLocations, UVDAnalyzedMemoryLocations::iterator &iterSuperblock)
{
	//Commented out because its not being used?  What about labeling?
	return UV_ERR_OK;
/*
	uv_err_t rc = UV_ERR_GENERAL;
	uint32_t lastSuperblockLocation = 0;
	uint32_t superblockMinAddress = 0;
	uint32_t superblockMaxAddress = 0;
	UVDAnalyzedBlock *jumpedBlock = NULL;

	printf_debug("\n");
	UV_ENTER();

	uv_assert(superblock);

	uv_assert_err(superblock->getMinAddress(superblockMinAddress));
	uv_assert_err(superblock->getMaxAddress(superblockMaxAddress));
	
	//Skip ahead to where the superblock is
	while( iterSuperblock != superblockLocations.end() )
	{
		UVDAnalyzedMemoryLocation *memoryCalled = *iterSuperblock;
		uv_assert(memoryCalled);

		//Do we have at least func start address?
		if( memoryCalled->m_min_addr >= superblockMinAddress )
		{
			//If so, we are done seeking
			break;
		}
		++iterSuperblock;
	}
	//Loop for all called locations in this block
	for( ; iterSuperblock != superblockLocations.end(); ++iterSuperblock )
	{
		UVDAnalyzedMemoryLocation *memoryJumped = *iterSuperblock;		
		uint32_t jumpedBlockEnd = 0;
		
		uv_assert(memoryJumped);
		
		//Are we past superblock?
		if( memoryJumped->m_min_addr > superblockMaxAddress )
		{
			break;
		}
		
		//Nothing precedes superblock start, skip block
		if( memoryJumped->m_min_addr == superblockMinAddress )
		{
			continue;
		}

		//Add the block
		jumpedBlockEnd = memoryJumped->m_min_addr - 1;
		uv_assert_err(constructBlock(lastSuperblockLocation, jumpedBlockEnd, &jumpedBlock));
		uv_assert(jumpedBlock);
		superblock->m_blocks.push_back(jumpedBlock);
		lastSuperblockLocation = memoryJumped->m_min_addr;
	}
	
	//Finally, end of function block
	//We want location after last block
	uv_assert_err(constructBlock(lastSuperblockLocation, superblockMaxAddress, &jumpedBlock));
	uv_assert(jumpedBlock);
	superblock->m_blocks.push_back(jumpedBlock);
	
	rc = UV_ERR_OK;
	
error:
	return UV_DEBUG(rc);
*/
}


uv_err_t UVDInit()
{
	//Initially we log to console until a "real" log is setup which may be an actual file
	//we don't know actual file because we haven't parsed args yet
	uv_assert_err_ret(uv_log_init("/dev/stdout"));
	uv_assert_err_ret(UVDDebugInit());
	uv_assert_err_ret(UVDInitConfig());
	printf_debug_level(UVD_DEBUG_PASSES, "UVDInit(): done\n");
	return UV_ERR_OK;
}

uv_err_t UVDDeinit()
{
	if( g_uvd )
	{
		delete g_uvd;
		g_uvd = NULL;
	}

	//This won't get deleted by prev if it was global instance
	if( g_config )
	{
		delete g_config;
		g_config = NULL;
	}

	uv_assert_err_ret(UVDDebugDeinit());
	uv_assert_err_ret(uv_log_deinit());

	return UV_ERR_OK;
}

UVD::UVD()
{
	m_data = NULL;
	m_opcodeTable = NULL;
	m_architecture = 0;
	m_interpreter = NULL;
	m_analyzer = NULL;
	m_format = NULL;
	//m_CPU = NULL;
	m_config = NULL;
	m_symMap = NULL;
}

UVD::~UVD()
{
	deinit();
}

uv_err_t UVD::deinit()
{
	//m_data deallocated by UVD engine caller
	
	delete m_opcodeTable;
	m_opcodeTable = NULL;

	delete m_symMap;
	m_symMap = NULL;

	delete m_interpreter;
	m_interpreter = NULL;

	delete m_analyzer;
	m_analyzer = NULL;

	delete m_format;
	m_format = NULL;

	for( std::map<std::string, UVDRegisterShared *>::iterator iter = m_registers.begin(); iter != m_registers.end(); ++iter )
	{
		UVDRegisterShared *regShared = (*iter).second;

		if( !regShared )
		{
			printf_warn("bad regShared entry\n");
		}
		else
		{
			delete regShared;
		}
	}
	m_registers.clear();
	
	if( m_config != g_config )
	{
		delete m_config;
		m_config = NULL;
	}
	
	return UV_ERR_OK;
}

//Factory function for construction
uv_err_t UVD::getUVD(UVD **uvdIn, UVDData *data)
{
	UVD *uvd = NULL;
	
	if( g_uvd )
	{
		uvd = g_uvd;	
	}
	else
	{
		uvd = new UVD();
		if( !uvd )
		{
			return UV_DEBUG(UV_ERR_GENERAL);
		}
		
		uv_assert_ret(g_config);
		uvd->m_config = g_config;
		
		if( UV_FAILED(uvd->init(data)) )
		{
			delete uvd;
			return UV_DEBUG(UV_ERR_GENERAL);
		}
		
		g_uvd = uvd;
	}

	uv_assert_ret(uvdIn);
	*uvdIn = uvd;
	return UV_ERR_OK;
}

/*
uv_err_t UVD::opcodeDeinit()
{
	return m_opcodeTable->deinit();
}
*/

UVDIterator UVD::begin()
{
	UVDIterator iter;
	
	UV_DEBUG(iter.init(this));
	iter.m_data = m_data;
	return iter;
}

UVDIterator UVD::begin(uint32_t offset)
{
	UVDIterator iter;
	
	UV_DEBUG(iter.init(this, offset, 0));
	iter.m_data = m_data;
	return iter;
}

UVDIterator UVD::begin(UVDData *data)
{
	UVDIterator iter = begin();
	iter.m_data = data;
	return iter;
}

UVDIterator UVD::end()
{
	//Pos is "next position"
	//Size is first invalid position
	UVDIterator iter;
	
	//This will work fine unless we fill up the entire address space
	UV_DEBUG(iter.init(this));
	iter.m_data = m_data;
	//The key part
	UV_DEBUG(iter.makeEnd());
	return iter;
}

UVDIterator UVD::end(UVDData *data)
{
	UVDIterator iter = end();
	iter.m_data = data;
	return iter;
}

//FIXME: doesn't split
std::vector<std::string> split(const std::string &s, char delim)
{
	std::vector<std::string> ret;
	ret.push_back(s);
	return ret;
}

uv_err_t UVD::stringListAppend(UVDInstruction *inst, std::vector<std::string> &list)
{
	uv_err_t rc = UV_ERR_GENERAL;
	char buff[512];
	
	if( !inst )
	{
		return UV_DEBUG(rc);
	}
	
	printf_debug("printing disasm\n");
	rc = inst->print_disasm(buff, 512);
	printf_debug("printed disasm\n");
	if( UV_SUCCEEDED(rc) )
	{
		std::vector<std::string> toAppend;

		toAppend = split(std::string(buff), '\n', true);
		for( std::vector<std::string>::size_type i = 0; i < toAppend.size(); ++i )
		{
			list.insert(list.end(), toAppend[i]);
		}
	}
	
	return UV_DEBUG(rc);
}

uv_err_t UVD::createAnalysisDir(const std::string &file, const std::string &outputDir)
{
	std::string oldDir;
	
	oldDir = m_config->m_analysisDir;
	m_config->m_analysisDir = outputDir;
	uv_assert_err_ret(createAnalysisDir());
	m_config->m_analysisDir = oldDir;

	return UV_ERR_OK;
}

uv_err_t UVD::createAnalysisDir()
{
	uint32_t oldAnalysisOnly = 0;
	//Due to decompile hack
	std::string discard;
	
	uv_assert_ret(m_config);
	oldAnalysisOnly = m_config->m_analysisOnly;
	m_config->m_analysisOnly = 1;
	//Sorta hackish, do some cleanup later
	//Lang is relativly unimportant, assembly indicates don't do any high level analysis
	//(although it is ignored for now anyway)
	uv_assert_err_ret(decompile(UVD_LANGUAGE_ASSEMBLY, discard));
	m_config->m_analysisOnly = oldAnalysisOnly;
	
	return UV_ERR_OK;
}

uv_err_t UVD::disassemble(std::string &output)
{
	return UV_DEBUG(decompile(UVD_LANGUAGE_ASSEMBLY, output));
}

uv_err_t UVD::decompile(int destinationLanguage, std::string &output)
{
	//uint8_t *dat = NULL;	
	unsigned int dat_sz = 0;
	//unsigned int read = 0;
	std::string configFile;
	
	UV_ENTER();

	UVDBenchmark decompileBenchmark;
	decompileBenchmark.start();
	
	uv_assert_ret(m_data);
	dat_sz = m_data->size();
	
	//Trivial case: nothing to analyze
	if( dat_sz == 0 )
	{
		return UV_ERR_OK;
	}
	
	uv_assert_ret(m_config);
	
	//If unspecified, default to full range
	/*
	if( m_config->m_addr_max == 0 )
	{
		m_config->m_addr_max = dat_sz - 1;
	}
	*/
	printf_debug("Raw data size: 0x%x (%d)\n", dat_sz, dat_sz);

	//Most of program time should be spent here
	uv_assert_err_ret(analyze());

	uv_assert_ret(m_config);
	if( !m_config->m_analysisOnly )
	{
		uv_assert_err_ret(decompilePrint(output));
	}

	printf_debug_level(UVD_DEBUG_PASSES, "decompile: done\n");
	decompileBenchmark.stop();
	printf_debug_level(UVD_DEBUG_PASSES, "decompile time: %s\n", decompileBenchmark.toString().c_str());
	
//printf("DEBUG BREAK\n");
//exit(1);

	return UV_ERR_OK;
}

uv_err_t UVD::decompilePrint(std::string &output)
{
	UVDIterator iter;
	//UVDIterator iterEnd;
	int printPercentage = 1;
	int printNext = printPercentage;
	uint32_t analyzedBytes = 0;

	uv_assert_ret(m_config);
	uv_assert_err_ret(m_analyzer->getNumberAnalyzedBytes(&analyzedBytes));
	uv_assert_ret(analyzedBytes != 0);

	printf_debug_level(UVD_DEBUG_PASSES, "decompile: printing...\n");
	UVDBenchmark decompilePrintBenchmark;
	decompilePrintBenchmark.start();
	iter = begin();

	//Due to the huge number of concatenations
#ifdef USING_ROPE
	__gnu_cxx::crope outputRope;
#else
	output.clear();
#endif //USING_ROPE
	int iterations = 0;
	while( iter != end() )
	{
		std::string line;
		uint32_t startPos = iter.getPosition();

		++iterations;				
		printf_debug("\n\n\n");
		printf_debug("Iteration loop iteration\n");		

		int curPercent = 100 * startPos / analyzedBytes;
		if( curPercent >= printNext )
		{
			uint64_t delta = getTimingMicroseconds() - decompilePrintBenchmark.getStart();
			double iterationTime = 1.0 * delta / iterations;
			printf_debug_level(UVD_DEBUG_SUMMARY, "uvd: printing: %d %% (us / record: %.2lf)\n", curPercent, iterationTime);
			printNext += printPercentage;
		}

		line = *iter;
		printf_debug("Line (0x%.8X): %s\n", iter.getPosition(), line.c_str());

		//This didn't help for the bottleneck under investigation
#if USING_ROPE
		outputRope += (
#else
		output +=
#endif //USING_ROPE
				line + "\n"
#if USING_ROPE
				).c_str()
#endif //USING_ROPE
				;				
		printf_debug("\n");
		if( UV_FAILED(iter.next()) )
		{
			printf_debug("Failed to get next\n");
			return UV_DEBUG(UV_ERR_GENERAL);
		}
		iter.debugPrint();
	}
#ifdef USING_ROPE
	output = outputRope.c_str();
	outputRope.clear();
#endif //USING_ROPE
	
	if( m_config->m_print_used )
	{
		m_opcodeTable->usedStats();
	}

	decompilePrintBenchmark.stop();
	printf_debug_level(UVD_DEBUG_PASSES, "decompile print time (%d records): %s\n", iterations, decompilePrintBenchmark.toString().c_str());

	return UV_ERR_OK;
}

/*
uv_err_t UVD::changeConfig(UVDConfig *config)
{
	if( m_config )
	{
		delete m_config;
	}
	m_config = config;
	return UV_ERR_OK;
}
*/

UVDData *UVD::getData()
{
	return m_data;
}
