/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/hash/crc.h"
#include "uvd/util/util.h"
#include "uvd/flirt/flirt.h"
#include "uvdbfd/flirt/function.h"
#include "uvdbfd/flirt/module_printer.h"
#include "uvdbfd/flirt/relocation.h"
#include "uvdbfd/flirt/section.h"
#include "uvdbfd/flirt/core.h"

#define printDebug() \

#define printDebugIter(iter) \

/*
#define printDebug() \
		printf_flirt_debug("m_iter.m_offset: 0x%.4X, iter reloc offset: 0x%.4X, func offset: 0x%.4X, func size: 0x%.4X\n", m_iter.m_offset, \
		m_iter.m_relocIter == m_func->m_relocations.m_relocations.end() ? 0xFFFF : (*m_iter.m_relocIter)->m_offset, \
		m_func->m_offset, m_func->m_size)

#define printDebugIter(iter) \
		printf_flirt_debug("given iter.m_offset: 0x%.4X, iter reloc offset: 0x%.4X, func offset: 0x%.4X, func size: 0x%.4X\n", iter.m_offset, \
		iter.m_relocIter == m_func->m_relocations.m_relocations.end() ? 0xFFFF : (*iter.m_relocIter)->m_offset)
*/

UVDBFDPatModulePrinter::UVDBFDPatModulePrinter(UVDBFDPatModule *module)
{
	m_module = module;
}

uv_err_t UVDBFDPatModulePrinter::printLeadingBytes()
{
	UVDBFDPatModule::const_iterator endPosIter;
	uint32_t numberLeadingBytes = 0;
	uint32_t endPosRaw = 0;
	uint32_t startSize = 0;
	uint32_t endSize = 0;
	uint32_t expectedChars = 0;
	uint32_t moduleSize = 0;
	uint32_t moduleOffset = 0;
	
	expectedChars = g_config->m_flirt.m_patLeadingLength * 2;	
	
	uv_assert_err_ret(m_module->offset(&moduleOffset));
	uv_assert_err_ret(m_module->size(&moduleSize));
	numberLeadingBytes = uvd_min(moduleSize, g_config->m_flirt.m_patLeadingLength);
	endPosRaw = moduleOffset + numberLeadingBytes;
	printf_flirt_debug("print leading bytes, endPosRaw: 0x%04X\n", endPosRaw);
	endPosIter = m_module->const_offset_begin(endPosRaw);
	//printf_flirt_debug("Leading pattern bytes, m_size: 0x%.4X, config max length: 0x%.4X, selected end pos: 0x%.4X\n", m_size, g_config->m_flirt.m_patLeadingLength, endPos);

	//And let it be born
	//printf_flirt_debug("printing leading bytes, endPosIter.m_offset: 0x%.4X, func offset: 0x%.4X, func size: 0x%.4X\n", endPosIter.m_offset, m_func->m_offset, m_func->m_size);
	startSize = getStringWriter()->m_buffer.size();
	uv_assert_err_ret(printPatternBytes(m_module->const_begin(), endPosIter));
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

UVDStringWriter *UVDBFDPatModulePrinter::getStringWriter()
{
	if( !m_module )
	{
		printf_error("no module, impending crash\n");
	}
	if( !m_module->m_section )
	{
		printf_error("no section, impending crash\n");
	}
	if( !m_module->m_section->m_core )
	{
		printf_error("no core, impending crash\n");
	}
	return &m_module->m_section->m_core->m_writer;
}

uv_err_t UVDBFDPatModulePrinter::printPatternBytes(UVDBFDPatModule::const_iterator startIter, UVDBFDPatModule::const_iterator endIter)
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
	uv_assert_ret(startIter.m_offset <= endIter.m_offset);
	//Each iteration write as much as possible between relocations
	for( UVDBFDPatModule::const_iterator iter = startIter; iter != endIter; )
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

void UVDBFDPatModulePrinter::printRelocationByte()
{
	getStringWriter()->print("%c%c", UVD_FLIRT_PAT_RELOCATION_CHAR, UVD_FLIRT_PAT_RELOCATION_CHAR);
}

uv_err_t UVDBFDPatModulePrinter::nextRelocation(UVDBFDPatModule::const_iterator &out) const
{
	for( out = m_iter; out != m_module->end(); ++out )
	{
		if( (*out).m_relocation )
		{
			break;
		}
	}

	return UV_ERR_OK;
}

uv_err_t UVDBFDPatModulePrinter::printCRC()
{
	UVDBFDPatModule::const_iterator effectiveEndPosition;
	
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
		getStringWriter()->print(" %02X %04X", crcLength, uvd_crc16((char *)(m_module->m_section->m_content + m_iter.sectionOffset()), crcLength));
		//Shift
		m_iter = effectiveEndPosition;
	}
	
	return UV_ERR_OK;
}

static bool isValidPublicName(const std::string &symbolName)
{
	bool ret = false;
	ret = symbolName.size() >= g_config->m_flirt.m_patternPublicNameLengthMin;
	//printf_flirt_debug("check public name validity: %s, %d\n", symbolName.c_str(), ret);
	return ret;
}

