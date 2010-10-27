/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/config/config.h"
#include "uvd/hash/crc.h"
#include "uvd/flirt/pat/pat.h"
#include "uvd/flirt/lib/lib.h"
#include "uvd/flirt/function.h"
#include "uvd/util/util.h"

static uv_err_t checkPadding(const std::string &in, const std::string &printName, uint32_t expectedSize)
{
	/*
	Consider adding config directives to control the strictness of this processing
	Hackers are lazy sobs
	*/
	
	//Maybe too many leading 0's?  Punish them anyway
	if( in.size() > expectedSize )
	{
		printf_error("%s invalid (size > %d): %s\n", printName.c_str(), expectedSize, in.c_str());
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	if( in.size() != expectedSize )
	{
		printf_flirt_warning("%s should be 0 padded to size %d, found: %s\n", printName.c_str(), expectedSize, in.c_str()); 
	}
	return UV_ERR_OK;
}

UVDFLIRTPatternGenerator::UVDFLIRTPatternGenerator()
{
}

UVDFLIRTPatternGenerator::~UVDFLIRTPatternGenerator()
{
	UV_DEBUG(deinit());
}

uv_err_t UVDFLIRTPatternGenerator::deinit()
{
	return UV_ERR_OK;
}

uv_err_t UVDFLIRTPatternGenerator::saveToFile(UVDObject *object, const std::string &file)
{
	std::string output;
	
	uv_assert_err_ret(saveToString(object, output, true));
	uv_assert_err_ret(writeFile(file, output));
	
	return UV_ERR_OK;
}

uv_err_t UVDFLIRTPatternGenerator::saveToString(UVDObject *object, std::string &output, uint32_t terminateFile)
{
	uv_assert_err_ret(saveToStringCore(object, output));
	
	if( terminateFile )
	{
		output += UVD_FLIRT_PAT_TERMINATOR + g_config->m_flirt.m_patternFileNewline;
	}

	return UV_ERR_OK;
}

uv_err_t UVDFLIRTPatternGenerator::patLineToFunction(const std::string &in, UVDFLIRTModule *function)
{
	/*
	ex:
	5589E583EC18E8........C1E81F84C07426C744240C........C74424085800 2B D964 004B :015F _Z14initProgConfigv ^0007 _Z21initFLIRTSharedConfigv ^0016 ^0026 ^0032 uv_err_ret_handler ^0039 g_config ^0040 5589E583EC18........FFC1E81F84C07426........630100........085800........240406000000C7
	*/

	//In order of required and/or usual appearance
	std::vector<std::string> parts;
	std::string sLeadingSignature;
	std::string sCRC16Len;
	std::string sCRC16;
	std::string sTotalLen;
	//std::vector<std::string> publicNames;	
	//std::vector<std::string> referencedNames;
	std::string sTailingBytes;
	//UVDFLIRTSignatureRawSequence leadingSignature;
	uint32_t leadingSignatureLength = 0;
	
	//This is too verbose for general testing
	//printf_flirt_debug("loading FLIRT function from pat line: %s\n", in.c_str());
	
	uv_assert_ret(function);
	
	//Split it up and then parse it
	parts = split(in, ' ', false);
	
	if( parts.size() < 6 )
	{
		printf_error("line truncated, need at least 6 parts, found %d\n", parts.size());
		return UV_DEBUG(UV_ERR_GENERAL);
	}

	//"5589E583EC18E8........C1E81F84C07426C744240C........C74424085800"
	sLeadingSignature = parts[0];

	//"2B"
	sCRC16Len = parts[1];	
	uv_assert_err_ret(checkPadding(sCRC16Len, "CRC16 length", 2));
	function->m_crc16Length = strtol(sCRC16Len.c_str(), NULL, 16);

	//"D964"
	sCRC16 = parts[2];
	uv_assert_err_ret(checkPadding(sCRC16, "CRC16", 4));
	function->m_crc16 = strtol(sCRC16.c_str(), NULL, 16);
	
	//"004B"
	sTotalLen = parts[3];
	uv_assert_err_ret(checkPadding(sTotalLen, "total length", 4));
	function->m_totalLength = strtol(sTotalLen.c_str(), NULL, 16);

	printf_flirt_debug("going to load seq\n");
	leadingSignatureLength = uvd_min(function->m_totalLength, g_config->m_flirt.m_patLeadingLength);
	uv_assert_err_ret(function->m_sequence.fromStringCore(sLeadingSignature, leadingSignatureLength));
	printf_flirt_debug("loaded seq from string: %s\n", function->m_sequence.toString().c_str());

	//How to tell between an unnamed symbol at the end and one named ABCD?  See if we have trailing bytes
	uint32_t endIndex = parts.size();
	if( leadingSignatureLength + function->m_crc16Length < function->m_totalLength )
	{
		--endIndex;
		sTailingBytes = parts[endIndex];
		if( !isValidPatBytePattern(sTailingBytes) )
		{
			printf_error("Invalid tailing byte pattern (CRC16 len: 0x%.2X): %s\n", function->m_crc16Length, sTailingBytes.c_str());
			return UV_DEBUG(UV_ERR_GENERAL);
		}
	}
	for( uint32_t i = 4; i < endIndex; ++i )
	{
		const std::string &first = parts[i];
	
		/*
		Public name
		:015F _Z14initProgConfigv
		Relocation (referenced name, symbol)
		^0007 _Z21initFLIRTSharedConfigv
		Relocation (anonymous: jump target, etc)
		^0016 ^0026
		*/
		if( first[0] == UVD_FLIRT_PAT_PUBLIC_NAME_CHAR )
		{
			std::string name;
			uint32_t offset = 0;
			
			//We might not necessarily have to clean this up, but it improves reliability/portability of code
			//for now though, just be lazy and don't filter out @s and stuff at the end
			std::string offsetOnly = first.substr(1, std::string::npos);
			offset = strtol(offsetOnly.c_str(), NULL, 16);
			
			++i;
			if( i >= endIndex )
			{
				printf_error("public name must be given with public name offset: %s\n", first.c_str());
				return UV_DEBUG(UV_ERR_GENERAL);
			}
			
			name = parts[i];
			function->m_publicNames.push_back(UVDFLIRTPublicName(name, offset));
		}
		else if( first[0] == UVD_FLIRT_PAT_REFERENCED_NAME_CHAR )
		{
			UVDFLIRTSignatureReference ref;
			std::string sOffset;
			
			++i;
			if( i < endIndex )
			{
				ref.m_name = parts[i];
			}
			
			sOffset = first.substr(1);
			uv_assert_err_ret(checkPadding(sOffset, "offset", 4));
			ref.m_offset = strtol(sOffset.c_str(), NULL, 16);
			//eh need to know more about how to set these
			//they should be modeled after .sig file values
			ref.m_attributeFlags = 0;
			
			function->m_references.push_back(ref);
		}
		else
		{
			//We already tried to handle the tailing bytes case...so error
			printf_error("unknown token: %s\n", first.c_str());
			return UV_DEBUG(UV_ERR_GENERAL);
		}		
	}

	return UV_ERR_OK;
}

bool UVDFLIRTPatternGenerator::isValidPatBytePattern(const std::string in)
{
	for( std::string::size_type i = 0; i < in.size(); ++i )
	{
		char c = in[i];
		if( !(c == UVD_FLIRT_PAT_RELOCATION_CHAR
				|| (c >= '0' && c <= '9')
				|| (c >= 'A' && c <= 'F')
				|| (c >= 'a' && c <= 'f')) )
		{
			return false;
		}
	}
	return true;
}

