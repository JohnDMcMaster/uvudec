/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd_crc.h"
#include "uvd_util.h"
#include "uvd_flirt.h"
#include "uvd_flirt_pattern_bfd_function.h"
#include "uvd_flirt_pattern_bfd_relocation.h"
#include "uvd_flirt_pattern_bfd_section.h"
#include "uvd_flirt_pattern_bfd_core.h"

/*
UVDBFDPatFunction
*/

UVDBFDPatFunction::UVDBFDPatFunction()
{
	m_bfdAsymbol = NULL;
	m_relocations.m_function = this;
	m_section = NULL;
	m_offset = 0;
	m_size = 0;
}

UVDBFDPatFunction::~UVDBFDPatFunction()
{
}

//Returns the first invalid index
uv_err_t UVDBFDPatFunction::getEffectiveEndPosition(uint32_t *endPos)
{
	//TODO: find out what IDA does if we have a relocation on the end
	//I don't think we should trim off end relocations as even an ending wildcard IS part of a match
	*endPos = m_size;
	/*
	//No relocations implies we end when its over
	if( m_relocations.m_relocations.empty() )
	{
		*endPos = m_size;
	}
	else
	{
		//last relocation pos gives us only an estimate...seems useless
		*endPos = m_relocations.m_relocations[m_relocations.m_relocations.size() - 1]->m_offset;
	}
	*/
	return UV_ERR_OK;
}

uint8_t UVDBFDPatFunction::readByte(uint32_t offset)
{
	//Offset into section + offset within section
	return m_section->m_content[m_offset + offset];
}

UVDStringWritter *UVDBFDPatFunction::getStringWritter()
{
	return &m_section->m_core->m_writter;
}

uv_err_t UVDBFDPatFunction::printLeadingBytes()
{
	uint32_t endPos = 0;
	
	endPos = uvd_min(m_size, g_config->m_flirt.m_patLeadingLength);
	printf_flirt_debug("Leading pattern bytes, m_size: 0x%.4X, length: 0x%.4X\n", m_size, g_config->m_flirt.m_patLeadingLength);
	//And let it be born
	uv_assert_err_ret(printPatternBytes(0, endPos));
	return UV_ERR_OK;
}

uv_err_t UVDBFDPatFunction::printPatternBytes(uint32_t start, uint32_t end)
{
	/*
	TODO: instead of buffering, inch along relocations as needed
	Original code did this, but it was somewhat hard to follow and this was easier
	*/
	//max sig len nibbles + null terminator
	char leadingBytesBuff[0x1000];
	char buff[16];
	uv_addr_t curAddress = 0;
	
	printf_flirt_debug("start: 0x%.8X, end: 0x%.8X, m_size: 0x%.8X\n", start, end, m_size);
	fflush(stdout);
	
	uv_assert_ret(start < end);
	uv_assert_ret(start < m_size);
	//Inclusive, so can be equal
	uv_assert_ret(end <= m_size);
	//Leave room for terminating byte
	uv_assert_ret(end * 2 < sizeof(leadingBytesBuff) - 1);

	
	//Fill in the definied bytes
	//We will override relocations as needed next
	curAddress = start;
	for( uint32_t i = 0; ; ++i, ++curAddress )
	{
		if( curAddress >= end )
		{
			leadingBytesBuff[2 * i] = 0;
			break;
		}

		sprintf(buff, "%.2X", readByte(i));
		leadingBytesBuff[2 * i] = buff[0];
		leadingBytesBuff[2 * i + 1] = buff[1];
	}
	
	//Print relocation bytes as needed
	for( std::vector<UVDBFDPatRelocation *>::iterator relocIter = m_relocations.m_relocations.begin(); relocIter != m_relocations.m_relocations.end(); ++relocIter )
	{
		UVDBFDPatRelocation *reloc = *relocIter;
		//Relative to function
		uint32_t relocationAddress = reloc->m_address - m_offset;
		uv_addr_t curAddress = relocationAddress;
		
		//All relocations are not necessarily in this range
		//Avoid nasty overflow condition
		if( relocationAddress >= start )
		{
			printf_flirt_debug("Beginning to print relocations\n");
			printf_flirt_debug("relocation in range, reloc addr: 0%.4X, print start: 0x%.4X\n", relocationAddress, start);
			//Hmm we should just do a memset
			for( uint32_t i = (relocationAddress - start) * 2; curAddress < relocationAddress + reloc->m_size; i += 2, ++curAddress )
			{
				printf_flirt_debug("relocation i: 0x%.4X\n", i);
				printf_flirt_debug("start: 0x%.4X, end: 0x%.4X\n", start, end);
				leadingBytesBuff[i] = UVD_FLIRT_PAT_RELOCATION_CHAR;
				leadingBytesBuff[i + 1] = UVD_FLIRT_PAT_RELOCATION_CHAR;
			}
		}
		else
		{
			printf_flirt_debug("skipping relocation b/c not in range, reloc addr: 0%.4X, print start: 0x%.4X\n", relocationAddress, start);
		}
	}
	
	getStringWritter()->print("%s", &leadingBytesBuff[0]);

	printf_flirt_debug("pattern leading wrote\n");
	fflush(stdout);
	return UV_ERR_OK;
}

