/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd_crc.h"
#include "uvd_util.h"
#include "flirt/flirt.h"
#include "flirt/pat/bfd/function.h"
#include "flirt/pat/bfd/function_printer.h"
#include "flirt/pat/bfd/relocation.h"
#include "flirt/pat/bfd/section.h"
#include "flirt/pat/bfd/core.h"

#define printDebug() \
		printf_flirt_debug("m_iter.m_offset: 0x%.4X, iter reloc offset: 0x%.4X, func offset: 0x%.4X, func size: 0x%.4X\n", m_iter.m_offset, \
		m_iter.m_relocIter == m_func->m_relocations.m_relocations.end() ? 0xFFFF : (*m_iter.m_relocIter)->m_offset, \
		m_func->m_offset, m_func->m_size)

#define printDebugIter(iter) \
		printf_flirt_debug("given iter.m_offset: 0x%.4X, iter reloc offset: 0x%.4X, func offset: 0x%.4X, func size: 0x%.4X\n", iter.m_offset, \
		iter.m_relocIter == m_func->m_relocations.m_relocations.end() ? 0xFFFF : (*iter.m_relocIter)->m_offset)

UVDBFDPatFunctionPrinter::UVDBFDPatFunctionPrinter(UVDBFDPatFunction *func)
{
	m_func = func;
}

uv_err_t UVDBFDPatFunctionPrinter::printLeadingBytes()
{
	UVDBFDPatFunction::const_iterator endPosIter;
	uint32_t endPosRaw = 0;
	uint32_t startSize = 0;
	uint32_t endSize = 0;
	uint32_t expectedChars = 0;
	
	expectedChars = g_config->m_flirt.m_patLeadingLength * 2;	
	
	endPosRaw = uvd_min(m_func->m_size, g_config->m_flirt.m_patLeadingLength);
	endPosIter = m_func->const_offset_begin(endPosRaw);
	//printf_flirt_debug("Leading pattern bytes, m_size: 0x%.4X, config max length: 0x%.4X, selected end pos: 0x%.4X\n", m_size, g_config->m_flirt.m_patLeadingLength, endPos);

	//And let it be born
	printf_flirt_debug("printing leading bytes, endPosIter.m_offset: 0x%.4X, func offset: 0x%.4X, func size: 0x%.4X\n", endPosIter.m_offset, m_func->m_offset, m_func->m_size);
	startSize = getStringWriter()->m_buffer.size();
	uv_assert_err_ret(printPatternBytes(m_func->const_begin(), endPosIter));
	endSize = getStringWriter()->m_buffer.size();
	uv_assert_ret(endSize - startSize <= expectedChars);
	//Pad trailing dots
	for( uint32_t i = endSize - startSize; i < expectedChars; i += 2 )
	{
		getStringWriter()->print("%c%c", UVD_FLIRT_PAT_RELOCATION_CHAR, UVD_FLIRT_PAT_RELOCATION_CHAR);
	}
	//Recalc, should be fixed at 0x20
	endSize = getStringWriter()->m_buffer.size();
	uv_assert_ret(endSize - startSize == expectedChars);
	
	//Shift
	printDebug();
	printDebugIter(endPosIter);
	m_iter = endPosIter;
	printDebug();

	return UV_ERR_OK;
}

UVDStringWriter *UVDBFDPatFunctionPrinter::getStringWriter()
{
	return &m_func->m_section->m_core->m_writer;
}

