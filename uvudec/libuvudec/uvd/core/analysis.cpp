/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

/*
This does UVD prepping analysis before passing data off to m_analyzer
Things such as interpreting results and converting to generic structures
*/

#include "uvd/core/uvd.h"
#include "uvd/core/analysis.h"
#include "uvd/architecture/architecture.h"
#include "uvd/assembly/cpu_vector.h"
#include "uvd/util/benchmark.h"
#include "uvd/util/util.h"
#include "uvd/core/runtime.h"

int g_filterPostRet;

int g_analyzeOtherFunctionJump;

static uv_err_t nextReferencedAddress(UVDAnalyzedBlock *superblock, UVDAnalyzedMemoryRanges &space, UVDAnalyzedMemoryRanges::iterator &iter, uint32_t types, bool curAddressLegal = false);

static uv_err_t nextReferencedAddress(UVDAnalyzedBlock *superblock, UVDAnalyzedMemoryRanges &space, UVDAnalyzedMemoryRanges::iterator &iter, uint32_t types, bool curAddressLegal)
{
	uint32_t superblockMaxAddress = 0;
	uv_addr_t absoluteMaxAddress = 0;
	
	uv_assert_err_ret(superblock->m_addressSpace->getMaxValidAddress(&absoluteMaxAddress));

	uv_assert_ret(superblock);
	uv_assert_err_ret(superblock->getMaxAddress(superblockMaxAddress));
	
	if( iter != space.end() && !curAddressLegal )
	{
		++iter;
	}

	for( ; iter != space.end(); ++iter )
	{
		UVDAnalyzedMemoryRange *referencedMemory = *iter;

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
		if( referencedMemory->m_min_addr > absoluteMaxAddress )
		{
			printf_debug("Limit reached: referencedMemory->m_min_addr (0x%.8X) > absoluteMaxAddress (0x%.8X)\n", referencedMemory->m_min_addr, absoluteMaxAddress);
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
	//UVDAnalyzedMemoryRanges calledLocations;
	uint32_t superblockMinAddress = 0;
	uint32_t superblockMaxAddress = 0;
	UVDAnalyzedMemoryRanges jumpedLocations;
	UVDAnalyzedBlock *functionBlock = NULL;
	
	//Addresses that are (directly?) referened in some way
	UVDAnalyzedMemoryRanges referencedAddresses;
	UVDAnalyzedMemoryRanges::iterator iterAddresses;
	UVDAnalyzedMemoryRanges::iterator iterJumped;
	//UVDAnalyzedMemorySpace jumpedAddresses;
	//UVDAnalyzedMemorySpace calledAddresses;

	//UVDAnalysisDBArchive *curDb = NULL;	
	uv_addr_t absoluteMaxAddress = 0;
	UVDAddressSpace *space = superblock->m_addressSpace;
	
	uv_assert_ret(m_config);
	uv_assert_ret(space);

	uv_assert_err_ret(superblock->m_addressSpace->getMaxValidAddress(&absoluteMaxAddress));

	printf_debug("\n");
	printf_debug("\n");
	
	uv_assert_ret(m_analyzer);
	//uv_assert_err_ret(m_analyzer->getAnalyzedProgramDB(&curDb));
	//uv_assert_ret(curDb);

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

	//printf_debug("Starting at end? %d\n", iterAddresses == referencedAddresses.end());

	while( iterAddresses != referencedAddresses.end() )
	{
		UVDAnalyzedMemoryRanges::iterator iterNextAddress = iterAddresses;
		UVDAnalyzedMemoryRange *referencedMemory = *iterAddresses;
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
			UVDAnalyzedMemoryRange *nextReferencedMemory = *iterNextAddress;
			
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
		if( functionBlockEnd > absoluteMaxAddress )
		{
			functionBlockEnd = absoluteMaxAddress;
		}
		
		/*
		Here we are making a very aggressive guess as to what the function consists of
		We take the maximum possible code
		It will be trimmed down later if we find we have taken too much
		Bad data could be a jumped location after our func, ROM data, ISR, function pointer target, w/e
		*/
		uv_assert_err_ret(constructBlock(UVDAddressRange(functionBlockStart, functionBlockEnd, space), &functionBlock));
		uv_assert_ret(functionBlock);
		superblock->m_blocks.push_back(functionBlock);
		
		//Inner/block analysis
		//uv_assert_err(constructJumpBlocks(functionBlock, referencedAddresses, iterJumped));
		//Analyze control structures: if, else, (return trimming?), etc
		uv_assert_err(analyzeBlock(functionBlock));
	
		UVDBinaryFunction *function = NULL;
		//UVDBinaryFunctionShared *functionShared = NULL;
		uv_assert_err_ret(blockToFunction(functionBlock, &function));
		//functionShared = function->m_shared;
		//uv_assert_err_ret(analyzeFunction(functionShared));
		//uv_assert_err_ret(curDb->loadFunction(functionShared));
		uv_assert_err_ret(m_analyzer->loadFunction(function));
	
		iterAddresses = iterNextAddress;
	}
	printf_debug_level(UVD_DEBUG_SUMMARY, "found functions: %d\n", m_analyzer->m_functions.size());

	rc = UV_ERR_OK;
	
error:
	return UV_DEBUG(rc);
}

/*
uv_err_t UVD::analyzeBinaryFunctionCodeShared(UVDBinaryFunctionInstance *binaryFunctionCodeShared)
{
	uv_assert_ret(binaryFunctionCodeShared);
	UVDIterator iter = begin(binaryFunctionCodeShared->m_data); 
	while( iter != end(binaryFunctionCodeShared->m_data) )
	{
		UVDInstruction instruction;
		//If we aren't at end, there should be more data
		uv_assert_err_ret(iter.nextInstruction(instruction));
		if( iter == end() )
		{
			printf_debug("disassemble: end");
			break;
		}
	}
}
*/

/*
uv_err_t UVD::analyzeFunction(UVDBinaryFunctionShared *functionShared)
{
	uv_assert_ret(functionShared);
	//Fills in the relocatable entries
	for( std::vector<UVDBinaryFunctionInstance *>::iterator iter = 
			functionShared->m_representations.begin(); 
			iter != functionShared->m_representations.end(); ++iter)
	{
		UVDBinaryFunctionInstance *binaryFunctionCodeShared = *iter;
		uv_assert_err_ret(analyzeFunctionRelocatables(binaryFunctionCodeShared));
		
	}
	
	return UV_ERR_OK;
}

uv_err_t UVD::analyzeFunctionRelocatables(UVDBinaryFunctionInstance *binaryFunctionCodeShared)
{
	//FIXME: this code looks like it doesn't do anything
	UVDData *data = NULL;

	uv_assert_ret(binaryFunctionCodeShared);
	data = binaryFunctionCodeShared->getData();
	uv_assert_ret(data);
	
	return UV_ERR_OK;
}
*/

uv_err_t UVD::constructBlocks()
{
	uv_err_t rc = UV_ERR_GENERAL;
	UVDAnalyzedBlock *superblock = NULL;
	uv_addr_t absoluteMaxAddress = 0;
	uv_addr_t absoluteMinAddress = 0;
	UVDAddressSpace *addressSpace = NULL;
	
	printf_debug("\n");
	printf_debug_level(UVD_DEBUG_PASSES, "uvd: block analysis...\n");
	UVDBenchmark blockAnalysisBenchmark;
	blockAnalysisBenchmark.start();

	uv_assert_err_ret(m_runtime->getPrimaryExecutableAddressSpace(&addressSpace));

	uv_assert_ret(m_config);
	uv_assert_err_ret(addressSpace->getMinValidAddress(&absoluteMinAddress));
	uv_assert_err_ret(addressSpace->getMaxValidAddress(&absoluteMaxAddress));
	
	//Highest level block: entire program
	uv_assert_err(constructBlock(UVDAddressRange(absoluteMinAddress, absoluteMaxAddress, addressSpace), &superblock));
	m_analyzer->m_block = superblock;

	//Find functions, add them as sub blocks
	//Since functions were already pre-processed, we can use that map and don't actually need to iterate
	//This saves a lot of time since scripting is very slow right now
	
	//Functions
	uv_assert_err(constructFunctionBlocks(superblock));
	
	//Map the symbols into blocks/functions
	uv_assert_err(mapSymbols());	

	printf_debug("\n");

	blockAnalysisBenchmark.stop();
	printf_debug_level(UVD_DEBUG_PASSES, "block analysis time: %s\n", blockAnalysisBenchmark.toString().c_str());

	rc = UV_ERR_OK;
	
error:
	return UV_DEBUG(rc);
}

uv_err_t UVD::mapSymbols()
{
	/*
	Map symbols into functions
	*/
	
	uv_assert_ret(m_analyzer);
	uv_assert_err_ret(m_analyzer->mapSymbols());
	
	return UV_ERR_OK;
}

uv_err_t UVD::analyzeBlock(UVDAnalyzedBlock *block)
{
	uv_err_t rc = UV_ERR_OK;
	
//error:
	return UV_DEBUG(rc);
}

uv_err_t UVD::analyzeControlFlow()
{
	UVDBenchmark controlStructureAnalysisBenchmark;

	printf_debug_level(UVD_DEBUG_PASSES, "uvd: control flow analysis...\n");
	controlStructureAnalysisBenchmark.start();

	switch( m_config->m_flowAnalysisTechnique )
	{
	case UVD__FLOW_ANALYSIS__LINEAR:
		uv_assert_err_ret(analyzeControlFlowLinear());
		break;
	case UVD__FLOW_ANALYSIS__TRACE:
		uv_assert_err_ret(analyzeControlFlowTrace());
		break;
	default:
		return UV_DEBUG(UV_ERR_GENERAL);
	};

	controlStructureAnalysisBenchmark.stop();
	printf_debug_level(UVD_DEBUG_PASSES, "control flow analysis time: %s\n", controlStructureAnalysisBenchmark.toString().c_str());

	return UV_ERR_OK;
}

uv_err_t UVD::analyzeControlFlowLinear()
{
	UVDInstructionIterator iter;
	UVDInstructionIterator iterEnd;
	uint32_t printPercentage = 1;
	uint32_t printNext = printPercentage;
	uv_addr_t numberAnalyzedBytes = 0;
	uv_addr_t startPosition = 0;
	
	printf_debug_level(UVD_DEBUG_PASSES, "control flow analysis linear\n");

	//Need at least one start location
	uv_assert_ret(!m_runtime->m_architecture->m_vectors.empty());
	//FIXME: take the lowest vector
	startPosition = m_runtime->m_architecture->m_vectors[0]->m_offset;
	if( m_runtime->m_architecture->m_vectors.size() != 1 )
	{
		printf_warn("%d vectors, only sweeping from 0x%08X, consider using recursive descent or specify start\n",
				m_runtime->m_architecture->m_vectors.size(), startPosition);
	}
	uv_assert_err_ret(instructionBegin(iter));
	uv_assert_err_ret(instructionEnd(iterEnd));
	uv_assert_ret(m_config);
	uv_assert_err_ret(iter.m_addressSpace->getNumberAnalyzedBytes(&numberAnalyzedBytes));

	for( ;; )
	{
		uint32_t startPos = iter.getPosition();
		UVDInstruction *instruction = iter.m_instruction;
		
		if( numberAnalyzedBytes )
		{
			uint32_t curPercent = 100 * startPos / numberAnalyzedBytes;
			if( curPercent >= printNext )
			{
				printf_debug_level(UVD_DEBUG_SUMMARY, "uvd: raw control structure analysis: %d %%\n", curPercent);
				printNext += printPercentage;
			}
		}

		//printf("\n\nAnalysis at: 0x%.8X\n", startPos);		
		uv_assert_err_ret(instruction->analyzeControlFlow());

		//If we aren't at end, there should be more data
		uv_assert_err_ret(iter.next());
		if( iter == iterEnd )
		{
			printf_debug("disassemble: end");
			break;
		}
	}

	return UV_ERR_OK;
}

uv_err_t UVD::suspectValidInstruction(uint32_t address, int *isValid)
{
	/*	
	FIXME: Should probably actually be in UVDOpcodeLookupTable
	Oftentimes these will be 0xFF or 0x00 when unused
	Also consider if address is in a string table or other blacklisted area
	*/
	uv_assert_ret(isValid);
	
	//FIXME: do check
	*isValid = true;
	
	return UV_ERR_OK;
}

uv_err_t UVD::analyzeControlFlowTrace()
{
	printf_debug_level(UVD_DEBUG_PASSES, "control flow analysis trace / recursive descent\n");
	//For now only try to find where code is, so it doesn't matter how we arrived
	std::set<uint32_t> openSet;
	std::set<uint32_t> closedSet;
	
	//Probably need full ranges, do only start for now
	std::set<uint32_t> calls;
	std::set<uint32_t> jumps;
	//uv_addr_t numberAnalyzedBytes = 0;
	
	uv_assert_ret(m_runtime->m_architecture);
	uv_assert_ret(m_config);
	//uv_assert_err_ret(m_analyzer->getNumberAnalyzedBytes(&numberAnalyzedBytes));

	//Another way to do with would be to do "START" and then all other vectors
	for( std::vector<UVDCPUVector *>::iterator iter = m_runtime->m_architecture->m_vectors.begin();
			iter != m_runtime->m_architecture->m_vectors.end(); ++iter )
	{
		UVDCPUVector *vector = *iter;
		
		uv_assert_ret(vector);
		openSet.insert(vector->m_offset);
	}
	
	while( !openSet.empty() )
	{
		uint32_t nextStartAddress = *openSet.begin();
		UVDIterator iter;
		int isVectorValid = 0;

		openSet.erase(openSet.begin());
		//Did we already try to process it?
		if( closedSet.find(nextStartAddress) != closedSet.end() )
		{
			continue;
		}
		closedSet.insert(nextStartAddress);
	
		//Make sure it seems reasonable
		uv_assert_err_ret(suspectValidInstruction(nextStartAddress, &isVectorValid));
		{
		if( !isVectorValid )
			printf_warn("ignoring address: 0x%08X\n", nextStartAddress);
			continue;
		}

		//Keep going until we hit a branch point

		//uint32_t printPercentage = 1;
		//uint32_t printNext = printPercentage;

		iter = begin(nextStartAddress);
		for( ;; )
		{
			UVDInstructionAnalysis instructionAnalysis;
			//std::string action;
			//uint32_t startPos = iter.getPosition();
			//uint32_t endPos = 0;
			
			/*
			if( numberAnalyzedBytes )
			{
				uint32_t curPercent = 100 * startPos / numberAnalyzedBytes;
				if( curPercent >= printNext )
				{
					printf_debug_level(UVD_DEBUG_SUMMARY, "uvd: raw control structure analysis: %d %%\n", curPercent);
					printNext += printPercentage;
				}
			}
			*/

			UVDInstruction *instruction = NULL;

			//printf_debug("\n\nAnalysis at: 0x%.8X\n", startPos);

			if( iter == end() )
			{
				printf_debug("disassemble: end");
				break;
			}
			instruction = iter.m_instruction;
			//endPos = iter.getPosition();
			
			//action = instruction.m_shared->m_action;

			//printf_debug("Next instruction (start: 0x%.8X, end: 0x%.8X): %s\n", startPos, endPos, instruction.m_shared->m_memoric.c_str());

			//printf_debug("Action: %s, type: %d\n", action.c_str(), instruction.m_shared->m_inst_class);
			uv_assert_err_ret(instruction->analyzeControlFlow(&instructionAnalysis));
			//Only care about resolved targets
			if( instructionAnalysis.m_isJump == UVD_TRI_TRUE || instructionAnalysis.m_isCall == UVD_TRI_TRUE )
			{
				openSet.insert(nextStartAddress);
			}
			//Unconditional jumps mean end of sweep
			//TODO: also consider adding something for noreturn type functions?
			//rare on embedded platforms?
			if( instructionAnalysis.m_isJump == UVD_TRI_TRUE && !instructionAnalysis.m_isConditional )
			{
				//Straight JMP style instruction
				break;
			}

			//If we aren't at end, there should be more data
			uv_assert_err_ret(iter.next());
		}
	}
	
	return UV_DEBUG(UV_ERR_GENERAL);
}

uv_err_t UVD::analyzeConstData()
{
	uv_assert_err_ret(analyzeStrings());
	return UV_ERR_OK;
}

uv_err_t UVD::analyze()
{
	uv_err_t rc = UV_ERR_GENERAL;
	int verbose_pre = 0;
	
	uv_assert_ret(m_config);
	
	verbose_pre = m_config->m_verbose;
	
	m_config->m_verbose = m_config->m_verbose_analysis;	
	
	//Strings must be found first to find ROM data to exclude from disassembly
	uv_assert_err(analyzeConstData());
	//Then find constrol flow
	uv_assert_err(analyzeControlFlow());
	//Now that instructions have undergone basic processing,
	//turn code into blocks using the control flow
	uv_assert_err(constructBlocks());
	
	rc = UV_ERR_OK;
	
error:
	m_config->m_verbose = verbose_pre;
	return UV_DEBUG(rc);
}

