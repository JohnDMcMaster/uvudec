/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/


#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>
#include <sys/stat.h>
#include <vector>
#include <algorithm>
#include "uvd_ascii_art.h"
#include "uvd_debug.h"
#include "uvd_error.h"
#include "uvd_log.h"
#include "uvd_util.h"
#include "uvd.h"
#include "uvd_address.h"
#include "uvd_analysis.h"
#include "uvd_benchmark.h"
#include "uvd_data.h"
#include "uvd_format.h"
#include "uvd_instruction.h"
#include "uvd_types.h"
#include "uvd_config_symbol.h"

UVDIteratorCommon::UVDIteratorCommon()
{
	m_uvd = NULL;
	m_isEnd = FALSE;
	m_currentSize = 0;
}

/*
UVDIteratorCommon::UVDIteratorCommon(UVD *disassembler)
{
	UV_DEBUG(init(disassembler));
}

UVDIteratorCommon::UVDIteratorCommon(UVD *disassembler, uv_addr_t position, uint32_t index)
{
	UV_DEBUG(init(disassembler, position, index));
}
*/

UVDIteratorCommon::~UVDIteratorCommon()
{
	UV_DEBUG(deinit());
}

uv_err_t UVDIteratorCommon::init(UVD *uvd)
{
	uv_addr_t absoluteMinAddress = 0;
	m_isEnd = FALSE;
	m_currentSize = 0;

	m_uvd = uvd;

	uv_assert_ret(m_uvd);
	uv_assert_ret(m_uvd->m_analyzer);
	uv_assert_err_ret(m_uvd->m_analyzer->getAddressMin(&absoluteMinAddress));
	
	return UV_DEBUG(init(uvd, absoluteMinAddress));
}

uv_err_t UVDIteratorCommon::init(UVD *disassembler, uv_addr_t position)
{
	m_isEnd = FALSE;
	m_currentSize = 0;
	m_uvd = disassembler;
	m_nextPosition = position;
	m_initialProcess = FALSE;
	return UV_ERR_OK;
}

uv_err_t UVDIteratorCommon::deinit()
{
	m_data = NULL;
	m_uvd = NULL;
	return UV_ERR_OK;
}

uv_err_t UVDIteratorCommon::makeEnd()
{
	uv_assert_err_ret(makeNextEnd());
	return UV_ERR_OK;
}

uv_err_t UVDIteratorCommon::makeNextEnd()
{
	m_nextPosition = 0;
	m_initialProcess = FALSE;
	m_isEnd = TRUE;
	return UV_ERR_OK;
}

uv_addr_t UVDIteratorCommon::getPosition()
{
	return m_nextPosition;
}

uv_err_t UVDIteratorCommon::next()
{
	uv_err_t rc = UV_ERR_GENERAL;
	//UVDBenchmark nextInstructionBenchmark;
	//uv_addr_t absoluteMaxAddress = 0;
	
	uv_assert_ret(g_config);
	//uv_assert_err_ret(m_uvd->m_analyzer->getAddressMax(&absoluteMaxAddress));	
	//uv_assert_ret(*this != m_uvd->end());

	printf_debug("m_nextPosition: 0x%.8X\n", m_nextPosition);
		
	//Global debug sort of cutoff
	//Force to end
	//This shouldn't happen actually
	/*
	if( m_nextPosition > absoluteMaxAddress )
	{
		uv_assert_err_ret(makeEnd());
		rc = UV_ERR_DONE;
		goto error;
	}
	*/
	//But we should check that we aren't at end otherwise we will loop since end() has address set to 0
	if( m_isEnd )
	{
		rc = UV_ERR_DONE;
		//Although we are flagged for no more instructions, we need to make it bitwise end for iter==end() checks
		uv_assert_err_ret(makeEnd());
		goto error;
	}

	//Otherwise, get next output cluster
	
	//Advance to next position, don't save parsed instruction
	//Maybe we should save parsed instruction to the iter?  Seems important enough
	uv_assert_err_ret(nextCore());
	rc = UV_ERR_OK;
error:
	//nextInstructionBenchmark.stop();
	//printf_debug_level(UVD_DEBUG_PASSES, "next() time: %s\n", nextInstructionBenchmark.toString().c_str());
	return rc;
}

