/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/config.h"
#include <algorithm>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "uvd/core/block.h"
#include "uvd/core/uvd.h"
#include "uvd/util/debug.h"
#include "uvd/util/error.h"
#include "uvd/util/util.h"
#include "uvd/assembly/address.h"
#include "uvd/assembly/function.h"
#include "uvd/assembly/instruction.h"
#include "uvd/compiler/assembly.h"
#include "uvd/core/analysis.h"
#include "uvd/core/std_iterator.h"
#include "uvd/core/runtime.h"
#include "uvd/data/data.h"
#include "uvd/language/format.h"
#include "uvd/language/language.h"
#include "uvd/string/engine.h"
#include "uvd/util/types.h"
#include "uvd/util/benchmark.h"

UVD *g_uvd = NULL;

/*
TODO: there was a paradigm shift between how to analyze functions
Moving towards DB based storage
Each of these accomplish essentially same thing, but different way
*/

#if 0
//FIXME: this code is nearly dead and should probably be removed
uv_err_t UVD::analyzeNewFunction(const UVDAnalyzedMemoryRange *memLoc, UVDAnalyzedFunction &analyzedFunction)
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
#endif


#if 0
uv_err_t UVD::analyzeCode(UVDAnalyzedCode &analyzedCode)
{
	return UV_ERR_OK;
}
#endif

uv_err_t UVD::analyzeStrings()
{
	uv_assert_ret(m_analyzer->m_stringEngine);
	return UV_DEBUG(m_analyzer->m_stringEngine->analyze());
}