uv_err_t UVDBFDPatFunctionPrinter::printPatternBytes(UVDBFDPatFunction::const_iterator startIter, UVDBFDPatFunction::const_iterator endIter)
{
	/*
	TODO: instead of buffering, inch along relocations as needed
	Original code did this, but it was somewhat hard to follow and this was easier
	*/
	//max sig len nibbles + null terminator
	//char leadingBytesBuff[0x1000];
	//char buff[16];
	//Relative to function start
	//uv_addr_t curAddress = 0;
	
	//printf_flirt_debug("start: 0x%.8X, end: 0x%.8X, m_size: 0x%.8X\n", start, end, m_size);
	
	//uv_assert_ret(start < end);
	//uv_assert_ret(start < m_size);
	//Inclusive, so can be equal
	//uv_assert_ret(end <= m_size);
	//Leave room for terminating byte
	//printf_flirt_debug("end: 0x%.8X, sizeof(leadingBytesBuff): 0x%.8X\n", end, sizeof(leadingBytesBuff));
	//uv_assert_ret(end * 2 < sizeof(leadingBytesBuff) - 1);
	
	//Fill in the definied bytes
	//We will override relocations as needed next
	//curAddress = start;
	//curIter = startIter;
	//Print relocation bytes as needed
	//std::vector<UVDBFDPatRelocation *>::iterator relocIter = m_relocations.m_relocations.begin();
	
	//Skip relocations until we are in range
	/*
	FIXME: make sure we handle splitting a relocation across leading and tailing bytes
	*/
	//printf_flirt_debug("printing pattern bytes, number relocations: %d\n", m_relocations.m_relocations.size());
	
	printf_flirt_debug("end iter: 0x%.8X\n", endIter.m_offset);	
	//Each iteration write as much as possible between relocations
	for( UVDBFDPatFunction::const_iterator iter = startIter; iter != endIter; )
	{
		printf_flirt_debug("iterating print, iter.m_offset: 0x%.4X, endIter.m_offset: 0x%.4X\n", iter.m_offset, endIter.m_offset);
		//If we have a relocation, fill it in
		if( (*iter).m_relocation )
		{
			getStringWriter()->print("%c%c", UVD_FLIRT_PAT_RELOCATION_CHAR, UVD_FLIRT_PAT_RELOCATION_CHAR);
		}
		else
		{
			getStringWriter()->print("%.2X", (*iter).m_byte);
		}
		uv_assert_err_ret(iter.next());
		printDebugIter(iter);
	}
	
	printf_flirt_debug("pattern leading wrote\n");
	return UV_ERR_OK;
}

void UVDBFDPatFunctionPrinter::printRelocationByte()
{
	getStringWriter()->print("%c%c", UVD_FLIRT_PAT_RELOCATION_CHAR, UVD_FLIRT_PAT_RELOCATION_CHAR);
}

uv_err_t UVDBFDPatFunctionPrinter::nextRelocation(UVDBFDPatFunction::const_iterator &out) const
{
	for( out = m_iter; out != m_func->end(); ++out )
	{
		if( (*out).m_relocation )
		{
			break;
		}
	}

	return UV_ERR_OK;
}

uv_err_t UVDBFDPatFunctionPrinter::printCRC()
{
	UVDBFDPatFunction::const_iterator effectiveEndPosition;
	
	uv_assert_err_ret(nextRelocation(effectiveEndPosition));
	//No progress?  Means nothing to print
	if( effectiveEndPosition == m_iter )
	{
		getStringWriter()->print(" 00 0000");
	}
	else
	{
		uint32_t crcLength = 0;
		
		crcLength = effectiveEndPosition.difference(m_iter);
		if( crcLength > UVD_FLIRT_PAT_CRC_LEN_MAX )
		{
			crcLength = UVD_FLIRT_PAT_CRC_LEN_MAX;
			//effectiveEndPosition = m_iter + UVD_FLIRT_PAT_CRC_LEN_MAX;
			effectiveEndPosition = m_iter;
			effectiveEndPosition.advance(UVD_FLIRT_PAT_CRC_LEN_MAX);
		}
		printf_flirt_debug("CRC length 0x%02X\n", crcLength);
		getStringWriter()->print(" %02X %04X", crcLength, uvd_crc16((char *)(m_func->m_section->m_content + m_iter.sectionOffset()), crcLength));
		//Shift
		m_iter = effectiveEndPosition;
	}
	
	return UV_ERR_OK;
}