/*
Make it print nicely for output
Any non-printable characters should be converted to some "nicer" form
*/
std::string stringTableStringFormat(const std::string &s)
{
	std::string sRet;
	
	for( std::string::size_type i = 0; i < s.size(); ++i )
	{
		char c = s[i];
		
		if( isprint(c) )
		{
			sRet += c;
		}
		else
		{
			char buff[64];
		
			//<0x0A> form
			snprintf(buff, 64, "<0x%.2X>", (unsigned int)(unsigned char)c);
			sRet += buff;
		}
	}
	return sRet;
}

uv_err_t UVDIteratorCommon::initialProcess()
{
	m_initialProcess = true;
	return UV_ERR_OK;
}

uv_err_t UVDIteratorCommon::clearBuffers()
{
	return UV_ERR_OK;
}

uv_err_t UVDIteratorCommon::nextCore()
{
	/*
	Gets the next logical print group
	These all should be associated with a small peice of data, such as a single instruction
	Ex: an address on line above + call count + disassembled instruction
	*/

	UVD *uvd = NULL;
	UVDAnalyzer *analyzer = NULL;
	UVDFormat *format = NULL;
	//UVDBenchmark nextInstructionBenchmark;
			
	uvd = m_uvd;
	uv_assert_ret(uvd);
	analyzer = uvd->m_analyzer;
	uv_assert_ret(analyzer);
	format = uvd->m_format;
	uv_assert_ret(format);

	uv_assert_ret(g_config);
	
	uv_assert_err_ret(clearBuffers());

	printf_debug("m_nextPosition: 0x%.8X\n", m_nextPosition);

	if( !m_initialProcess )
	{
		uv_err_t rcInitialProcess = UV_ERR_GENERAL;
	
		rcInitialProcess = initialProcess();
		uv_assert_err_ret(rcInitialProcess);	
		uv_assert_ret(m_initialProcess);
		//We may have had all prelim prints disabled
		if( rcInitialProcess == UV_ERR_DONE )
		{
			return UV_ERR_OK;
		}
	}
		
	//nextInstructionBenchmark.start();
	//Currently it seems we do not need to store if the instruction was properly decoded or not
	//this can be caused in a multitude of ways by a multibyte instruction
	if( UV_FAILED(nextInstruction()) )
	{
		printf_debug("Failed to get next instruction\n");
		return UV_DEBUG(UV_ERR_GENERAL);
	}	
	//nextInstructionBenchmark.stop();
	//printf_debug_level(UVD_DEBUG_SUMMARY, "nextInstruction() time: %s\n", nextInstructionBenchmark.toString().c_str());

	return UV_ERR_OK;
}

uv_err_t UVDIteratorCommon::nextValidExecutableAddress()
{
	uv_err_t rcNextAddress = UV_ERR_GENERAL;
	
	//Should this actually be an error?
	if( m_isEnd )
	{
		return UV_ERR_DONE;
	}

	++m_nextPosition;
	++m_currentSize;
	rcNextAddress = m_uvd->m_analyzer->nextValidExecutableAddress(m_nextPosition, &m_nextPosition);
	uv_assert_err_ret(rcNextAddress);
	if( rcNextAddress == UV_ERR_DONE )
	{
		//Don't try to advance further then
		//But we are in middle of decoding, so let caller figure out what to do with buffers
		uv_assert_err_ret(makeNextEnd());
		return UV_ERR_DONE;
	}

	return UV_ERR_OK;
}

uv_err_t UVDIteratorCommon::addWarning(const std::string &lineRaw)
{
	printf_warn("%s\n", lineRaw.c_str());
	return UV_ERR_OK;
}