/*
static int UVDAnalyzedMemorySpaceSorter(UVDAnalyzedMemoryRange *l, UVDAnalyzedMemoryRange *r)
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

uv_err_t memorySpaceToMemoryLocations(const UVDAnalyzedMemorySpace &space, UVDAnalyzedMemoryRanges& ret)
{
	uv_err_t rc = UV_ERR_GENERAL;

	ret.clear();
	
	//Some way to make insertion copy, but can't remember off of top of head
	//std::copy(ret.begin(), space.begin(), space.end());
	for( UVDAnalyzedMemorySpace::const_iterator iter = space.begin(); iter != space.end(); ++iter )
	{
		UVDAnalyzedMemoryRange *memoryLocation = (*iter).second;
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



UVD::UVD()
{
	m_analyzer = NULL;
	m_format = NULL;
	m_config = NULL;
	m_pluginEngine = NULL;
	m_runtime = NULL;
	//m_flirt = NULL;
	m_eventEngine = NULL;
}

UVD::~UVD()
{
	deinit();
}

uv_err_t UVD::deinit()
{
	UVD_POKE(this);
	UVD_POKE(&m_runtime);
	UVD_POKE(&m_analyzer);
	UVD_POKE(&m_config);
	UVD_POKE(&m_format);
	//UVD_POKE(&m_flirt);
	UVD_POKE(&m_eventEngine);
	UVD_POKE(&m_pluginEngine);

	printf_debug_level(UVD_DEBUG_PASSES, "UVD::deinit, this=0x%08X, g_uvd=0x%08X, m_config=0x%08X, g_config=0x%08X\n",
			(int)this, (int)g_uvd, (int)m_config, (int)g_config);
	if( m_config )
	{
		uv_assert_err_ret(m_config->m_plugin.m_pluginEngine.onUVDDeinit());
	}
	
	//m_data deallocated by UVD engine caller
	
	delete m_analyzer;
	m_analyzer = NULL;

	delete m_format;
	m_format = NULL;

	//To help migrate from global instances
	//At the time of this writing, g_config is deleted during UVDDeinit()
	if( m_config != g_config )
	{
		delete m_config;
		m_config = NULL;
	}
	
	if( g_uvd == this )
	{
		g_uvd = NULL;
	}
	
	return UV_ERR_OK;
}

uv_err_t UVD::getUVDFromFileName(UVD **uvdOut, const std::string &file)
{
	UVD *uvd = NULL;
	
	//Because code needs lots of fixing before this will work
	if( g_uvd )
	{
		printf_error("sorry, only one UVD instance supported at a time (active = 0x%08X)\n", (int)g_uvd);
		return UV_DEBUG(UV_ERR_NOTIMPLEMENTED);
	}

	uvd = new UVD();
	if( !uvd )
	{
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	
	uv_assert_ret(g_config);
	uvd->m_config = g_config;
	
	if( UV_FAILED(uvd->initFromFileName(file)) )
	{
		delete uvd;
		return UV_DEBUG(UV_ERR_GENERAL);
	}	
	g_uvd = uvd;

	uv_assert_ret(uvdOut);
	*uvdOut = uvd;
	return UV_ERR_OK;
}

//Factory function for construction
uv_err_t UVD::getUVDFromData(UVD **uvdOut, UVDData *data)
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
		g_uvd = uvd;
		
		if( UV_FAILED(uvd->initFromData(data)) )
		{
			delete uvd;
			return UV_DEBUG(UV_ERR_GENERAL);
		}
	}

	uv_assert_ret(uvdOut);
	*uvdOut = uvd;
	return UV_ERR_OK;
}

UVDPrintIterator UVD::begin()
{
	UVDPrintIterator iter;
	
	if( UV_FAILED(begin(iter)) )
	{
		UVD_PRINT_STACK();
	}
	return iter;
}

uv_err_t UVD::begin(UVDPrintIterator &iter)
{
	uv_assert_ret(m_runtime);
	uv_assert_ret(m_runtime->m_architecture);
	uv_assert_ret(m_runtime->m_architecture->m_printIteratorFactory);
	uv_assert_err_ret(m_runtime->m_architecture->m_printIteratorFactory->printIteratorBegin(&iter));
	uv_assert_err_ret(iter.check());
	return UV_ERR_OK;
}

UVDPrintIterator UVD::begin(uv_addr_t offset)
{
	UVDPrintIterator iter;

	UV_DEBUG(begin(UVDAddress(offset, NULL), iter));
	return iter;
}

uv_err_t UVD::begin(UVDAddress address, UVDPrintIterator &iter)
{
	if( address.m_space == NULL )
	{
		uv_assert_err_ret(m_runtime->getPrimaryExecutableAddressSpace(&address.m_space));
	}
	return UV_DEBUG(m_runtime->m_architecture->m_printIteratorFactory->printIteratorBeginByAddress(&iter, address));
}

UVDPrintIterator UVD::end()
{
	UVDPrintIterator iter;

	UV_DEBUG(end(iter));
	return iter;
}

uv_err_t UVD::end(UVDPrintIterator &iter)
{
	return UV_DEBUG(m_runtime->m_architecture->m_printIteratorFactory->printIteratorEnd(&iter));
}

UVDInstructionIterator UVD::instructionBegin()
{
	UVDInstructionIterator iter;
	UV_DEBUG(instructionBegin(iter));
	return iter;
}

uv_err_t UVD::instructionBegin(UVDInstructionIterator &iter)
{
	/*
	UVDAddressSpace *addressSpace = NULL;
	
	uv_assert_err_ret(m_runtime->getPrimaryExecutableAddressSpace(&addressSpace));	
	uv_assert_ret(addressSpace);
	uv_assert_err_ret(iter.init(this, addressSpace));

	return UV_ERR_OK;	
	*/
	uv_assert_ret(m_runtime);
	uv_assert_ret(m_runtime->m_architecture);
	uv_assert_ret(m_runtime->m_architecture->m_instructionIteratorFactory);
	return UV_DEBUG(m_runtime->m_architecture->m_instructionIteratorFactory->instructionIteratorBegin(&iter));
}

uv_err_t UVD::instructionBeginByAddress(UVDAddress address, UVDInstructionIterator &iter)
{
	/*
	if( address.m_space == NULL )
	{
		uv_assert_err_ret(m_runtime->getPrimaryExecutableAddressSpace(&address.m_space));
	}
	return UV_DEBUG(iter.init(this, address));
	*/
	uv_assert_ret(address.m_space);
	return UV_DEBUG(m_runtime->m_architecture->m_instructionIteratorFactory->instructionIteratorBeginByAddress(&iter, address));
}

