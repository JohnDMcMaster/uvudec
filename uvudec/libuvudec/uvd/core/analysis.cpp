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
#include "uvd/core/block.h"
#include "uvd/architecture/architecture.h"
#include "uvd/assembly/cpu_vector.h"
#include "uvd/util/benchmark.h"
#include "uvd/util/util.h"
#include "uvd/core/runtime.h"

int g_filterPostRet;

int g_analyzeOtherFunctionJump;

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

uv_err_t UVD::mapSymbols()
{
	/*
	Map symbols into functions
	*/
	
	uv_assert_ret(m_analyzer);
	uv_assert_err_ret(m_analyzer->mapSymbols());
	
	return UV_ERR_OK;
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
	/*
	Linear flow anlaysis constructs functions in a greedy manner:
	A function is defined as all of the code between one function entry and the next function entry
	Each time a function is found we will add it and it will take up the remaining data, spliting as necessary
	*/
	
	UVDInstructionIterator iter;
	UVDInstructionIterator iterEnd;
	//uint32_t printPercentage = 1;
	//uint32_t printNext = printPercentage;
	//uv_addr_t numberAnalyzedBytes = 0;
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
	uv_assert_err_ret(iterEnd.check());
	uv_assert_ret(m_config);
	//FIXME: iterator rework
	//uv_assert_err_ret(iter.m_address.m_space->getNumberAnalyzedBytes(&numberAnalyzedBytes));

	for( ;; )
	{
		UVDAddress startPos;
		uv_assert_err_ret(iter.getAddress(&startPos));
		UVDInstruction *instruction = NULL;
		
		//printf("analysis: ITERATING\n");
		uv_assert_err_ret(iter.check());
		
		if( iter == iterEnd )
		{
			printf_debug("disassemble: end");
			break;
		}

		uv_assert_err_ret(iter.get(&instruction));
		
		if( instruction ) {
			//printf("\n\nAnalysis at: 0x%.8X\n", startPos);		
			uv_assert_err_ret(instruction->analyzeControlFlow());
		}

		//If we aren't at end, there should be more data
		uv_assert_err_ret(iter.next());
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
	
	UVDInstructionIterator end;
	uv_assert_err_ret(instructionEnd(end));
	
	while( !openSet.empty() )
	{
		uint32_t nextStartAddress = *openSet.begin();
		UVDInstructionIterator iter;
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

		uv_assert_err_ret(instructionBeginByAddress(nextStartAddress, iter));
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

			//XXX is this really correct?  what if we jumped towards the end
			//should be more of a continue
			if( iter == end )
			{
				printf_debug("disassemble: end");
				break;
			}
			uv_assert_err_ret(iter.get(&instruction));
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
	//uv_assert_err(constructBlocks());
	
	rc = UV_ERR_OK;
	
error:
	m_config->m_verbose = verbose_pre;
	return UV_DEBUG(rc);
}

