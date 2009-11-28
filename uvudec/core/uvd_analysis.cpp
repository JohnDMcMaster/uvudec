/*
Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#include "uvd_benchmark.h"
#include "uvd_analysis.h"
#include "uvd.h"

int g_filterPostRet;

int g_analyzeOtherFunctionJump;

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

	//printf_debug("Starting at end? %d\n", iterAddresses == referencedAddresses.end());

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
		uv_assert_err_ret(analyzeFunction(functionShared));
		//uv_assert_err_ret(curDb->loadFunction(functionShared));
		uv_assert_err_ret(m_analyzer->loadFunction(function));
	
		iterAddresses = iterNextAddress;
	}
	printf_debug_level(UVD_DEBUG_SUMMARY, "found functions: %d\n", curDb->m_functions.size());

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
	UVDData *data = NULL;

	uv_assert_ret(binaryFunctionCodeShared);
	data = binaryFunctionCodeShared->getData();
	uv_assert_ret(data);
	
	return UV_ERR_OK;
}

uv_err_t UVD::constructBlocks()
{
	uv_err_t rc = UV_ERR_GENERAL;
	UVDAnalyzedBlock *superblock = NULL;
	
	printf_debug("\n");
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
	
	printf_debug_level(UVD_DEBUG_SUMMARY, "Fetching DB...\n");
	//uv_assert_err_ret(dbConcentrator->getAnalyzedProgramDB(&curDb));
	uv_assert_err_ret(m_analyzer->getAnalyzedProgramDB(&curDb));
	uv_assert_ret(curDb);
	printf_debug_level(UVD_DEBUG_SUMMARY, "going to save functions: %d\n", curDb->m_functions.size());

	printf_debug_level(UVD_DEBUG_SUMMARY, "Saving data...\n");
	uv_assert_err_ret(curDb->saveData(m_config->m_analysisDir));
	printf_debug_level(UVD_DEBUG_SUMMARY, "Data saved!\n");
	
	analysisSaveBenchmark.stop();
	printf_debug_level(UVD_DEBUG_PASSES, "analysis save time: %s\n", analysisSaveBenchmark.toString().c_str());

	return UV_ERR_OK;
}

uv_err_t UVD::analyzeControlFlow()
{
	UVDIterator iter;
	int printPercentage = 1;
	int printNext = printPercentage;
	UVDBenchmark controlStructureAnalysisBenchmark;

	printf_debug_level(UVD_DEBUG_PASSES, "uvd: control flow analysis...\n");
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
		uv_assert_err_ret(iter.nextInstruction(instruction));
		if( iter == end() )
		{
			printf_debug("disassemble: end");
			break;
		}
		endPos = iter.getPosition();
		
		action = instruction.m_shared->m_action;

		printf_debug("Next instruction (start: 0x%.8X, end: 0x%.8X): %s\n", startPos, endPos, instruction.m_shared->m_memoric.c_str());

		uv_assert_err_ret(instruction.m_shared->analyzeAction());
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

			uv_assert_err_ret(instruction.collectVariables(environment));
						
			/*
			Add iterator specific environment
			*/

			//Register environment
			//PC/IP is current instruction location
			environment["PC"] = UVDVarient(endPos);

			//About 0.03 sec per exec...need to speed it up
			//Weird...cast didn't work to solve pointer incompatibility
			uv_assert_ret(m_interpreter);
			uv_assert_err_ret(m_interpreter->interpretKeyed(action, environment, mapOut));
			uv_assert_ret(mapOut.find(SCRIPT_KEY_CALL) != mapOut.end());
			mapOut[SCRIPT_KEY_CALL].getString(sAddr);
			targetAddress = (unsigned int)strtol(sAddr.c_str(), NULL, 0);
			
			uv_assert_err_ret(m_analyzer->insertCallReference(targetAddress, startPos));
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

			uv_assert_err_ret(instruction.collectVariables(environment));
						
			/*
			Add iterator specific environment
			*/

			//Register environment
			//PC/IP is current instruction location
			environment["PC"] = UVDVarient(endPos);

			//About 0.03 sec per exec...need to speed it up
			//Weird...cast didn't work to solve pointer incompatibility
			uv_assert_ret(m_interpreter);
			uv_assert_err_ret(m_interpreter->interpretKeyed(action, environment, mapOut));
			uv_assert_ret(mapOut.find(SCRIPT_KEY_JUMP) != mapOut.end());
			mapOut[SCRIPT_KEY_JUMP].getString(sAddr);
			targetAddress = (unsigned int)strtol(sAddr.c_str(), NULL, 0);
			
			uv_assert_err_ret(m_analyzer->insertJumpReference(targetAddress, startPos));
			m_analyzer->updateCache(startPos, mapOut);
		}
	}
	controlStructureAnalysisBenchmark.stop();
	printf_debug_level(UVD_DEBUG_PASSES, "control flow analysis time: %s\n", controlStructureAnalysisBenchmark.toString().c_str());

	return UV_ERR_OK;
}

uv_err_t UVD::analyzeConstData()
{
	uv_assert_err_ret(analyzeStrings());
	return UV_ERR_OK;
}
	
uv_err_t UVD::analyze()
{
	uv_err_t rc = UV_ERR_GENERAL;
	int verbose_pre = g_verbose;
	
	g_verbose = g_verbose_analysis;
	
	//Set a default compiler to generate code for
	//How this is set will probably change drastically in the future
	uv_assert(m_format);
	m_format->m_compiler = new UVDCompiler();
	
	//Strings must be found first to find ROM data to exclude from disassembly
	uv_assert_err(analyzeConstData());
	//Then find constrol flow
	uv_assert_err(analyzeControlFlow());
	//Now that instructions have undergone basic processing,
	//turn code into blocks using the control flow
	uv_assert_err(constructBlocks());
	//And output analysis files, if requested
	uv_assert_err(generateAnalysisDir());
	
	rc = UV_ERR_OK;
	
error:
	g_verbose = verbose_pre;
	return UV_DEBUG(rc);
}