UVDInstructionIterator UVD::instructionEnd()
{
	UVDInstructionIterator iter;

	UV_DEBUG(instructionEnd(iter));
	return iter;
}

uv_err_t UVD::instructionEnd(UVDInstructionIterator &iter)
{
	return UV_DEBUG(m_runtime->m_architecture->m_instructionIteratorFactory->instructionIteratorEnd(&iter));
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
	//eh this is odd
	//I don't think an instruction can print multiple lines
	std::string instructionLines;
	
	uv_assert_ret(inst);
	
	printf_debug("printing disasm\n");
	uv_assert_err_ret(inst->print_disasm(instructionLines));
	printf_debug("printed disasm\n");
	std::vector<std::string> toAppend;

	toAppend = split(instructionLines, '\n', true);
	for( std::vector<std::string>::size_type i = 0; i < toAppend.size(); ++i )
	{
		list.insert(list.end(), toAppend[i]);
	}
	
	return UV_ERR_OK;
}

uv_err_t UVD::disassemble(std::string &output)
{
	//TODO: set config directive here
	return UV_DEBUG(decompile(output));
}

uv_err_t UVD::disassembleByCallback(uvd_string_callback_t callback, void *user)
{
	return UV_DEBUG(decompileByCallback(callback, user));
}

uv_err_t UVD::decompile(std::string &output)
{
	UVDBenchmark decompileBenchmark;
	UVDPrintIterator iterBegin;
	UVDPrintIterator iterEnd;

	output.clear();
	decompileBenchmark.start();

	//Most of program time should be spent here
	uv_assert_err_ret(analyze());
	//Until we can do better
	setDestinationLanguage(UVD_LANGUAGE_ASSEMBLY);
	//And print
	uv_assert_err_ret(begin(iterBegin));
	uv_assert_err_ret(iterBegin.check());
	uv_assert_err_ret(end(iterEnd));
	uv_assert_err_ret(iterEnd.check());
	uv_assert_err_ret(printRangeCore(iterBegin, iterEnd, UVDPrintToStringStringCallback, &output));

	printf_debug_level(UVD_DEBUG_PASSES, "decompile: done\n");
	decompileBenchmark.stop();
	printf_debug_level(UVD_DEBUG_PASSES, "decompile time: %s\n", decompileBenchmark.toString().c_str());
	
	return UV_ERR_OK;
}

uv_err_t UVD::decompileByCallback(uvd_string_callback_t callback, void *user)
{
	UVDBenchmark decompileBenchmark;
	UVDPrintIterator iterBegin;
	UVDPrintIterator iterEnd;

	decompileBenchmark.start();

	//Most of program time should be spent here
	uv_assert_err_ret(analyze());
	//Until we can do better
	setDestinationLanguage(UVD_LANGUAGE_ASSEMBLY);
	//And print
	uv_assert_err_ret(begin(iterBegin));
	uv_assert_err_ret(iterBegin.check());
	uv_assert_err_ret(end(iterEnd));
	uv_assert_err_ret(iterEnd.check());
	uv_assert_err_ret(printRangeCore(iterBegin, iterEnd, callback, user));

	printf_debug_level(UVD_DEBUG_PASSES, "decompile: done\n");
	decompileBenchmark.stop();
	printf_debug_level(UVD_DEBUG_PASSES, "decompile time: %s\n", decompileBenchmark.toString().c_str());
	
	return UV_ERR_OK;
}

uv_err_t UVD::printRange(uv_addr_t start, uv_addr_t end, uint32_t destinationLanguage, std::string &output)
{
	return UV_DEBUG(UV_ERR_NOTIMPLEMENTED);
}

uv_err_t UVD::setDestinationLanguage(uint32_t destinationLanguage)
{
	UVDCompiler *compiler = NULL;

	switch( destinationLanguage )
	{
	case UVD_LANGUAGE_ASSEMBLY:
		compiler = new UVDCompilerAssembly();
		break;
	default:
		printf_error("Unknown destination langauge: 0x%.4X (%d)\n", destinationLanguage, destinationLanguage);
		return UV_DEBUG(UV_ERR_GENERAL);
	};
	uv_assert_ret(compiler);
	//Set a default compiler to generate code for
	//How this is set will probably change drastically in the future
	uv_assert_ret(m_format);
	uv_assert_err_ret(m_format->setCompiler(compiler));
	return UV_ERR_OK;
}

