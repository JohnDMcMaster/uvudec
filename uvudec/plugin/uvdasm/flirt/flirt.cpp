/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifdef UVD_FLIRT_PATTERN_UVD

#include "flirt/pat/pat.h"
#include "flirt/pat/uvd.h"
#include "uvd/config/config.h"
#include "uvd/hash/crc.h"
#include "uvd_library.h"
#include "uvd/util/util.h"

/*
*/

UVDFLIRTPatternEntry::UVDFLIRTPatternEntry()
{
}

UVDFLIRTPatternEntry::~UVDFLIRTPatternEntry()
{
}

uv_err_t UVDFLIRTPatternEntry::deinit()
{
	delete m_data;
	m_data = NULL;

	return UV_ERR_OK;
}

/*
*/

UVDFLIRTPatternAnalysisUVD::UVDFLIRTPatternAnalysisUVD()
{
	m_library = NULL;
}

UVDFLIRTPatternAnalysisUVD::~UVDFLIRTPatternAnalysisUVD()
{
	deinit();
}

uv_err_t UVDFLIRTPatternAnalysisUVD::deinit()
{
	for( std::vector<UVDFLIRTPatternEntry* >::iterator iter = m_entries.begin(); iter != m_entries.end(); ++iter )
	{
		delete *iter;
	}
	m_entries.clear();

	return UV_ERR_OK;
}