uv_err_t UVDBFDPatFunctionPrinter::printRelocations()
{
	uint32_t publicNameOffset = 0;
	//FLAIR only emits the first reference location
	std::set<std::string> alreadyPrintedSymbols;
	uint32_t flags = 0;
	asymbol *bfdAsymbol = NULL;
	std::string offsetSuffix;
	std::string symbolPrefix;

	bfdAsymbol = m_func->m_bfdAsymbol;
	flags = bfdAsymbol->flags;
	//Local referenced names should have a @ suffix
	//weak syms are close enough to local, and certainly aren't global
	if( flags & BSF_LOCAL || flags & BSF_WEAK )
	{
		offsetSuffix = "@";
	}
	
	if( g_config->m_flirt.m_prefixUnderscores )
	{
		symbolPrefix = "_";
	}
	
	//The symbol name
	//FIXME: there are a lot of special cases are are skipping
	//Can .pat files have more than one :0000 entry?
	//yes they can
	//ex: 5589E583EC208B4508D975E483F8FF746F83F8FE0F849E0000000FB7100FB74D A0 98D8 00C0 :0000 ___fesetenv :0000@ _fesetenv 
	//Or should they always be one per line?
	//Seems like its allowed, but probably somewhat specialized what will directly generate it in a .pat
	//publicNameOffset = m_offset
	getStringWriter()->print(" %c%.4X%s %s%s",
			UVD_FLIRT_PAT_PUBLIC_NAME_CHAR, publicNameOffset, offsetSuffix.c_str(),
			symbolPrefix.c_str(), bfd_asymbol_name(m_func->m_bfdAsymbol));
	
	//Dependencies
	for( std::vector<UVDBFDPatRelocation *>::iterator depRelocIter = m_func->m_relocations.m_relocations.begin(); depRelocIter != m_func->m_relocations.m_relocations.end(); ++depRelocIter )
	{
		UVDBFDPatRelocation *depRelocation = *depRelocIter;
		
		//Not all relocations have names
		//these are anonymous and should be skipped
		if( !depRelocation->m_symbolName.empty() && alreadyPrintedSymbols.find(depRelocation->m_symbolName) == alreadyPrintedSymbols.end() )
		{			
			getStringWriter()->print(" %c%.4X %s%s",
					UVD_FLIRT_PAT_REFERENCED_NAME_CHAR, depRelocation->m_offset, 
					symbolPrefix.c_str(), depRelocation->m_symbolName.c_str());
			alreadyPrintedSymbols.insert(depRelocation->m_symbolName);
		}
		/*
		else
		{
			printf_flirt_debug("unnamed relocation for function %s, offset 0x%.4X\n", bfd_asymbol_name(m_func->m_bfdAsymbol), depRelocation->m_offset);
			uv_assert_ret(!depRelocation->m_symbolName.empty());
		}
		*/
	}
	
	return UV_ERR_OK;
}

uv_err_t UVDBFDPatFunctionPrinter::printTailingBytes()
{
	//Burn up the rest of the iterator
	//Tail/trailing bytes
	if( m_iter != m_func->const_end() )
	{
		getStringWriter()->print(" ");
		uv_assert_err_ret(printPatternBytes(m_iter, m_func->const_end()));
	}
	//Not strictly necessary
	m_iter = m_func->const_end();
	return UV_ERR_OK;
}