void UVDBFDPatFunction::printRelocationByte()
{
	getStringWritter()->print("%c%c", UVD_FLIRT_PAT_RELOCATION_CHAR, UVD_FLIRT_PAT_RELOCATION_CHAR);
}

uv_err_t UVDBFDPatFunction::printCRC()
{
	uint32_t effectiveEndPosition = 0;
	
	uv_assert_err_ret(getEffectiveEndPosition(&effectiveEndPosition));

	//length of trailing data and the CRC of that data
	//printf_flirt_debug("pos 0x%X, endPosition 0x%X\n", pos, endPosition);
	//Do we not have any trailing data?
	//pos now holds how many bytes were in the first part
	/*
	There was some weird logic here
	It might be b/c if our trailing data is ONLY relocations, we don't compute the crc16
	Guess relocations should be 0'd for computing the CRC16
	*/
	if( effectiveEndPosition <= g_config->m_flirt.m_patLeadingLength )
	{
		getStringWritter()->print(" 00 0000");
	}
	else
	{
		uint32_t crcLength = 0;
		
		crcLength = effectiveEndPosition - g_config->m_flirt.m_patLeadingLength;
		printf_flirt_debug("CRC length 0x%02X\n", crcLength);
		if( crcLength > 0xff )
		{
			crcLength = 0xff;
		}
		getStringWritter()->print(" %02X %04X", crcLength, uvd_crc16((char *)(m_section->m_content + g_config->m_flirt.m_patLeadingLength), crcLength));
	}
	
	return UV_ERR_OK;
}

uv_err_t UVDBFDPatFunction::printRelocations()
{
	//The symbol name
	//hmm this shouldn't be :0000, thats only valid if its at the start of the library?
	getStringWritter()->print(" %04X", m_size);
	getStringWritter()->print(" %c%.4X %s", UVD_FLIRT_PAT_PUBLIC_NAME_CHAR, m_offset, bfd_asymbol_name(m_bfdAsymbol));
	
	//Dependencies
	for( std::vector<UVDBFDPatRelocation *>::iterator depRelocIter = m_relocations.m_relocations.begin(); depRelocIter != m_relocations.m_relocations.end(); ++depRelocIter )
	{
		UVDBFDPatRelocation *depRelocation = *depRelocIter;
		
		//Not all relocations have names...blank if doesn't have a name?
		getStringWritter()->print(" %c%04X", UVD_FLIRT_PAT_REFERENCED_NAME_CHAR, depRelocation->m_offset, depRelocation->m_symbolName.c_str());
		//Many anoymous labels and other unnamed symbols
		if( !depRelocation->m_symbolName.empty() )
		{
			getStringWritter()->print(" %s", depRelocation->m_symbolName.c_str());
		}
	}
	
	return UV_ERR_OK;
}

