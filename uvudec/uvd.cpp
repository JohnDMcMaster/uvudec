/*
Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/


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
#include "uv_debug.h"
#include "uv_error.h"
#include "uv_log.h"
#include "uv_util.h"
#include "uvd.h"
#include "uvd_address.h"
#include "uvd_analysis.h"
#include "uvd_benchmark.h"
#include "uvd_data.h"
#include "uvd_format.h"
#include "uvd_instruction.h"
#include "uvd_language.h"
#include "uvd_types.h"
#include "uvd_symbol.h"

/*
TODO: there was a paradigm shift between how to analyze functions
Moving towards DB based storage
Each of these accomplish essentially same thing, but different way
*/

//Called when function first discovered
uv_err_t UVD::analyzeNewFunction(const UVDAnalyzedMemoryLocation *memLoc, UVDAnalyzedFunction &analyzedFunction)
{
	uv_err_t rc = UV_ERR_GENERAL;
	UVDAnalyzedFunctionShared *funcShared = NULL;
	UVDAnalyzedCodeShared *codeShared = NULL;
	UVDDataChunk *dataChunk = NULL;
	
	uv_assert(memLoc);
	
	funcShared = new UVDAnalyzedFunctionShared();
	analyzedFunction.m_shared = funcShared;
	
	codeShared = new UVDAnalyzedCodeShared();
	funcShared->m_code = codeShared;
	
	dataChunk = new UVDDataChunk();
	codeShared->m_dataChunk = dataChunk;
	if( UV_FAILED(dataChunk->init(m_data, memLoc->m_min_addr, memLoc->m_max_addr)) )
	{
		UV_DEBUG(rc);
		goto error;
	}
	
	//Ready, perform analysis
	if( UV_FAILED(analyzeCode(*codeShared)) )
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

//Second pass used to create for DB storage
//Block contains analyzed code
uv_err_t UVD::blockToFunction(UVDAnalyzedBlock *functionBlock, UVDBinaryFunction **functionIn)
{
	UVDBinaryFunction *function = NULL;
	UVDBinaryFunctionShared *functionShared = NULL;
	UVDDataChunk *dataChunk = NULL;
	UVDBinaryFunctionCodeShared *functionCodeShared = NULL;

	uv_assert_ret(functionBlock);
	uv_assert_ret(functionIn);
	
	/*
	dataChunk = new UVDDataChunk();
	uv_assert_ret(dataChunk);
	uv_assert_err_ret(dataChunk->init(m_data, functionBlock->getMinAddress(), functionBlock->getMaxAddress()));
	*/
	functionBlock->getDataChunk(&dataChunk);
	uv_assert_ret(dataChunk);

	function = new UVDBinaryFunction();
	uv_assert_ret(function);

	functionShared = new UVDBinaryFunctionShared();
	printf_debug("new UVDBinaryFunctionShared: 0x%.8X\n", (unsigned int)function);	
	uv_assert_ret(functionShared);

	char buff[512];
	uint32_t minAddress = 0;
	uv_assert_err_ret(functionBlock->getMinAddress(minAddress));
	std::string sourceBase = uv_basename(m_data->getSource());
	
	//file + address
	snprintf(buff, 512, "%s__%.4X", sourceBase.c_str(), minAddress);
	functionShared->m_name = buff;	
	functionShared->m_description = "Automatically generated.";	

	//Create the single known instance of this function
	functionCodeShared = new UVDBinaryFunctionCodeShared();
	uv_assert_ret(functionCodeShared);
	/*
	Skip for now:
	UVDCompiler *m_compiler;
	UVDSupportedVersions m_versions;
	UVDCompilerOptions *m_compilerOptions;
	int m_language;
	std::string m_code;
	std::string m_origin;
	std::string m_notes;functionCodeSharedfunctionCodeSharedfunctionCodeShared
	*/
	functionCodeShared->m_dataChunk = dataChunk;
	//And register it to that particular function
	functionShared->m_representations.push_back(functionCodeShared);
	
	function->m_shared = functionShared;
	function->m_uvd = this;
	function->m_dataChunk = dataChunk;
	
	*functionIn = function;

	return UV_ERR_OK;
}

uv_err_t UVD::analyzeCode(UVDAnalyzedCodeShared &UVDAnalyzedCodeShared)
{
	uv_err_t rc = UV_ERR_GENERAL;
	
	rc = UV_ERR_OK;
//error:
	return UV_DEBUG(rc);
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
	uv_err_t rc = UV_ERR_GENERAL;
	UVDAnalyzedBlock *block = NULL;
	UVDAnalyzedCode *analyzedCode = NULL;
	UVDAnalyzedCodeShared *analyzedCodeShared = NULL;
	UVDDataChunk *dataChunk = NULL;
	//uint32_t dataSize = 0;
	
	uv_assert(blockOut);
	
	printf_debug("Constructing block 0x%.8X:0x%.8X\n", minAddr, maxAddr);

	uv_assert(minAddr <= maxAddr);
	uv_assert(maxAddr <= g_addr_max);
	//dataSize = maxAddr - minAddr;

	block = new UVDAnalyzedBlock();
	uv_assert(block);
	
	analyzedCode = new UVDAnalyzedCode();
	uv_assert(analyzedCode);
	block->m_code = analyzedCode;

	analyzedCodeShared = new UVDAnalyzedCodeShared();
	uv_assert(analyzedCodeShared);
	analyzedCode->m_shared = analyzedCodeShared;

	dataChunk = new UVDDataChunk();
	uv_assert(dataChunk);
	analyzedCodeShared->m_dataChunk = dataChunk;

	uv_assert_err(dataChunk->init(m_data, minAddr, maxAddr));

	*blockOut = block;

	rc = UV_ERR_OK;
	
error:
	return UV_DEBUG(rc);
}

int UVDAnalyzedMemorySpaceSorter(UVDAnalyzedMemoryLocation *l, UVDAnalyzedMemoryLocation *r)
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

static uv_err_t nextReferencedAddress(UVDAnalyzedBlock *superblock, UVDAnalyzedMemoryLocations &space, UVDAnalyzedMemoryLocations::iterator &iter, uint32_t types, bool curAddressLegal = false);

static uv_err_t nextReferencedAddress(UVDAnalyzedBlock *superblock, UVDAnalyzedMemoryLocations &space, UVDAnalyzedMemoryLocations::iterator &iter, uint32_t types, bool curAddressLegal)
{
	uint32_t superblockMaxAddress = 0;

	uv_assert_ret(superblock);

	uv_assert_err_ret(superblock->getMaxAddress(superblockMaxAddress));
	
	
	if( iter != space.end() && !curAddressLegal )
	{
		++iter;
	}

	for( ; iter != space.end(); ++iter )
	{
		UVDAnalyzedMemoryLocation *referencedMemory = *iter;

		uv_assert_ret(referencedMemory);

		printf_debug("nextReferencedAddress() item @ 0x%.8X\n", referencedMemory->m_min_addr);

		//Nothing precedes program start, skip block (although this would be odd for all architectures I know of)
		if( referencedMemory->m_min_addr == 0 )
		{
			//Assume unknown location for now
			printf_debug("Min address zero (unknown memory location?)\n");
			continue;
		}

		//Stop once locations are too high
		if( referencedMemory->m_min_addr > superblockMaxAddress )
		{
			printf_debug("Limit reached: referencedMemory->m_min_addr (0x%.8X) > superblockMaxAddress (0x%.8X)\n", referencedMemory->m_min_addr, superblockMaxAddress);
			iter = space.end();
			break;
		}
		if( referencedMemory->m_min_addr > g_addr_max )
		{
			printf_debug("Limit reached: referencedMemory->m_min_addr (0x%.8X) > g_addr_max (0x%.8X)\n", referencedMemory->m_min_addr, g_addr_max);
			iter = space.end();
			break;
		}
		/*
		//Should only happen if we enacted partial anlysis
		//This is still valid
		if( referencedMemory->m_max_addr > superblockMaxAddress )
		{
			printf_debug("Limit reached: referencedMemory->m_max_addr (0x%.8X) > superblockMaxAddress (0x%.8X)\n", referencedMemory->m_max_addr, superblockMaxAddress);
			iter = space.end();
			break;
		}
		*/
		
		uint32_t memTypes = referencedMemory->getReferenceTypes();
		printf_debug("item type: 0x%.8X, wanted; 0x%.8X\n", memTypes, types);
		//Have we hit our type yet?
		if( (memTypes & types) == types )
		{
			break;
		}
	}
	return UV_ERR_OK;
}

uv_err_t UVD::constructFunctionBlocks(UVDAnalyzedBlock *superblock)
{
	uv_err_t rc = UV_ERR_GENERAL;
	//Need sorted list, convert from map stuff
	//UVDAnalyzedMemoryLocations calledLocations;
	uint32_t superblockMinAddress = 0;
	uint32_t superblockMaxAddress = 0;
	UVDAnalyzedMemoryLocations jumpedLocations;
	UVDAnalyzedBlock *functionBlock = NULL;
	
	//Addresses that are (directly?) referened in some way
	UVDAnalyzedMemoryLocations referencedAddresses;
	UVDAnalyzedMemoryLocations::iterator iterAddresses;
	UVDAnalyzedMemoryLocations::iterator iterJumped;
	//UVDAnalyzedMemorySpace jumpedAddresses;
	//UVDAnalyzedMemorySpace calledAddresses;

	UVDAnalysisDBArchive *curDb = NULL;
	

	printf_debug("\n");
	printf_debug("\n");
	UV_ENTER();
	
	uv_assert_ret(m_analyzer);
	uv_assert_err_ret(m_analyzer->getAnalyzedProgramDB(&curDb));
	uv_assert_ret(curDb);

	uv_assert_ret(superblock);
	uv_assert_err_ret(superblock->getMinAddress(superblockMinAddress));
	uv_assert_err_ret(superblock->getMaxAddress(superblockMaxAddress));
	printf_debug("Superblock: 0x%.8X : 0x%.8X\n", superblockMinAddress, superblockMaxAddress);
	
	printf_debug("Getting addresses\n");
	uv_assert_err_ret(m_analyzer->getAddresses(referencedAddresses));
	
	//uv_assert_err_ret(m_analyzer->getJumpedAddresses(jumpedAddresses));
	//printf_debug("Jumped addresses: %d\n", jreferencedAddressesumpedAddresses.size());
	//uv_assert_err(memorySpaceToMemoryLocations(jumpedAddresses, jumpedLocations));
	iterJumped = referencedAddresses.begin();

	//uv_assert_err_ret(m_analyzer->getCalledAddresses(calledAddresses));
	//printf_debug("Called addresses: %d\n", calledAddresses.size());
	//uv_assert_err(memorySpaceToMemoryLocations(calledAddresses, calledLocations));

	/*
	Seek to find first call entry point
	Consider all space after each until next (or end of program?) to be part of function
	Later will need to analyze branching and rets to better understand how functions exist
	*/
	
	iterAddresses = referencedAddresses.begin();
	printf_debug("Starting func parse, ref address: %d\n", referencedAddresses.size());
	//Seek for first func start
	uv_assert_err(nextReferencedAddress(superblock, referencedAddresses, iterAddresses, UVD_MEMORY_REFERENCE_CALL_DEST, true));

	printf_debug("Starting at end? %d\n", iterAddresses == referencedAddresses.end());

	while( iterAddresses != referencedAddresses.end() )
	{
		UVDAnalyzedMemoryLocations::iterator iterNextAddress = iterAddresses;
		UVDAnalyzedMemoryLocation *referencedMemory = *iterAddresses;
		uint32_t functionBlockStart = 0;
		uint32_t functionBlockEnd = 0;

		uv_assert_err(nextReferencedAddress(superblock, referencedAddresses, iterNextAddress, UVD_MEMORY_REFERENCE_CALL_DEST));

		uv_assert_ret(referencedMemory);
		printf_debug("Function address: 0x%.8X:0x%.8X\n", referencedMemory->m_min_addr, referencedMemory->m_max_addr);
		functionBlockStart = referencedMemory->m_min_addr;
		//Note: this code sets this so probably isn't reliable yet here
		functionBlockEnd = referencedMemory->m_max_addr;

		if( iterNextAddress != referencedAddresses.end() )
		{
			UVDAnalyzedMemoryLocation *nextReferencedMemory = *iterNextAddress;
			
			uv_assert_ret(nextReferencedMemory);
			//Even if overlapping function, min address should always advance
			uv_assert_ret(nextReferencedMemory->m_min_addr > referencedMemory->m_min_addr);
			
			//One before next block
			functionBlockEnd = nextReferencedMemory->m_min_addr - 1;
		}
		else
		{
			//Assume rest of file
			functionBlockEnd = superblockMaxAddress;
		}
		
		//Truncate to end of analyzed region if necessary
		if( functionBlockEnd > g_addr_max )
		{
			functionBlockEnd = g_addr_max;
		}
		
		/*
		Here we are making a very aggressive guess as to what the function consists of
		We take the maximum possible code
		It will be trimmed down later if we find we have taken too much
		Bad data could be a jumped location after our func, ROM data, ISR, function pointer target, w/e
		*/
		uv_assert_err_ret(constructBlock(functionBlockStart, functionBlockEnd, &functionBlock));
		uv_assert_ret(functionBlock);
		superblock->m_blocks.push_back(functionBlock);
		
		//Inner/block analysis
		//uv_assert_err(constructJumpBlocks(functionBlock, referencedAddresses, iterJumped));
		//Analyze control structures: if, else, (return trimming?), etc
		uv_assert_err(analyzeBlock(functionBlock));
	
		UVDBinaryFunction *function = NULL;
		UVDBinaryFunctionShared *functionShared = NULL;
		uv_assert_err_ret(blockToFunction(functionBlock, &function));
		functionShared = function->m_shared;
		uv_assert_err_ret(curDb->loadFunction(functionShared));
	
		iterAddresses = iterNextAddress;
	}

	rc = UV_ERR_OK;
	
error:
	return UV_DEBUG(rc);
}

uv_err_t UVD::constructBlocks()
{
	uv_err_t rc = UV_ERR_GENERAL;
	UVDAnalyzedBlock *superblock = NULL;
	
	printf_debug("\n");
	UV_ENTER();
	printf_debug_level(UVD_DEBUG_PASSES, "uvd: block analysis...\n");
	UVDBenchmark blockAnalysisBenchmark;
	blockAnalysisBenchmark.start();

	
	//Highest level block: entire program
	uv_assert_err(constructBlock(g_addr_min, g_addr_max, &superblock));
	m_analyzer->m_block = superblock;

	//Find functions, add them as sub blocks
	//Since functions were already pre-processed, we can use that map and don't actually need to iterate
	//This saves a lot of time since scripting is very slow right now
	
	//Functions
	uv_assert_err(constructFunctionBlocks(superblock));
	
	printf_debug("\n");

//DEBUG_BREAK();

	blockAnalysisBenchmark.stop();
	printf_debug_level(UVD_DEBUG_PASSES, "block analysis time: %s\n", blockAnalysisBenchmark.toString().c_str());

	rc = UV_ERR_OK;
	
error:
	return UV_DEBUG(rc);
}

uv_err_t UVD::analyzeBlock(UVDAnalyzedBlock *block)
{
	uv_err_t rc = UV_ERR_OK;
	
//error:
	return UV_DEBUG(rc);
}

uv_err_t UVD::generateAnalysisDir()
{
	UVDAnalysisDBConcentrator *dbConcentrator = NULL;
	UVDAnalysisDBArchive *curDb = NULL;

	uv_assert_ret(m_config);
	if( m_config->m_analysisDir.empty() )
	{
		return UV_ERR_OK;
	}
	
	printf_debug_level(UVD_DEBUG_PASSES, "uvd: generating analysis save files...\n");
	UVDBenchmark analysisSaveBenchmark;
	analysisSaveBenchmark.start();

	uv_assert_ret(m_analyzer);
	dbConcentrator = m_analyzer->m_db;
	uv_assert_ret(dbConcentrator);
	
	printf_debug("Fetching DB...\n");
	//uv_assert_err_ret(dbConcentrator->getAnalyzedProgramDB(&curDb));
	uv_assert_err_ret(m_analyzer->getAnalyzedProgramDB(&curDb));
	uv_assert_ret(curDb);

	printf_debug("Saving data...\n");
	uv_assert_err_ret(curDb->saveData(m_config->m_analysisDir));
	printf_debug("Data saved!\n");
	
	analysisSaveBenchmark.stop();
	printf_debug_level(UVD_DEBUG_PASSES, "analysis save time: %s\n", analysisSaveBenchmark.toString().c_str());

	return UV_ERR_OK;
}

uv_err_t UVD::analyze()
{
	uv_err_t rc = UV_ERR_GENERAL;
	int verbose_pre = g_verbose;
	UVDIterator iter;
	int printPercentage = 1;
	int printNext = printPercentage;
	UVDBenchmark controlStructureAnalysisBenchmark;

	UV_ENTER();
	
	g_verbose = g_verbose_analysis;
	
	//Set a default compiler to generate code for
	//How this is set will probably change dramatically in the future
	uv_assert(m_format);
	m_format->m_compiler = new UVDCompiler();
	
	if( UV_FAILED(analyzeStrings()) )
	{
		UV_DEBUG(rc);
		goto error;
	}
	
	printf_debug_level(UVD_DEBUG_PASSES, "uvd: raw control structure analysis...\n");
	controlStructureAnalysisBenchmark.start();

	iter = begin();
	for( ;; )
	{
		UVDInstruction instruction;
		std::string action;
		uint32_t startPos = iter.getPosition();
		uint32_t endPos = 0;
		
		int curPercent = 100 * startPos / g_addr_max;
		if( curPercent >= printNext )
		{
			printf_debug_level(UVD_DEBUG_SUMMARY, "uvd: raw control structure analysis: %d %%\n", curPercent);
			printNext += printPercentage;
		}

		printf_debug("\n\nAnalysis at: 0x%.8X\n", startPos);

		//If we aren't at end, there should be more data
		if( UV_FAILED(iter.nextInstruction(instruction)) )
		{
			return UV_DEBUG(UV_ERR_GENERAL);
		}
		if( iter == end() )
		{
			printf_debug("disassemble: end");
			break;
		}
		endPos = iter.getPosition();
		
		action = instruction.m_shared->m_action;

		printf_debug("Next instruction (start: 0x%.8X, end: 0x%.8X): %s\n", startPos, endPos, instruction.m_shared->m_memoric.c_str());

		uv_assert_err(instruction.m_shared->analyzeAction());
		printf_debug("Action: %s, type: %d\n", action.c_str(), instruction.m_shared->m_inst_class);
		//See if its a call instruction
		if( instruction.m_shared->m_inst_class == UVD_INSTRUCTION_CLASS_CALL )
		{
			/*
			NAME=ACALL
			DESC=Absolute Call (page 3)
			USAGE=0x71,u8_0
			SYNTAX=u8_0
			ACTION=CALL(%PC&0x1F00+u8_0+0x6000)
			*/
			
			unsigned int targetAddress = 0;
			UVDVariableMap environment;
			UVDVariableMap mapOut;
			std::string sAddr;

			uv_assert_err(instruction.collectVariables(environment));
						
			/*
			Add iterator specific environment
			*/

			//Register environment
			//PC/IP is current instruction location
			environment["PC"] = UVDVarient(endPos);

			//About 0.03 sec per exec...need to speed it up
			//Weird...cast didn't work to solve pointer incompatibility
			uv_assert_ret(m_interpreter);
			uv_assert_err(m_interpreter->interpretKeyed(action, environment, mapOut));
			if( mapOut.find(SCRIPT_KEY_CALL) == mapOut.end() )
			{
				UV_DEBUG(rc);
				goto error;
			}
			mapOut[SCRIPT_KEY_CALL].getString(sAddr);
			targetAddress = (unsigned int)strtol(sAddr.c_str(), NULL, 0);
			
			uv_assert_err(m_analyzer->insertCallReference(targetAddress, startPos));
			//uv_assert_err(m_analyzer->insertReference(targetAddress, startPos, ));
			m_analyzer->updateCache(startPos, mapOut);
		}
		else if( instruction.m_shared->m_inst_class == UVD_INSTRUCTION_CLASS_JUMP )
		{
			/*
			NAME=JNB
			DESC=Jump if Bit Not Set
			USAGE=0x30,u8_0,u8_1
			SYNTAX=u8_0,u8_1
			ACTION=GOTO(%PC+u8_1)
			*/

			unsigned int targetAddress = 0;
			UVDVariableMap environment;
			UVDVariableMap mapOut;
			std::string sAddr;

			uv_assert_err(instruction.collectVariables(environment));
						
			/*
			Add iterator specific environment
			*/

			//Register environment
			//PC/IP is current instruction location
			environment["PC"] = UVDVarient(endPos);

			//About 0.03 sec per exec...need to speed it up
			//Weird...cast didn't work to solve pointer incompatibility
			uv_assert(m_interpreter);
			if( UV_FAILED(m_interpreter->interpretKeyed(action, environment, mapOut)) )
			{
				UV_DEBUG(rc);
				goto error;
			}
			if( mapOut.find(SCRIPT_KEY_JUMP) == mapOut.end() )
			{
				UV_DEBUG(rc);
				goto error;
			}
			mapOut[SCRIPT_KEY_JUMP].getString(sAddr);
			targetAddress = (unsigned int)strtol(sAddr.c_str(), NULL, 0);
			
			uv_assert_err(m_analyzer->insertJumpReference(targetAddress, startPos));
			m_analyzer->updateCache(startPos, mapOut);
		}
	}
	controlStructureAnalysisBenchmark.stop();
	printf_debug_level(UVD_DEBUG_PASSES, "control structure analysis time: %s\n", controlStructureAnalysisBenchmark.toString().c_str());
	
	//Now that instructions have undergone basic processing, turn code into blocks
	uv_assert_err(constructBlocks());
	
	uv_assert_err_ret(generateAnalysisDir());
	
	rc = UV_ERR_OK;
	
error:
	g_verbose = verbose_pre;
	return UV_DEBUG(rc);
}


UVD::UVD()
{
	m_data = NULL;
	m_opcodeTable = NULL;
	m_architecture = 0;
	m_interpreter = NULL;
	m_analyzer = NULL;
	m_format = NULL;
}

//Factory function for construction
uv_err_t UVD::getUVD(UVD **uvdIn, UVDData *data)
{
	UVD *uvd = NULL;
		
	uvd = new UVD();
	if( !uvd )
	{
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	if( UV_FAILED(uvd->init(data)) )
	{
		delete uvd;
		return UV_DEBUG(UV_ERR_GENERAL);
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
	UVDIterator i = UVDIterator(this);
	return i;
}

UVDIterator UVD::end()
{
	//Pos is "next position"
	//Size is first invalid position
	UVDIterator iter = UVDIterator(this, m_data->size());
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

uv_err_t UVD::disassemble(std::string file, std::string &output)
{
	return UV_DEBUG(decompile(file, UVD_LANGUAGE_ASSEMBLY, output));
}

uv_err_t UVD::decompile(std::string file, int destinationLanguage, std::string &output)
{
	//uint8_t *dat = NULL;	
	unsigned int dat_sz = 0;
	//unsigned int read = 0;
	std::string configFile;
	UVDIterator iter;
	//UVDIterator iterEnd;
	int printPercentage = 1;
	int printNext = printPercentage;
	
	UV_ENTER();

	UVDBenchmark decompileBenchmark;
	decompileBenchmark.start();
	
	/*
	printf_debug("Reading program data...\n");
	if( UV_FAILED(read_file(file.c_str(), &dat, &dat_sz)) )
	{
		printf("Couldn't read file: %s\n", file.c_str());
		UV_ERR(rc);
		goto error;		
	}
	*/
	dat_sz = m_data->size();
	
	//Trivial case: nothing to analyze
	if( dat_sz == 0 )
	{
		return UV_ERR_OK;
	}
	
	//If unspecified, default to full range
	if( g_addr_max == 0 )
	{
		g_addr_max = dat_sz - 1;
	}
	printf_debug("Raw data size: 0x%x (%d)\n", dat_sz, dat_sz);

	//Most of program time should be spent here
	uv_assert_err_ret(analyze());

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

		int curPercent = 100 * startPos / g_addr_max;
		if( curPercent >= printNext )
		{
			uint64_t delta = getTimingMicroseconds() - decompilePrintBenchmark.getStart();
			double iterationTime = 1.0 * delta / iterations;
			printf_debug_level(UVD_DEBUG_SUMMARY, "uvd: printing: %d %% (us / record: %.2lf)\n", curPercent, iterationTime);
			printNext += printPercentage;
		}

		line = *iter;
		printf_debug("Line (0x%.8X): %s\n", iter.getPosition(), line.c_str());

		//This didn't help, must be some other bottleneck
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
		/*
		if( startPos > 16 )
		{
			break;
		}
		*/
				
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
	
	if( g_print_used )
	{
		m_opcodeTable->usedStats();
	}

	decompilePrintBenchmark.stop();
	printf_debug_level(UVD_DEBUG_PASSES, "decompile print time (%d records): %s\n", iterations, decompilePrintBenchmark.toString().c_str());
		
	printf_debug_level(UVD_DEBUG_PASSES, "decompile: done\n");
	decompileBenchmark.stop();
	printf_debug_level(UVD_DEBUG_PASSES, "decompile time: %s\n", decompileBenchmark.toString().c_str());
	
//printf("DEBUG BREAK\n");
//exit(1);

	return UV_ERR_OK;
}

uv_err_t UVD::changeConfig(UVDConfig *config)
{
	if( m_config )
	{
		delete m_config;
	}
	m_config = config;
	return UV_ERR_OK;
}
