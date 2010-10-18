/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>
#include <sys/stat.h>
#include <vector>
#include <algorithm>
#include "uvd/util/ascii_art.h"
#include "uvd/util/debug.h"
#include "uvd/util/error.h"
#include "uvd/util/log.h"
#include "uvd/util/util.h"
#include "uvd/core/uvd.h"
#include "uvd/assembly/address.h"
#include "uvd/core/analysis.h"
#include "uvd/util/benchmark.h"
#include "uvd/data/data.h"
#include "uvd/language/format.h"
#include "uvd/assembly/instruction.h"
#include "uvd/util/types.h"
#include "uvd/architecture/architecture.h"
#include "uvd/core/runtime.h"

UVDIteratorCommon::UVDIteratorCommon()
{
	m_instruction = NULL;
	m_uvd = NULL;
	m_addressSpace = NULL;
	m_nextPosition = 0;
	//m_isEnd = FALSE;
	m_currentSize = 0;
	m_initialProcess = FALSE;
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

uv_err_t UVDIteratorCommon::init(UVD *uvd, UVDAddressSpace *addressSpace)
{
	uv_assert_ret(uvd);
	uv_assert_ret(addressSpace);	
	
	return UV_DEBUG(init(uvd, UVDAddress(addressSpace->m_min_addr, addressSpace)));
}

uv_err_t UVDIteratorCommon::init(UVD *uvd, UVDAddress address)
{
	//m_isEnd = FALSE;
	m_currentSize = 0;
	m_addressSpace = address.m_space;
	uv_assert_ret(m_addressSpace);
	m_uvd = uvd;
	m_nextPosition = address.m_addr;
	m_initialProcess = FALSE;
	uv_assert_err_ret(prime());
	return UV_ERR_OK;
}

uv_err_t UVDIteratorCommon::prime()
{
	uint32_t holdPosition = m_nextPosition;
	uv_err_t rcTemp = UV_ERR_GENERAL;
	
	printf_debug("Priming iterator\n");
	rcTemp = nextValidExecutableAddressIncludingCurrent();
	uv_assert_err_ret(rcTemp);
	//Eh this should be rare
	//We don't prime
	if( rcTemp == UV_ERR_DONE )
	{
		uv_assert_err_ret(makeEnd());
	}
	//This will cause last instruction (which doesn't exist) to be negated
	m_currentSize = 0;
	if( UV_FAILED(next()) )
	{
		printf_error("Failed to prime iterator!\n");
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	m_nextPosition = holdPosition;
	return UV_ERR_OK;
}

uv_err_t UVDIteratorCommon::deinit()
{
	m_addressSpace = NULL;
	m_uvd = NULL;
	return UV_ERR_OK;
}

uv_err_t UVDIteratorCommon::makeEnd()
{
	//uv_assert_err_ret(makeNextEnd());
	//Seems reasonable enough for now
	//Change to invalidating m_data or something later if needed
	m_nextPosition = UINT_MAX;	
	return UV_ERR_OK;
}

bool UVDIteratorCommon::isEnd()
{
	return m_nextPosition == UINT_MAX;
}

/*
uv_err_t UVDIteratorCommon::makeNextEnd()
{
	m_nextPosition = 0;
	m_initialProcess = FALSE;
	m_isEnd = TRUE;
	return UV_ERR_OK;
}
*/

uv_addr_t UVDIteratorCommon::getPosition()
{
	return m_nextPosition;
}

uv_err_t UVDIteratorCommon::next()
{
	//uv_err_t rc = UV_ERR_GENERAL;
	//UVDBenchmark nextInstructionBenchmark;
	//uv_addr_t absoluteMaxAddress = 0;
	
	uv_assert_err_ret(g_config);
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
	/*
	if( m_isEnd )
	{
		rc = UV_ERR_DONE;
		//Although we are flagged for no more instructions, we need to make it bitwise end for iter==end() checks
		uv_assert_err_ret(makeEnd());
		goto error;
	}
	*/

	//Otherwise, get next output cluster
	
	//Advance to next position, don't save parsed instruction
	//Maybe we should save parsed instruction to the iter?  Seems important enough
	uv_assert_err_ret(nextCore());
	return UV_ERR_OK;
//error:
	//nextInstructionBenchmark.stop();
	//printf_debug_level(UVD_DEBUG_PASSES, "next() time: %s\n", nextInstructionBenchmark.toString().c_str());
//	return rc;
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
	uv_err_t rcTemp = UV_ERR_GENERAL;
			
	uvd = m_uvd;
	uv_assert_ret(uvd);
	analyzer = uvd->m_analyzer;
	uv_assert_ret(analyzer);
	format = uvd->m_format;
	uv_assert_ret(format);

	uv_assert_ret(g_config);
	
	uv_assert_err_ret(clearBuffers());

	printf_debug("previous position we are advancing from (m_nextPosition): 0x%.8X\n", m_nextPosition);

	/*
	For printing, there are certain things which may need to be added before the first line
	In the longer term, this should be replaced with checking a special action list indexed by address
	That way there is a clean way to print function starts and such as well
	This should be done with a virtual function like checkSpecialActions() or something
	*/
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
	
	//begin() has special processing that goes directly to nextInstruction()
	//We must go past the current instruction and parse the next
	//Also, be careful that we do not land on a non-executable address
/*
static int count = 0;
++count;
*/
//printf("m_nextPosition: 0x%08X, current size: 0x%08X\n", m_nextPosition, m_currentSize);
//fflush(stdout);
	m_nextPosition += m_currentSize;
//if( count == 5 )
//UVD_BREAK();
	rcTemp = nextValidExecutableAddressIncludingCurrent();
	uv_assert_err_ret(rcTemp);
	if( rcTemp == UV_ERR_DONE )
	{
		uv_assert_err_ret(makeEnd());
		return UV_ERR_DONE;
	}
	
	//nextInstructionBenchmark.start();
	//Currently it seems we do not need to store if the instruction was properly decoded or not
	//this can be caused in a multitude of ways by a multibyte instruction
	if( UV_FAILED(parseCurrentInstruction()) )
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
	/*
	//Should this actually be an error?
	if( m_isEnd )
	{
		return UV_ERR_DONE;
	}
	*/

	//This may put us into an invalid area, but we will find the next valid if availible
	++m_nextPosition;
	return UV_DEBUG(nextValidExecutableAddressIncludingCurrent());
}

uv_err_t UVDIteratorCommon::nextValidExecutableAddressIncludingCurrent()
{
	uv_err_t rcNextAddress = UV_ERR_GENERAL;
	
	//FIXME: look into considerations for an instruction split across areas, which probably doesn't make sense
	uv_assert_ret(m_addressSpace);
	rcNextAddress = m_addressSpace->nextValidExecutableAddress(m_nextPosition, &m_nextPosition);
	uv_assert_err_ret(rcNextAddress);
	if( rcNextAddress == UV_ERR_DONE )
	{
		//Don't do this, we might be partial through an instruction and don't want to mess up address
		//Don't try to advance further then
		//But we are in middle of decoding, so let caller figure out what to do with buffers
		//uv_assert_err_ret(makeNextEnd());
		//uv_assert_err_ret(makeEnd());
		return UV_ERR_DONE;
	}

	return UV_ERR_OK;
}

uv_err_t UVDIteratorCommon::consumeCurrentExecutableAddress(uint8_t *out)
{
	//Current address should always be valid unless we are at end()
	//This is not a hard error because it can happen from malformed opcodes in the input
	if( isEnd() )
	{
		return UV_ERR_DONE;
	}

//UVD_PRINT_STACK();	
	uv_assert_err_ret(m_addressSpace->m_data->readData(m_nextPosition, (char *)out));	
	++m_currentSize;
	//We don't care if next address leads to end
	//Current address was valid and it is up to next call to return done if required
	uv_assert_err_ret(nextValidExecutableAddress());
	
	return UV_ERR_OK;
}

uv_err_t UVDIteratorCommon::addWarning(const std::string &lineRaw)
{
	printf_warn("%s\n", lineRaw.c_str());
	return UV_ERR_OK;
}

uv_err_t UVDIteratorCommon::parseCurrentInstruction()
{
	return UV_DEBUG(m_uvd->m_runtime->m_architecture->parseCurrentInstruction(*this));
}	

/*
UVDIterator
*/

UVDIterator::UVDIterator()
{
	m_positionIndex = 0;
}

UVDIterator::~UVDIterator()
{
}

uv_err_t UVDIterator::init(UVD *uvd, UVDAddressSpace *addressSpace)
{
	uv_assert_err_ret(UVDIteratorCommon::init(uvd, addressSpace));
	m_positionIndex = 0;
	return UV_ERR_OK;
}

uv_err_t UVDIterator::init(UVD *uvd, UVDAddress address, uint32_t index)
{
	uv_assert_err_ret(UVDIteratorCommon::init(uvd, address));
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
	return /*m_isEnd == other.m_isEnd
			&& */m_nextPosition == other.m_nextPosition 
			&& m_positionIndex == other.m_positionIndex;
			//Is this check necessary?  Think this was leftover from hackish ways of representing m_isEnd
			//This check is needed for proper ending
			//.end will not have a buffer, but post last address read will (or we are at .end anyway)
			//&& m_indexBuffer.size() == other.m_indexBuffer.size();
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
	//uv_assert_ret(!m_isEnd);

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
	/*
	Just to be clear: I don't own the copyright for the generated file,
	I'm inserting information to help the user track tool versions
	and to advertise
	*/
	snprintf(szBuff, 256, "Generated by uvudec version %s", UVUDEC_VER_STRING);
	uv_assert_err_ret(addComment(szBuff));
	snprintf(szBuff, 256, "uvudec copyright 2009-2010 John McMaster <JohnDMcMaster@gmail.com>");
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

	uv_assert_ret(m_addressSpace);
	uv_assert_ret(m_addressSpace->m_data);

	for( UVDAnalyzedMemorySpace::iterator iter = analyzer->m_stringAddresses.begin(); iter != analyzer->m_stringAddresses.end(); ++iter )
	{
		UVDAnalyzedMemoryRange *mem = (*iter).second;
		std::string sData;
		std::vector<std::string> lines;
		
		//Read a string
		m_addressSpace->m_data->read(mem->m_min_addr, sData, mem->m_max_addr - mem->m_min_addr);

		
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

uv_err_t UVDIterator::getEnd(UVD *uvd, UVDIterator &iter)
{
	uv_assert_ret(uvd);
	iter.m_uvd = uvd;
	//iter.m_data = uvd->m_data;
	uv_assert_err_ret(iter.makeEnd());
	return UV_ERR_OK;
}

uv_err_t UVDIterator::makeEnd()
{
	//Like almost at end
	uv_assert_err_ret(UVDIteratorCommon::makeEnd());
	//But without the buffered data to flush
	m_indexBuffer.clear();
	m_positionIndex = 0;
	//To try to fix some errors I'm having
	//m_instruction = UVDInstruction();
	return UV_ERR_OK;
}

/*
uv_err_t UVDIterator::makeNextEnd()
{
	uv_assert_err_ret(UVDIteratorCommon::makeNextEnd());
	m_positionIndex = 0;
	return UV_ERR_OK;
}
*/

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
	//uv_err_t rcSuper = UV_ERR_GENERAL;
	UVDBenchmark benchmark;
	uv_addr_t startPosition = m_nextPosition;
	uv_err_t rcTemp = UV_ERR_GENERAL;
	
	benchmark.start();
	
	//Advance to next instruction location
	rcTemp = UVDIteratorCommon::nextCore();
	uv_assert_err_ret(rcTemp);
	if( rcTemp == UV_ERR_DONE )
	{
		return UV_ERR_OK;
	}
	uv_assert_ret(m_instruction->m_inst_size);

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
	uv_assert_err_ret(m_uvd->stringListAppend(m_instruction, m_indexBuffer));
	printf_debug("Generated string list, size: %d\n", m_indexBuffer.size());

	benchmark.stop();
	//printf_debug_level(UVD_DEBUG_SUMMARY, "other than nextInstruction() time: %s\n", benchmark.toString().c_str());

	return UV_ERR_OK;
}

uv_err_t UVDIterator::printReferenceList(UVDAnalyzedMemoryRange *memLoc, uint32_t type)
{
	UVDAnalyzedMemoryRangeReferences references;
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

	for( UVDAnalyzedMemoryRangeReferences::iterator iter = references.begin(); iter != references.end(); ++iter )
	{
		//uint32_t key = (*iter).first;
		UVDMemoryReference *value = (*iter).second;
		uint32_t from = 0;
		
		uv_assert_err_ret(value);
		from = value->m_from;
		
		std::string formattedAddress;
		uv_assert_err_ret(format->formatAddress(from, formattedAddress));
		snprintf(buff, 256, "#\t%s", formattedAddress.c_str());
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
	UVDAnalyzedMemoryRange *memLoc = NULL;
	UVDAnalyzedMemorySpace calledAddresses;

	uv_assert_err_ret(m_uvd->m_analyzer->getCalledAddresses(calledAddresses));
	if( calledAddresses.find(startPosition) == calledAddresses.end() )
	{
		return UV_ERR_OK;
	}

	memLoc = (*(calledAddresses.find(startPosition))).second;
	
	//uv_assert_err_ret(m_uvd->analyzeNewFunction(memLoc, analyzedFunction));

	//empty name indicates no data
	if( !analyzedFunction.m_sName.empty() )
	{
		sNameBlock = analyzedFunction.m_sName + "(args?) ";
	}
	
	m_indexBuffer.insert(m_indexBuffer.end(), "\n");
	m_indexBuffer.insert(m_indexBuffer.end(), "\n");
	std::string formattedAddress;
	uv_assert_err_ret(m_uvd->m_format->formatAddress(startPosition, formattedAddress));
	snprintf(buff, 256, "# FUNCTION START %s@ %s", sNameBlock.c_str(), formattedAddress.c_str());
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
	UVDAnalyzedMemoryRange *memLoc = NULL;
	UVDAnalyzedMemorySpace jumpedAddresses;

	uv_assert_err_ret(m_uvd->m_analyzer->getJumpedAddresses(jumpedAddresses));
	//Can be an entry and continue point
	if( jumpedAddresses.find(startPosition) == jumpedAddresses.end() )
	{
		return UV_ERR_OK;
	}

	memLoc = (*(jumpedAddresses.find(startPosition))).second;
			
	std::string formattedAddress;
	uv_assert_err_ret(m_uvd->m_format->formatAddress(startPosition, formattedAddress));
	snprintf(buff, 256, "# Jump destination %s@ %s", sNameBlock.c_str(), formattedAddress.c_str());
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
	return /*m_isEnd == other.m_isEnd
			&& */m_nextPosition == other.m_nextPosition;
}

bool UVDInstructionIterator::operator!=(const UVDInstructionIterator &other) const
{
	return !operator==(other);
}

uv_err_t UVDInstructionIterator::getEnd(UVD *uvd, UVDInstructionIterator &iter)
{
	uv_assert_ret(uvd);
	iter.m_uvd = uvd;
	uv_assert_err_ret(iter.makeEnd());
	return UV_ERR_OK;
}