uv_err_t UVDIteratorCommon::nextInstruction()
{
	/*
	Gets the next logical print group
	These all should be associated with a small peice of data, such as a single instruction
	Ex: an address on line above + call count + disassembled instruction
	Note that we can safely assume the current address is valid, but no addresses after it
	*/
	uv_err_t rcParseOperands = UV_ERR_GENERAL;
	UVDInstructionShared *inst_shared = NULL;
	uv_addr_t startPosition = 0;
	//UVDData *data = m_uvd->m_data;
	UVDData *data = m_data;
	UVDOpcodeLookupElement *element = NULL;
	uint8_t opcode = 0;
	UVD *uvd = NULL;
	uv_addr_t absoluteMaxAddress = 0;
		
	printf_debug("\n");
	printf_debug("m_nextPosition: 0x%.8X\n", m_nextPosition);
	
	//Reduce errors from stale data
	m_instruction = UVDInstruction();
	
	uvd = m_uvd;
	uv_assert_ret(uvd);
	uv_assert_ret(data);
	
	//We are starting a new instruction, reset
	m_currentSize = 0;
	uv_assert_err_ret(m_uvd->m_analyzer->getAddressMax(&absoluteMaxAddress));
	
	//Hmm seems we should never branch here
	//Lets see if we can prove it or at least comment to as why
	uv_assert_ret(m_nextPosition <= absoluteMaxAddress);
	/*
	if( m_nextPosition > absoluteMaxAddress )
	{
		*this = m_uvd->end();
		return UV_ERR_DONE;
	}
	*/
	
	//Used to get delta for copying the data we just iterated over
	startPosition = m_nextPosition;
	
	uv_assert_err_ret(data->readData(m_nextPosition, (char *)&opcode));	
	printf_debug("Just read @ 0x%.8X: 0x%.2X\n", m_nextPosition, opcode);
	
	//Go to next position
	uv_assert_err_ret(nextValidExecutableAddress());
	//printf_debug("post nextValidExecutableAddress: start: 0x%.4X, next time: 0x%.4X and is end: %d\n", startPosition, m_nextPosition, m_isEnd);
	uv_assert_ret(m_nextPosition > startPosition || m_isEnd);
	//If we get UV_ERR_DONE,
	//Of course, if also we require operands, there will be issues
	//But this doesn't mean we can't analyze the current byte
	
	uv_assert_ret(m_uvd->m_opcodeTable);
	uv_assert_err_ret(m_uvd->m_opcodeTable->getElement(opcode, &element));

	if( element == NULL )
	{
		if( m_uvd->m_config->m_haltOnTruncatedInstruction )
		{
			printf_debug("Undefined instruction: 0x%.2X\n", opcode);
			return UV_DEBUG(UV_ERR_GENERAL);
		}
		//XXX add something more descriptive
		uv_assert_err_ret(addWarning("Undefined instruction"));
		return UV_ERR_OK;
	}
	//XXX: this may change in the future to be less direct
	inst_shared = element;
	printf_debug("Memoric: %s\n", inst_shared->m_memoric.c_str());
	
	m_instruction.m_offset = startPosition;
	m_instruction.m_shared = inst_shared;
	m_instruction.m_uvd = m_uvd;

	/*
	Setup extension structs
	There should be a perfect matching between each of these and the shared structs
	*/
	/* Since operand and shared operand structs are linked list, we can setup the entire structure by passing in the first elements */
	rcParseOperands = m_instruction.parseOperands(this, m_instruction.m_shared->m_operands, m_instruction.m_operands);
	uv_assert_err_ret(rcParseOperands);
	//Truncated analysis?
	if( rcParseOperands == UV_ERR_DONE )
	{
		//Did we request a half or should we comment and continue to best of our abilities?
		if( m_uvd->m_config->m_haltOnTruncatedInstruction )
		{
			return UV_DEBUG(UV_ERR_GENERAL);
		}
		//FIXME: add a more usefule error message with offsets, how many bytes short, etc
		uv_assert_err_ret(addWarning("Insufficient data to process instruction"));
		return UV_ERR_OK;
	}

	printf_debug("m_nextPosition, initial: %d, final: %d\n", startPosition, m_nextPosition);

	m_instruction.m_inst_size = m_currentSize;
	//For now these should match
	//XXX However, things like intel opcode extensions bytes will make this check invalid in the future
	if( m_instruction.m_inst_size != m_instruction.m_shared->m_total_length )
	{
		printf_error("Instruction size: %d, shared size: %d\n", m_instruction.m_inst_size, m_instruction.m_shared->m_total_length);
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	
	//We now know the actual size, read the data we just iterated over
	//This will always be valid since we are just storing a shadow copy of the instruction bytes
	//We could alternativly create a UVDData object referring to the source binary file
	uv_assert_err_ret(data->readData(m_instruction.m_offset, m_instruction.m_inst, m_instruction.m_inst_size));

	printf_debug("m_nextPosition, initial: %d, final: %d\n", startPosition, m_nextPosition);

	return UV_ERR_OK;
}	

/*
UVDIterator
*/

UVDIterator::UVDIterator()
{
}

UVDIterator::~UVDIterator()
{
}

uv_err_t UVDIterator::init(UVD *uvd)
{
	uv_assert_err_ret(UVDIteratorCommon::init(uvd));
	m_positionIndex = 0;
	return UV_ERR_OK;
}

uv_err_t UVDIterator::init(UVD *disassembler, uv_addr_t position, uint32_t index)
{
	uv_assert_err_ret(UVDIteratorCommon::init(disassembler, position));
	m_positionIndex = index;
	return UV_ERR_OK;
}

UVDIterator UVDIterator::operator++()
{
	next();
	return *this;
}

bool UVDIterator::operator==(const UVDIterator &other) const
{
	return m_isEnd == other.m_isEnd
			&& m_nextPosition == other.m_nextPosition 
			&& m_positionIndex == other.m_positionIndex
			//Is this check necessary?  Think this was leftover from hackish ways of representing m_isEnd
			//This check is needed for proper ending
			//.end will not have a buffer, but post last address read will (or we are at .end anyway)
			&& m_indexBuffer.size() == other.m_indexBuffer.size();
}

bool UVDIterator::operator!=(const UVDIterator &other) const
{
	return !operator==(other);
}

std::string UVDIterator::operator*()
{
	std::string ret;

	UV_DEBUG(getCurrent(ret));

	return ret;
}

uv_err_t UVDIterator::getCurrent(std::string &s)
{
	//New object not yet initialized?
	printf_debug("Index buffer size: %d\n", m_indexBuffer.size());
	if( m_indexBuffer.empty() )
	{
		uv_assert_ret(!m_isEnd);
		printf_debug("Priming iterator\n");
		if( UV_FAILED(nextCore()) )
		{
			printf_error("Failed to prime iterator!\n");
			return UV_DEBUG(UV_ERR_GENERAL);
		}
	}

	printf_debug("position index: %d\n", m_positionIndex);
	if( m_positionIndex >= m_indexBuffer.size() )
	{
		printf_error("Index out of bounds\n");
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	s = m_indexBuffer[m_positionIndex];
	return UV_ERR_OK;
}

uv_err_t UVDIterator::initialProcessHeader()
{
	char szBuff[256];
	
	//Program info
	snprintf(szBuff, 256, "Generated by uvudec version %s", UVUDEC_VER_STRING);
	uv_assert_err_ret(addComment(szBuff));
	snprintf(szBuff, 256, "uvudec copyright 2009-2010 John McMaster");
	uv_assert_err_ret(addComment(szBuff));
	snprintf(szBuff, 256, "JohnDMcMaster@gmail.com");
	uv_assert_err_ret(addComment(szBuff));
	m_indexBuffer.push_back("");
	m_indexBuffer.push_back("");

	return UV_ERR_OK;
}

uv_err_t UVDIterator::initialProcessUselessASCIIArt()
{
	std::string art = getRandomUVNetASCIIArt() + "\n";
	m_indexBuffer.push_back(art);
	m_indexBuffer.push_back("");
	m_indexBuffer.push_back("");		

	return UV_ERR_OK;
}

uv_err_t UVDIterator::initialProcessStringTable()
{
	char szBuff[256];
	UVDAnalyzer *analyzer = m_uvd->m_analyzer;

	snprintf(szBuff, 256, "# String table:");
	m_indexBuffer.push_back(szBuff);

	uv_assert_ret(m_data);

	for( UVDAnalyzedMemorySpace::iterator iter = analyzer->m_stringAddresses.begin(); iter != analyzer->m_stringAddresses.end(); ++iter )
	{
		UVDAnalyzedMemoryLocation *mem = (*iter).second;
		std::string sData;
		std::vector<std::string> lines;
		
		//Read a string
		m_data->read(mem->m_min_addr, sData, mem->m_max_addr - mem->m_min_addr);

		
		lines = split(sData, '\n', true);
		
		if( lines.size() == 1 )
		{
			snprintf(szBuff, 256, "# 0x%.8X: %s", mem->m_min_addr, stringTableStringFormat(lines[0]).c_str());
			m_indexBuffer.push_back(szBuff);				
		}
		else
		{
			snprintf(szBuff, 256, "# 0x%.8X:", mem->m_min_addr);
			m_indexBuffer.push_back(szBuff);				
			for( std::vector<std::string>::size_type i = 0; i < lines.size(); ++i )
			{
				snprintf(szBuff, 256, "#\t%s", stringTableStringFormat(lines[i]).c_str());
				m_indexBuffer.push_back(szBuff);				
			}
		}
	}
	m_indexBuffer.push_back("");
	m_indexBuffer.push_back("");

	return UV_ERR_OK;
}

uv_err_t UVDIterator::initialProcess()
{
	UVDConfig *config = NULL;
	UVDAnalyzer *analyzer = NULL;

	uv_assert_err_ret(UVDIteratorCommon::initialProcess());
		
	uv_assert_ret(m_uvd);
	config = m_uvd->m_config;
	uv_assert_ret(config);

	uv_assert_ret(m_uvd);
	analyzer = m_uvd->m_analyzer;
	
	if( config->m_print_header )
	{
		uv_assert_err_ret(initialProcessHeader());
	}
	
	if( config->m_uselessASCIIArt )
	{
		uv_assert_err_ret(initialProcessUselessASCIIArt());
	}

	//String table
	if( config->m_print_string_table )
	{
		uv_assert_err_ret(initialProcessStringTable());
	}

	//Don't include instructions in the first batch if we printed stuff
	if( !m_indexBuffer.empty() )
	{
		return UV_ERR_DONE;
	}
	else
	{
		return UV_ERR_OK;
	}
}

uv_err_t UVDIterator::clearBuffers()
{
	m_indexBuffer.clear();
	m_positionIndex = 0;
	return UV_ERR_OK;
}

uv_err_t UVDIterator::makeEnd()
{
	//Like almost at end
	uv_assert_err_ret(UVDIteratorCommon::makeEnd());
	//But without the buffered data to flush
	m_indexBuffer.clear();
	return UV_ERR_OK;
}

uv_err_t UVDIterator::makeNextEnd()
{
	uv_assert_err_ret(UVDIteratorCommon::makeNextEnd());
	m_positionIndex = 0;
	return UV_ERR_OK;
}

/*
void UVDIterator::debugPrint() const
{
	printf_debug("post next, offset: 0x%.8X, index: %d, buffer size: %d\n", m_nextPosition, m_positionIndex, m_indexBuffer.size()); 
}
*/

uv_err_t UVDIterator::next()
{
	++m_positionIndex;

	printf_debug("next(), position index: %d, index buffer size: %d\n", m_positionIndex, m_indexBuffer.size());
	
	//We may have buffered data leftover form last parse
	//Check this before seeing if we are at end
	//Take buffered element if availible
	if( m_positionIndex < m_indexBuffer.size() )
	{
		return UV_ERR_OK;
	}
	
	uv_assert_err_ret(UVDIteratorCommon::next());
	
	return UV_ERR_OK;
}

uv_err_t UVDIterator::nextCore()
{
	uv_err_t rcSuper = UV_ERR_GENERAL;
	UVDBenchmark benchmark;
	uv_addr_t startPosition = m_nextPosition;
	
	//Advance to next instruction location
	rcSuper = UVDIteratorCommon::nextCore();
	uv_assert_err_ret(rcSuper);
	if( rcSuper == UV_ERR_DONE )
	{
		return UV_ERR_OK;
	}

	benchmark.start();
	
	if( g_config->m_addressLabel )
	{
		uv_assert_err_ret(nextAddressLabel(startPosition));
	}

	if( g_config->m_addressComment )
	{
		uv_assert_err_ret(nextAddressComment(startPosition));
	}
	
	if( g_config->m_calledSources )
	{
		uv_assert_err_ret(nextCalledSources(startPosition));
	}
	
	if( g_config->m_jumpedSources )
	{
		uv_assert_err_ret(nextJumpedSources(startPosition));
	}

	//Best to have data follow analysis
	//Instruction is fully parsed now
	//Convert to necessary string values
	uv_assert_err_ret(m_uvd->stringListAppend(&m_instruction, m_indexBuffer));
	printf_debug("Generated string list, size: %d\n", m_indexBuffer.size());

	benchmark.stop();
	//printf_debug_level(UVD_DEBUG_SUMMARY, "other than nextInstruction() time: %s\n", benchmark.toString().c_str());

	return UV_ERR_OK;
}

uv_err_t UVDIterator::printReferenceList(UVDAnalyzedMemoryLocation *memLoc, uint32_t type)
{
	UVDAnalyzedMemoryLocationReferences references;
	char buff[256];
	UVD *uvd = NULL;
	UVDFormat *format = NULL;
		
	uvd = m_uvd;
	uv_assert_ret(uvd);
	format = uvd->m_format;
	uv_assert_ret(format);

	//Get all the locations that call this address
	//FIXME: should this be call source?
	uv_assert_err_ret(memLoc->getReferences(references, type));

	for( UVDAnalyzedMemoryLocationReferences::iterator iter = references.begin(); iter != references.end(); ++iter )
	{
		//uint32_t key = (*iter).first;
		UVDMemoryReference *value = (*iter).second;
		uint32_t from = 0;
		
		uv_assert_err_ret(value);
		from = value->m_from;
		
		snprintf(buff, 256, "#\t%s", format->formatAddress(from).c_str());
		m_indexBuffer.push_back(buff);
	}
	
	return UV_ERR_OK;
}

uv_err_t UVDIterator::addWarning(const std::string &lineRaw)
{
	return UV_DEBUG(addComment(lineRaw));
}

uv_err_t UVDIterator::addComment(const std::string &lineRaw)
{
	std::string lineCommented;
	
	uv_assert_err_ret(m_uvd->m_format->m_compiler->comment(lineRaw, lineCommented));
	m_indexBuffer.push_back(lineCommented);

	return UV_ERR_OK;
}

uv_err_t UVDIterator::nextAddressLabel(uint32_t startPosition)
{
	char buff[256];
	
	//This is like convention adapted by ds52
	//Limit leading zeros by max address size?
	//X00001234:
	snprintf(buff, 256, "X%.8X:", startPosition);
	
	m_indexBuffer.insert(m_indexBuffer.end(), buff);

	return UV_ERR_OK;
}

uv_err_t UVDIterator::nextAddressComment(uint32_t startPosition)
{
	char buff[256];
	
	//0x00001234:
	snprintf(buff, 256, "0x%.8X", startPosition);
	uv_assert_err_ret(addComment(buff));

	return UV_ERR_OK;
}

uv_err_t UVDIterator::nextCalledSources(uint32_t startPosition)
{
	char buff[256];
	std::string sNameBlock;
	UVDAnalyzedFunction analyzedFunction;
	UVDAnalyzedMemoryLocation *memLoc = NULL;
	UVDAnalyzedMemorySpace calledAddresses;

	uv_assert_err_ret(m_uvd->m_analyzer->getCalledAddresses(calledAddresses));
	if( calledAddresses.find(startPosition) == calledAddresses.end() )
	{
		return UV_ERR_OK;
	}

	memLoc = (*(calledAddresses.find(startPosition))).second;
	
	uv_assert_err_ret(m_uvd->analyzeNewFunction(memLoc, analyzedFunction));

	//empty name indicates no data
	if( !analyzedFunction.m_sName.empty() )
	{
		sNameBlock = analyzedFunction.m_sName + "(args?) ";
	}
	
	m_indexBuffer.insert(m_indexBuffer.end(), "\n");
	m_indexBuffer.insert(m_indexBuffer.end(), "\n");
	snprintf(buff, 256, "# FUNCTION START %s@ %s", sNameBlock.c_str(), m_uvd->m_format->formatAddress(startPosition).c_str());
	m_indexBuffer.insert(m_indexBuffer.end(), buff);

	//Print number of callees?
	if( g_config->m_calledCount )
	{
		snprintf(buff, 256, "# References: %d", memLoc->getReferenceCount());
		m_indexBuffer.push_back(buff);
	}

	//Print callees?
	if( g_config->m_calledSources )
	{
		uv_assert_err_ret(printReferenceList(memLoc, UVD_MEMORY_REFERENCE_CALL_DEST));
	}

	return UV_ERR_OK;
}

uv_err_t UVDIterator::nextJumpedSources(uint32_t startPosition)
{
	char buff[256];
	std::string sNameBlock;
	UVDAnalyzedMemoryLocation *memLoc = NULL;
	UVDAnalyzedMemorySpace jumpedAddresses;

	uv_assert_err_ret(m_uvd->m_analyzer->getJumpedAddresses(jumpedAddresses));
	//Can be an entry and continue point
	if( jumpedAddresses.find(startPosition) == jumpedAddresses.end() )
	{
		return UV_ERR_OK;
	}

	memLoc = (*(jumpedAddresses.find(startPosition))).second;
			
	snprintf(buff, 256, "# Jump destination %s@ %s", sNameBlock.c_str(), m_uvd->m_format->formatAddress(startPosition).c_str());
	m_indexBuffer.push_back(buff);

	//Print number of references?
	if( g_config->m_jumpedCount )
	{
		snprintf(buff, 256, "# References: %d", memLoc->getReferenceCount());
		m_indexBuffer.push_back(buff);
	}

	//Print sources?
	if( g_config->m_jumpedSources )
	{
		uv_assert_err_ret(printReferenceList(memLoc, UVD_MEMORY_REFERENCE_JUMP_DEST));
	}

	return UV_ERR_OK;
}

/*
UVDInstructionIterator
*/

UVDInstructionIterator::UVDInstructionIterator()
{
}

UVDInstructionIterator::~UVDInstructionIterator()
{
}

UVDInstructionIterator UVDInstructionIterator::operator++()
{
	next();
	return *this;
}

bool UVDInstructionIterator::operator==(const UVDInstructionIterator &other) const
{
	
	printf_debug("UVDIteratorCommon::operator==:\n");
	//debugPrint();
	//other.debugPrint();

	//Should comapre m_uvd as well?
	return m_isEnd == other.m_isEnd
			&& m_nextPosition == other.m_nextPosition;
}

bool UVDInstructionIterator::operator!=(const UVDInstructionIterator &other) const
{
	return !operator==(other);
}