uv_err_t UVDBFDPatModulePrinter::printRelocations()
{
	/*
	Recursive functions do not get a second reference
	Function names shorter than 3 chars (1-2) are treated specially
		We need at least 2 to tag them in the module
		They are ommited if any "real" public names are present
	C++ mangled names don't seem to get any special treatment
	If the module has NO public names, its name is ?
		including from short names
	FLAIR does NOT necessarily print references in increasing order
		It would be good to find out when/why it doesn't though in case it hints at a special case we're missing
	Is it possible to have a local unknown name module?
		Probably not
	*/
	
	//FLAIR only emits the first reference location
	std::set<std::string> alreadyPrintedSymbols;
	std::string symbolPrefix;
	std::set<std::string> validPublicNames;
	uint32_t invalidPublicNameCount = 0;

	if( g_config->m_flirt.m_prefixUnderscores )
	{
		symbolPrefix = "_";
	}
	
	for( std::vector<UVDBFDPatFunction *>::iterator iter = m_toPrint.begin();
			iter != m_toPrint.end(); ++iter )
	{
		UVDBFDPatFunction *function = *iter;
		std::string symbolName;
		
		uv_assert_ret(function);
		symbolName = bfd_asymbol_name(function->m_bfdAsymbol);
		if( isValidPublicName(symbolName) )
		{
			validPublicNames.insert(symbolName);
		}
		else
		{
			++invalidPublicNameCount;
		}
	}

	for( std::vector<UVDBFDPatFunction *>::iterator iter = m_toPrint.begin();
			iter != m_toPrint.end(); ++iter )
	{
		uint32_t publicNameOffset = 0;
		uint32_t moduleOffset = 0;
		UVDBFDPatFunction *function = *iter;
		uint32_t flags = 0;
		asymbol *bfdAsymbol = NULL;
		std::string offsetSuffix;
		std::string symbolName;
		std::string publicName;
		
		uv_assert_ret(function);

		uv_assert_err_ret(m_module->offset(&moduleOffset));
		publicNameOffset = function->m_offset - moduleOffset;

		bfdAsymbol = function->m_bfdAsymbol;
		flags = bfdAsymbol->flags;
		//Local referenced names should have a @ suffix
		//weak syms are close enough to local, and certainly aren't global
		if( flags & BSF_LOCAL || flags & BSF_WEAK )
		{
			offsetSuffix += UVD_FLIRT_PAT_LOCAL_SYMBOL_CHAR;
		}
	
		//The symbol name
		//publicNameOffset = m_offset
		symbolName = bfd_asymbol_name(function->m_bfdAsymbol);
		uv_assert_ret(!symbolName.empty());
		alreadyPrintedSymbols.insert(symbolName);

		//Do we have a single unknown public name to print a ? entry?
		if( validPublicNames.empty() && invalidPublicNameCount <= 1 )
		{
			publicName = UVD_FLIRT_PAT_UNKNOWN_PUBLIC_NAME_CHAR;
			uv_assert_ret(publicNameOffset == 0);
		}
		//Skip if wasn't a valid public name AND we have public names
		else if( !validPublicNames.empty() && validPublicNames.find(symbolName) == validPublicNames.end() )
		{
			continue;
		}
		//Standard case, just print the symbol name
		else
		{
			publicName = symbolName;
		}
		
		getStringWriter()->print(" %c%.4X%s %s%s",
				UVD_FLIRT_PAT_PUBLIC_NAME_CHAR, publicNameOffset, offsetSuffix.c_str(),
				symbolPrefix.c_str(), symbolName.c_str());

		//Only print the ? entry if we had a single invalid public name
		if( validPublicNames.empty() && invalidPublicNameCount <= 1 )
		{
			break;
		}
	}
	
	//Dependencies
	//I don't believe its possible to print invalid public names here, they only show up in relocations (if in range)
	for( std::vector<UVDBFDPatRelocation *>::iterator depRelocIter = m_module->m_relocations.m_relocations.begin(); depRelocIter != m_module->m_relocations.m_relocations.end(); ++depRelocIter )
	{
		UVDBFDPatRelocation *depRelocation = *depRelocIter;
		std::string symbolName;
		
		//Not all relocations have names (length 0)
		//these are anonymous and should be skipped
		//They will be taken care of by patternPublicNameLengthMin
		symbolName = depRelocation->m_symbolName;
		if( isValidPublicName(symbolName) && alreadyPrintedSymbols.find(depRelocation->m_symbolName) == alreadyPrintedSymbols.end() )
		{			
			uint32_t relocationOffset = 0;
			
			uv_assert_err_ret(depRelocation->offset(m_module, &relocationOffset));
			getStringWriter()->print(" %c%.4X %s%s",
					UVD_FLIRT_PAT_REFERENCED_NAME_CHAR, relocationOffset, 
					symbolPrefix.c_str(), depRelocation->m_symbolName.c_str());
			alreadyPrintedSymbols.insert(symbolName);
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

uv_err_t UVDBFDPatModulePrinter::printTailingBytes()
{
	//Burn up the rest of the iterator
	//Tail/trailing bytes
	if( m_iter != m_module->const_end() )
	{
		getStringWriter()->print(" ");
		uv_assert_err_ret(printPatternBytes(m_iter, m_module->const_end()));
	}
	else if( g_config->m_flirt.m_patternReferenceTrailingSpace )
	{
		getStringWriter()->print(" ");
	}
	//Not strictly necessary
	m_iter = m_module->const_end();
	return UV_ERR_OK;
}

uv_err_t UVDBFDPatModulePrinter::addToPrint(UVDBFDPatFunction *toPrint)
{
	std::vector<UVDBFDPatFunction *>::iterator iter;
	
	uv_assert_ret(toPrint);
	for( iter = m_toPrint.begin();
			iter != m_toPrint.end(); ++iter )
	{
		UVDBFDPatFunction *cur = *iter;
		uv_assert_ret(cur);
		if( cur->m_offset > toPrint->m_offset )
		{
			break;
		}
	}
	m_toPrint.insert(iter, toPrint);
	return UV_ERR_OK;
}

uv_err_t UVDBFDPatModulePrinter::shouldPrint()
{
	m_toPrint.clear();

	uv_assert_ret(m_module);
	//printf_flirt_debug("module number functions: %d\n", m_module->m_functions.size());
	for( std::vector<UVDBFDPatFunction *>::iterator iter = m_module->m_functions.begin();
			iter != m_module->m_functions.end(); ++iter )
	{
		UVDBFDPatFunction *function = *iter;
		uint32_t flags = 0;
		asymbol *bfdAsymbol = NULL;
		
		uv_assert_ret(function);

		bfdAsymbol = function->m_bfdAsymbol;

		//we must filter here - to don`t lose sizes ! 
		if( std::string(bfdAsymbol->name).empty() )
		{
			printf_flirt_debug("Skipping printing empty symbol\n");
			continue;
		}
		flags = bfdAsymbol->flags;
		/*
		if( (flags & BSF_EXPORT) != BSF_EXPORT )
		{
			printf_flirt_debug("Skipping private (non-exported) symbol %s\n", bfdAsymbol->name);
			return UV_ERR_DONE;
		}
		*/
		if( (flags & BSF_SECTION_SYM) || (flags & BSF_FILE) )
		{
			printf_flirt_debug("Skipping section/file symbol %s\n", bfdAsymbol->name);
			continue;
		}
		//Why is BSF_OBJECT important?
		if( !((flags & BSF_FUNCTION) /*|| (flags & BSF_OBJECT)*/) )
		{
			printf_flirt_debug("Skipping non-funcion symbol %s\n", bfdAsymbol->name);
			continue;
		}

		//Make sure we have enough bytes as per our policy
		if( function->m_size < g_config->m_flirt.m_patSignatureLengthMin )
		{
			printf_flirt_debug("Skipping short len symbol %s, length: 0x%.2X\n", bfdAsymbol->name, function->m_size);
			continue;
		}
	
		//now we have exported function 
		//FIXME: currentSymbol was from prev loop, looks wrong 
		//printf_flirt_debug("Section name %s, size %d, vma = 0x%X\n",
		//	   symbol->section->name, (unsigned int)bfd_section_size(m_bfd, symbol->section), (unsigned int)symbol->section->vma);
		printf_flirt_debug("Name: %s, base %d value 0x%X, flags 0x%X, size 0x%X\n",
				bfd_asymbol_name(bfdAsymbol), (unsigned int)bfd_asymbol_base(bfdAsymbol),
				(unsigned int)bfd_asymbol_value(bfdAsymbol), bfdAsymbol->flags, function->m_size);
		uv_assert_err_ret(addToPrint(function));
	}
	
	if( m_toPrint.empty() )
	{
		return UV_ERR_DONE;
	}

	return UV_ERR_OK;
}

uv_err_t UVDBFDPatModulePrinter::print()
{
	uv_err_t shouldPrintRc = UV_ERR_GENERAL;
	uint32_t moduleSize = 0;
	//Review scoping on this
	//uint32_t effectiveEndPosition = 0;
	
	shouldPrintRc = shouldPrint();
	uv_assert_err_ret(shouldPrintRc);
	if( shouldPrintRc == UV_ERR_DONE )
	{
		return UV_ERR_OK;
	}
	m_iter = m_module->const_begin();
	
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
	uv_assert_err_ret(m_module->size(&moduleSize));
	getStringWriter()->print(" %.4X", moduleSize);
	printf_flirt_debug("\n\nprinting relocations\n");
	printDebug();
	uv_assert_err_ret(printRelocations());
	printf_flirt_debug("\n\nprinting tailing bytes\n");
	printDebug();
	uv_assert_err_ret(printTailingBytes());
	
	getStringWriter()->print(g_config->m_flirt.m_patternFileNewline.c_str());

	printf_flirt_debug("\n\n\n");	
	printf_flirt_debug("buffer:\n%s\n", getStringWriter()->m_buffer.c_str());
	printf_flirt_debug("\n\n\n");	

	return UV_ERR_OK;
}