uv_err_t UVDFLIRTPatternAnalysisUVD::saveToString(std::string &output)
{
	/*
	This is documented in the FLAIR toolkit, see pat.txt.  In summary...
	558BEC8B5E04D1E3F787....02007406B8050050EB141EB43F8B5E048B4E0AC5 0B B56E 002F :0000 __read ^000B __openfd ^002C __IOERROR ....5DC3
	pppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppp ll ssss LLLL gggggggggggg rrrrrrrrrrrrrrrrrrrrrrrrrrrrrr tttttttt

    Module essentially refers to a function within a library
    
    p - PATTERN BYTES (64 positions)
        space
    l - 2 positions contain ALEN (example:12)
        space
    s - 4 positions contain ASUM (example:1234)
        space
    L - 4 positions contain TOTAL LENGTH OF MODULE IN BYTES (example:1234)
        space
    g - LIST OF PUBLIC NAMES
    r - LIST OF REFERENCED NAMES
    t - TAIL BYTES
	*/
	
	//Begining signature
	uint32_t signatureLengthMin = g_config->m_flirt.m_patSignatureLengthMin;
	std::string lineEnding = g_config->m_flirt.m_patternFileNewline;
	//From pat doc
	const uint32_t maxModuleLength = 0x8000;
	uint32_t moduleLength = 0;
	
	uv_assert_ret(m_library);
	uv_assert_ret(m_library->m_data);
	uv_assert_err_ret(m_library->m_data->size(&moduleLength));
	if( moduleLength > maxModuleLength )
	{
		printf_error("really big library (>0x%X)!  Cannot save data\n", maxModuleLength);
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	
	output = "";

	//One line per function
	for( std::vector<UVDFLIRTPatternEntry *>::iterator iter = m_entries.begin(); iter != m_entries.end(); ++iter )
	{
		UVDFLIRTPatternEntry *entry = *iter;
		std::string sEntry;
		//char **rawData = NULL;
		uint32_t rawDataSize = 0;
		/*
		Pat doc calls this list of "public names", ie names we export this symbol as.  Most likely one
		XXXX: offset in module
		:XXXX name
		*/
		UVDBinarySymbol *symbol = NULL;
		UVDData *data = NULL;
		
		//We are a bit careful here, but during subroutines don't do error check
		uv_assert_ret(entry);
		symbol = entry->m_symbol;
		uv_assert_ret(symbol);
		data = symbol->m_data;
		uv_assert_ret(data);
		
		uv_assert_err_ret(data->size(&rawDataSize));
		
		//Under special circumstances we take these, but usually ignore them
		//For example, FLAIR claims it will take short sigs if they contain relocations
		if( rawDataSize < signatureLengthMin )
		{
			continue;
		}
		
		//uv_assert_err_ret(data->readData(&rawData));
		
		std::string temp;
		
		uv_assert_err_ret(getPatLeadingSignature(entry, temp));
		sEntry += temp;
		uv_assert_err_ret(getPatCRC(entry, temp));
		sEntry += temp;
		uv_assert_err_ret(getPatPublicNames(entry, temp));
		sEntry += temp;
		uv_assert_err_ret(getPatReferencedNames(entry, temp));
		sEntry += temp;
		uv_assert_err_ret(getPatTailBytes(entry, temp));
		sEntry += temp;
		
		sEntry += lineEnding;
		output += sEntry;
		
		//free(rawData);
	}	
	
	output += "---" + lineEnding;

	return UV_ERR_OK;
}

uv_err_t UVDFLIRTPatternAnalysisUVD::getPatLeadingSignature(UVDFLIRTPatternEntry *entry, std::string &out)
{
	uint32_t signatureLengthMax = g_config->m_flirt.m_patSignatureLengthMax;
	char buff[0x100];
	//FIXME: this should use signatureLengthMax
	//32 byte start seq (64 hex chars)
	//char startSequenceBuff[64];
	char *startSequenceBuff = NULL;
	UVDBinarySymbol *symbol = entry->m_symbol;
	UVDData *data = symbol->m_data;
	uint32_t rawDataSize = 0;

	out = "";
	startSequenceBuff = (char *)malloc(signatureLengthMax * 2);
	uv_assert_ret(startSequenceBuff);

	uv_assert_err_ret(data->size(&rawDataSize));

	//Collect the beginning signature and then relocate out as needed
	uint32_t dataPos;
	for( dataPos = 0; dataPos < rawDataSize && dataPos < signatureLengthMax; ++dataPos )
	{
		char curByte = 0;
		uv_assert_err_ret(data->readData(dataPos, &curByte));

		snprintf(buff, sizeof(buff), "%.2X", curByte);
		startSequenceBuff[dataPos * 2] = buff[0];
		startSequenceBuff[dataPos * 2 + 1] = buff[1];
	}
	//Fill up remaining bytes with wildcard '.'
	for( ; dataPos < signatureLengthMax; ++dataPos )
	{
		startSequenceBuff[dataPos * 2] = '.';
		startSequenceBuff[dataPos * 2 + 1] = '.';
	}
	//And convert to a string
	out += std::string(startSequenceBuff, sizeof(startSequenceBuff));

	free(startSequenceBuff);

	return UV_ERR_OK;
}

uv_err_t UVDFLIRTPatternAnalysisUVD::getPatCRC(UVDFLIRTPatternEntry *entry, std::string &out)
{
	uint32_t signatureLengthMax = g_config->m_flirt.m_patSignatureLengthMax;
	//Compute CRC on remaining data up to 0xFF bytes
	uint32_t crcLength = 0;
	uint32_t crc = 0;
	char buff[0x100];
	UVDBinarySymbol *symbol = entry->m_symbol;
	UVDData *data = symbol->m_data;
	uint32_t rawDataSize = 0;
	
	out = "";
	uv_assert_err_ret(data->size(&rawDataSize));

	//FIXME: make this more generic later
	uv_assert_ret(rawDataSize <= sizeof(buff));
	
	if( rawDataSize > signatureLengthMax )
	{
		//We can compute on a max of 0xFF bytes
		crcLength = rawDataSize - signatureLengthMax;
		if( crcLength > 0xFF )
		{
			crcLength = 0xFF;
		}
		uv_assert_err_ret(data->readData(signatureLengthMax, &buff[0], rawDataSize));
	}
	
	crc = uvd_crc16(&buff[0] + signatureLengthMax, crcLength);

	snprintf(buff, sizeof(buff), " %.2X %.4X", crcLength, crc);
	out += buff;

	return UV_ERR_OK;
}

uv_err_t UVDFLIRTPatternAnalysisUVD::getPatPublicNames(UVDFLIRTPatternEntry *entry, std::string &out)
{
	char buff[0x100];
	UVDBinarySymbol *symbol = entry->m_symbol;
	uint32_t size = 0;
	
	out = "";
	
	//Collect public names
	//Assume only one for now...diff object 
	std::set<std::string> symbolNames;
	uv_assert_err_ret(symbol->getSymbolNames(symbolNames));
	uv_assert_err_ret(symbol->m_data->size(&size));
	/*
	If no public names: :0000 ?
	*/
	for( std::set<std::string>::iterator iter = symbolNames.begin(); iter != symbolNames.end(); ++iter )
	{
		std::string name = *iter;
		snprintf(buff, sizeof(buff), " :%.4X %s", size, name.c_str());
		out += buff;
		
		/*
		XXX add me
		if( is local (ie static in C) )
		{
			output += '@';
		}
		*/
	}

	return UV_ERR_OK;
}

uv_err_t UVDFLIRTPatternAnalysisUVD::getPatReferencedNames(UVDFLIRTPatternEntry *entry, std::string &out)
{
	char buff[0x100];
	UVDRelocatableData *relocatableData = entry->m_symbol->m_relocatableData;

	out = "";
	uv_assert_ret(relocatableData);

	//And collect the relocation strings
	for( std::set<UVDRelocationFixup *>::iterator iter = relocatableData->m_fixups.begin();
			iter != relocatableData->m_fixups.end(); ++iter )
	{
		UVDRelocationFixup *fixup = *iter;
		//uint32_t relocationSize = 0;
		uint32_t relocationOffset = 0;
		std::string relocationName;
		
		uv_assert_ret(fixup);
		//Looks like this only tells where they start, now how big
		//uv_assert_err_ret(fixup->getSizeBytes(&relocationSize));
		uv_assert_err_ret(fixup->getOffset(&relocationOffset));
		uv_assert_err_ret(fixup->m_symbol->getName(relocationName));
		
		snprintf(buff, sizeof(buff), " ^%.4X %s", relocationOffset, relocationName.c_str());
		out += buff;
	}

	return UV_ERR_OK;
}

uv_err_t UVDFLIRTPatternAnalysisUVD::getPatTailBytes(UVDFLIRTPatternEntry *entry, std::string &out)
{
	char buff[0x100];
	uint32_t signatureLengthMax = g_config->m_flirt.m_patSignatureLengthMax;
	UVDBinarySymbol *symbol = entry->m_symbol;
	UVDData *data = symbol->m_data;
	UVDRelocatableData *relocatableData = symbol->m_relocatableData;
	uint32_t rawDataSize = 0;

	out = "";
	uv_assert_ret(relocatableData);
	uv_assert_err_ret(data->size(&rawDataSize));

	if( signatureLengthMax >= rawDataSize )
	{
		return UV_ERR_OK;
	}
	
	std::string trailingData;
	out += ".";
	for( uint32_t dataPos = signatureLengthMax; dataPos < rawDataSize; ++dataPos )
	{
		char curByte = 0;
		uv_assert_err_ret(data->readData(dataPos, &curByte));

		snprintf(buff, sizeof(buff), "%.2X", curByte);
		trailingData += buff;
	}
	//Apply relocations now, remembering we are dealing with a nibble array
	//maybe we should move this to the other relocation pass
	for( std::set<UVDRelocationFixup *>::iterator iter = relocatableData->m_fixups.begin();
			iter != relocatableData->m_fixups.end(); ++iter )
	{
		UVDRelocationFixup *fixup = *iter;
		uv_assert_ret(fixup);
		
		//Does this patch belong to the early part only?
		if( fixup->m_offset + fixup->getSizeBytes() <= signatureLengthMax )
		{
			continue;
		}
		
		//Do we need to skip ahead a bit?
		uint32_t iMin = 0;
		if( fixup->m_offset < signatureLengthMax )
		{
			//How many bytes short are we?  We need to advance that much
			iMin = signatureLengthMax - fixup->m_offset;
		}
		for( uint32_t i = iMin; i < fixup->getSizeBytes(); ++i )
		{
			uint32_t base = ((fixup->m_offset - signatureLengthMax + i) * 2) * 2;
			uv_assert_ret(base < trailingData.size());
			trailingData[base] = '.';
			trailingData[base + 1] = '.';
		}
	}

	return UV_ERR_OK;
}

#endif