uv_err_t UVDBFDPatFunctionPrinter::shouldPrintFunction()
{
	uint32_t flags = 0;
	asymbol *bfdAsymbol = NULL;

	bfdAsymbol = m_func->m_bfdAsymbol;

	//we must filter here - to don`t lose sizes ! 
	if( std::string(bfdAsymbol->name).empty() )
	{
		printf_flirt_debug("Skipping printing empty symbol\n");
		return UV_ERR_DONE;
	}
	flags = bfdAsymbol->flags;
	if( BSF_EXPORT != (flags & BSF_EXPORT) )
	{
		printf_flirt_debug("Skipping private (non-exported) symbol %s\n", bfdAsymbol->name);
		return UV_ERR_DONE;
	}
	if( (flags & BSF_SECTION_SYM) || (flags & BSF_FILE) )
	{
		printf_flirt_debug("Skipping section/file symbol %s\n", bfdAsymbol->name);
		return UV_ERR_DONE;
	}
	//Why is BSF_OBJECT important?
	if( !((flags & BSF_FUNCTION) /*|| (flags & BSF_OBJECT)*/) )
	{
		printf_flirt_debug("Skipping non-funcion symbol %s\n", bfdAsymbol->name);
		return UV_ERR_DONE;
	}

	//Make sure we have enough bytes as per our policy
	if( m_func->m_size < g_config->m_flirt.m_patSignatureLengthMin )
	{
		printf_flirt_debug("Skipping short len symbol %s\n", bfdAsymbol->name);
		return UV_ERR_DONE;
	}
	
	//now we have exported function 
	//FIXME: currentSymbol was from prev loop, looks wrong 
	//printf_flirt_debug("Section name %s, size %d, vma = 0x%X\n",
	//	   symbol->section->name, (unsigned int)bfd_section_size(m_bfd, symbol->section), (unsigned int)symbol->section->vma);
	printf_flirt_debug("Name: %s, base %d value 0x%X, flags 0x%X, size 0x%X\n",
			bfd_asymbol_name(bfdAsymbol), (unsigned int)bfd_asymbol_base(bfdAsymbol),
			(unsigned int)bfd_asymbol_value(bfdAsymbol), bfdAsymbol->flags, m_func->m_size);

	return UV_ERR_OK;
}

uv_err_t UVDBFDPatFunctionPrinter::print()
{
	uv_err_t shouldPrintRc = UV_ERR_GENERAL;
	//Review scoping on this
	//uint32_t effectiveEndPosition = 0;
	
	shouldPrintRc = shouldPrintFunction();
	uv_assert_err_ret(shouldPrintRc);
	if( shouldPrintRc == UV_ERR_DONE )
	{
		return UV_ERR_OK;
	}
	m_iter = m_func->const_begin();
	
	/*
	//For debugging
	if( std::string(m_bfdAsymbol->name) != "main" )
	{
		printf_flirt_debug("skipping non main symbol: %s\n", m_bfdAsymbol->name);
		return UV_ERR_OK;
	}
	*/
	
	printf_flirt_debug("\n\n\nPRINT_START\n");	
	
	//Figure out where in the section our function ends
	//This will be important for the trailing part of the signature
	//To trim off relocations on the end
	//uv_assert_err_ret(getEffectiveEndPosition(&effectiveEndPosition));
	
	/*
	Ex:
	128BEC8BAB404D1E3F787....02007406B8050012EB141EB43F8B5E048B4E0AC5 0B B56E 002F :0000 __read ^000B __openfd ^002C __IOERROR ....5DC3
														crc dat len -/\  /\   /\    /\- defined sym
																	crc- /  total len
	*/
	
	printf_flirt_debug("\n\nprinting leading bytes\n");
	uv_assert_err_ret(printLeadingBytes());
	printf_flirt_debug("\n\nprinting CRC\n");
	printDebug();
	uv_assert_err_ret(printCRC());
	//Total length
	getStringWriter()->print(" %.4X", m_func->m_size);
	printf_flirt_debug("\n\nprinting relocations\n");
	printDebug();
	uv_assert_err_ret(printRelocations());
	printf_flirt_debug("\n\nprinting tailing bytes\n");
	printDebug();
	uv_assert_err_ret(printTailingBytes());
	
	getStringWriter()->print("\n");

	printf_flirt_debug("\n\n\n");	
	printf_flirt_debug("buffer:\n%s\n", getStringWriter()->m_buffer.c_str());
	printf_flirt_debug("\n\n\n");	

	return UV_ERR_OK;
}