uv_err_t UVD::printRangeCore(UVDPrintIterator iterBegin, UVDPrintIterator iterEnd, uvd_string_callback_t callback, void *user)
{
	UVDPrintIterator iter;
	//UVDPrintIterator iterEnd;
	//int printPercentage = 1;
	//int printNext = printPercentage;
	//uint32_t analyzedBytes = 0;
	int verbose_old = 0;

	uv_assert_err_ret(iterBegin.check());
	uv_assert_err_ret(iterEnd.check());

	uv_assert_ret(m_config);
	//FIXME: this should be delta, not single...w/e
	//uv_assert_err_ret(iterBegin.m_iter->m_address.m_space->getNumberAnalyzedBytes(&analyzedBytes));
	//uv_assert_ret(analyzedBytes != 0);
	verbose_old = m_config->m_verbose;
	m_config->m_verbose = m_config->m_verbose_printing;

	printf_debug_level(UVD_DEBUG_PASSES, "decompile: printing...\n");
	UVDBenchmark decompilePrintBenchmark;
	decompilePrintBenchmark.start();
	uv_assert_err_ret(iter = iterBegin);
	uv_assert_err_ret(iter.check());

	uv_assert_ret(iter.m_iter);
	uv_assert_ret(iterEnd.m_iter);
	{
		UVDStdPrintIterator *pIter = NULL;
		
		pIter = (UVDStdPrintIterator *)iterBegin.m_iter;
		uv_assert_ret(pIter->m_iter.m_iter);

		iter = (UVDStdPrintIterator *)iter.m_iter;
		uv_assert_ret(pIter->m_iter.m_iter);

		pIter = (UVDStdPrintIterator *)iterEnd.m_iter;
		uv_assert_ret(pIter->m_iter.m_iter);
	}


	//Due to the huge number of concatenations
	int iterations = 0;
	//FIXME: what if we misalign by accident and surpass?
	//need to add some check for that
	//maybe we should do <
	while( iter != iterEnd )
	{
		std::string line;
		//uint32_t startPos = iter.m_iter->getPosition();
		
		uv_assert_err_ret(iter.check());
		uv_assert_err_ret(iterEnd.check());
	
		//printf("Main print loop\n");

		++iterations;				
		printf_debug("\n\n\n");

		/*
		int curPercent = 100 * startPos / analyzedBytes;
		if( curPercent >= printNext )
		{
			uint64_t delta = getTimingMicroseconds() - decompilePrintBenchmark.getStart();
			double iterationTime = 1.0 * delta / iterations;
			
			printf_debug_level(UVD_DEBUG_SUMMARY, "uvd: printing: %d %% (us / record: %.2lf)\n", curPercent, iterationTime);
			printNext += printPercentage;
		}
		*/

		uv_assert_err_ret(iter.getCurrent(line));
		//printf_debug("Line (0x%.8X): %s\n", iter.m_iter->getPosition(), line.c_str());

		//This didn't help for the bottleneck under investigation

		uv_assert_err_ret(callback(line + "\n", user));
		
		printf_debug("\n");
		uv_assert_err_ret(iter.next());
	}
	
	/*
	drop for now during architecture abstraction
	if( m_config->m_print_used )
	{
		m_architecture->m_opcodeTable->usedStats();
	}
	*/

	decompilePrintBenchmark.stop();
	printf_debug_level(UVD_DEBUG_PASSES, "decompile print time (%d records): %s\n", iterations, decompilePrintBenchmark.toString().c_str());

	m_config->m_verbose = verbose_old;

	return UV_ERR_OK;
}

uv_err_t UVD::setOutputFormatting(UVDFormat *format)
{
	uv_assert_ret(format);

	//We could consider moving some of the settings in if they are undefined by the new object if an old exists
	delete m_format;
	m_format = format;

	return UV_ERR_OK;
}

UVD *UVDGetUVD()
{
	return g_uvd;
}