uv_err_t UVDBFDPatFunction::shouldPrintFunction()
{
	uint32_t flags = 0;

	//we must filter here - to don`t lose sizes ! 
	if( std::string(m_bfdAsymbol->name).empty() )
	{
		printf_flirt_debug("Skipping printing empty symbol\n");
		return UV_ERR_DONE;
	}
	flags = m_bfdAsymbol->flags;
	if( BSF_EXPORT != (flags & BSF_EXPORT) )
	{
		printf_flirt_debug("Skipping private (non-exported) symbol %s\n", m_bfdAsymbol->name);
		return UV_ERR_DONE;
	}
	if( (flags & BSF_SECTION_SYM) || (flags & BSF_FILE) )
	{
		printf_flirt_debug("Skipping section/file symbol %s\n", m_bfdAsymbol->name);
		return UV_ERR_DONE;
	}
	//Why is BSF_OBJECT important?
	if( !((flags & BSF_FUNCTION) /*|| (flags & BSF_OBJECT)*/) )
	{
		printf_flirt_debug("Skipping non-funcion symbol %s\n", m_bfdAsymbol->name);
		return UV_ERR_DONE;
	}
	//now we have exported function 
	//FIXME: currentSymbol was from prev loop, looks wrong 
	//printf_flirt_debug("Section name %s, size %d, vma = 0x%X\n",
	//	   symbol->section->name, (unsigned int)bfd_section_size(m_bfd, symbol->section), (unsigned int)symbol->section->vma);
	printf_flirt_debug("Name: %s, base %d value 0x%X, flags 0x%X, size 0x%X\n",
			bfd_asymbol_name(m_bfdAsymbol), (unsigned int)bfd_asymbol_base(m_bfdAsymbol),
			(unsigned int)bfd_asymbol_value(m_bfdAsymbol), m_bfdAsymbol->flags, m_size);

	return UV_ERR_OK;
}

uv_err_t UVDBFDPatFunction::print()
{
	uv_err_t shouldPrintRc = UV_ERR_GENERAL;
	//Review scoping on this
	uint32_t effectiveEndPosition = 0;
	
	shouldPrintRc = shouldPrintFunction();
	uv_assert_err_ret(shouldPrintRc);
	if( shouldPrintRc == UV_ERR_DONE )
	{
		return UV_ERR_OK;
	}
	
	//Make sure we have enough bytes as per our policy
	if( m_size < g_config->m_flirt.m_patSignatureLengthMin )
	{
		printf_flirt_debug("Skipping short len symbol %s\n", m_bfdAsymbol->name);
		return UV_ERR_OK;
	}
	
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
	uv_assert_err_ret(getEffectiveEndPosition(&effectiveEndPosition));
	
	/*
	Ex:
	128BEC8BAB404D1E3F787....02007406B8050012EB141EB43F8B5E048B4E0AC5 0B B56E 002F :0000 __read ^000B __openfd ^002C __IOERROR ....5DC3
														crc dat len -/\  /\   /\    /\- defined sym
																	crc- /  total len
	*/
	
	uv_assert_err_ret(printLeadingBytes());
	uv_assert_err_ret(printCRC());
	uv_assert_err_ret(printRelocations());		

	//Tail/trailing bytes
	if( effectiveEndPosition > g_config->m_flirt.m_patLeadingLength )
	{
		getStringWritter()->print(" ");
		uv_assert_err_ret(printPatternBytes(g_config->m_flirt.m_patLeadingLength, effectiveEndPosition));
	}
		
	getStringWritter()->print("\n");

	printf_flirt_debug("\n\n\n");	
	printf_flirt_debug("buffer:\n%s\n", getStringWritter()->m_buffer.c_str());
	printf_flirt_debug("\n\n\n");	

	return UV_ERR_OK;
}

/*
UVDBFDPatFunctions
*/

UVDBFDPatFunctions::UVDBFDPatFunctions()
{
}

UVDBFDPatFunctions::~UVDBFDPatFunctions()
{
	for( std::vector<UVDBFDPatFunction *>::iterator iter = m_functions.begin();
			iter != m_functions.end(); ++iter )
	{
		delete *iter;
	}
	m_functions.clear();
}

uv_err_t UVDBFDPatFunctions::add(UVDBFDPatFunction *function)
{
	std::vector<UVDBFDPatFunction *>::iterator iter;
	
	uv_assert_ret(function->m_section);
	//Sorted by function offset
	for( iter = m_functions.begin(); iter != m_functions.end(); ++iter )
	{
		UVDBFDPatFunction *curFunction = NULL;
		
		curFunction = *iter;
		uv_assert_ret(function != curFunction);
		if( function->m_offset < curFunction->m_offset )
		{
			break;
		}
	}
	m_functions.insert(iter, function);
	return UV_ERR_OK;
}

