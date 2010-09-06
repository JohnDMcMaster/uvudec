/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "flirt/flirt.h"
#include "flirt/pat/pat.h"
#include "flirt/sig/sig.h"
#include "flirt/sig/tree/tree.h"
#include "flirt/sig/format.h"
#include "flirt/sig/writer.h"
#include "uvd_debug.h"
#include "uvd_util.h"
#include <string>

/*
UVDFLIRTSignatureDBConflictSource
*/

UVDFLIRTSignatureDBConflictSource::UVDFLIRTSignatureDBConflictSource()
{
	m_node = NULL;
	m_patFileLine = 0;
}

UVDFLIRTSignatureDBConflictSource::~UVDFLIRTSignatureDBConflictSource()
{
}

/*
UVDFLIRTSignatureDBConflict
*/

UVDFLIRTSignatureDBConflict::UVDFLIRTSignatureDBConflict()
{
}

UVDFLIRTSignatureDBConflict::~UVDFLIRTSignatureDBConflict()
{
}

/*
UVDFLIRTSignatureDBConflicts
FIXME: we are not currently recording conflicts
*/

UVDFLIRTSignatureDBConflicts::UVDFLIRTSignatureDBConflicts()
{
}

UVDFLIRTSignatureDBConflicts::~UVDFLIRTSignatureDBConflicts()
{
}

uv_err_t UVDFLIRTSignatureDBConflicts::writeToFile(const std::string &fileNameIn)
{
	std::string fileContents;
	
	uv_assert_err_ret(writeToString(fileContents));
	uv_assert_err_ret(writeFile(fileNameIn, fileContents));

	return UV_ERR_OK;
}

uv_err_t UVDFLIRTSignatureDBConflicts::writeToString(std::string &out)
{
	return UV_DEBUG(UV_ERR_NOTIMPLEMENTED);
}

uv_err_t UVDFLIRTSignatureDBConflicts::readFromFile(const std::string &fileNameIn, UVDFLIRTSignatureDBConflicts **out)
{
	std::string fileContents;
	
	uv_assert_err_ret(readFile(fileNameIn, fileContents));
	uv_assert_err_ret(readFromString(fileContents, out));

	return UV_ERR_OK;
}

uv_err_t UVDFLIRTSignatureDBConflicts::readFromString(const std::string &in, UVDFLIRTSignatureDBConflicts **out)
{
	return UV_DEBUG(UV_ERR_NOTIMPLEMENTED);
}

/*
UVDPatLoaderCore
*/
class UVDPatLoaderCore
{
public:
	UVDPatLoaderCore(UVDFLIRTSignatureDB *db, const std::string &file = "");
	~UVDPatLoaderCore();
	
	uv_err_t fromString(const std::string &in);
	uv_err_t fileLine(const std::string &in);
	uv_err_t insert(UVDFLIRTFunction *function, UVDFLIRTSignatureRawSequence &leadingBytes);

public:
	//Do not own this
	UVDFLIRTSignatureDB *m_db;
	std::string m_file;
};

UVDPatLoaderCore::UVDPatLoaderCore(UVDFLIRTSignatureDB *db, const std::string &file)
{
	m_db = db;
	m_file = file;
}

UVDPatLoaderCore::~UVDPatLoaderCore()
{
}

uv_err_t UVDPatLoaderCore::fromString(const std::string &in)
{
	std::vector<std::string> lines = split(in, '\n', false);
	
	for( std::vector<std::string>::iterator iter = lines.begin(); ; ++iter )
	{
		std::string line;
		
		if( iter == lines.end() )
		{
			printf_error("ending .pat terminator " UVD_FLIRT_PAT_TERMINATOR " required\n");
			return UV_DEBUG(UV_ERR_GENERAL);
		}
		
		line = *iter;
		
		line = trimString(line);
		if( line.empty() )
		{
			continue;
		}
		if( line == UVD_FLIRT_PAT_TERMINATOR )
		{
			//There should not be any more lines
			uint32_t nonBlank = nonBlankLinesRemaining(lines, iter);
			if( nonBlank > 0 )
			{
				printf_flirt_warning("%s: non blank lines remaining in .pat load: %d\n", m_file, nonBlank);
			}
			break;
		}
		
		//Okay, all the prelims are over, ready to roll
		uv_assert_err_ret(fileLine(line));
	}
	
	return UV_ERR_OK;
}

static bool isValidPatBytePattern(const std::string in)
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

uv_err_t UVDPatLoaderCore::fileLine(const std::string &in)
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
	UVDFLIRTFunction function;
	//UVDFLIRTSignatureRawSequence leadingSignature;
	uint32_t leadingSignatureLength = 0;
	
	printf_flirt_debug("loading FLIRT function from pat line: %s\n", in.c_str());
	
	//function = new UVDFLIRTFunction();
	//uv_assert_ret(function);
	
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
	function.m_crc16Length = strtol(sCRC16Len.c_str(), NULL, 16);

	//"D964"
	sCRC16 = parts[2];
	uv_assert_err_ret(checkPadding(sCRC16, "CRC16", 4));
	function.m_crc16 = strtol(sCRC16.c_str(), NULL, 16);
	
	//"004B"
	sTotalLen = parts[3];
	uv_assert_err_ret(checkPadding(sTotalLen, "total length", 4));
	function.m_totalLength = strtol(sTotalLen.c_str(), NULL, 16);

	printf_flirt_debug("going to load seq\n");
	leadingSignatureLength = uvd_min(function.m_totalLength, g_config->m_flirt.m_patLeadingLength);
	uv_assert_err_ret(function.m_sequence.fromStringCore(sLeadingSignature, leadingSignatureLength));
	printf_flirt_debug("loaded seq from string: %s\n", function.m_sequence.toString().c_str());

	//How to tell between an unnamed symbol at the end and one named ABCD?  See if we have trailing bytes
	uint32_t endIndex = parts.size();
	if( function.m_crc16Length )
	{
		--endIndex;
		sTailingBytes = parts[endIndex];
		if( !isValidPatBytePattern(sTailingBytes) )
		{
			printf_error("Invalid tailing byte pattern (CRC16 len: 0x%.2X): %s\n", function.m_crc16Length, sTailingBytes.c_str());
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
			//The offset that the name was defined in in the original source file is somewhat useless
			//It would only matter if we are trying to print helpful information about resolving conflicts
			std::string publicName;
			
			++i;
			if( i >= endIndex )
			{
				printf_error("public name must be given with public name offset: %s\n", first.c_str());
				return UV_DEBUG(UV_ERR_GENERAL);
			}
			
			publicName = parts[i];
			function.m_publicNames.push_back(publicName);
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
			
			function.m_references.push_back(ref);
		}
		else
		{
			//We already tried to handle the tailing bytes case...so error
			printf_error("unknown token: %s\n", first.c_str());
			return UV_DEBUG(UV_ERR_GENERAL);
		}		
	}
	uv_assert_err_ret(m_db->insert(&function));

	return UV_ERR_OK;
}

/*
UVDFLIRTSignatureDB
*/

UVDFLIRTSignatureDB::UVDFLIRTSignatureDB()
{
	m_flirt = NULL;
	m_data = NULL;
	m_tree = NULL;
	m_conflicts = NULL;
}

UVDFLIRTSignatureDB::~UVDFLIRTSignatureDB()
{
	UV_DEBUG(deinit());
}

uv_err_t UVDFLIRTSignatureDB::init()
{
	m_libraryName = "Unnamed library";
	return UV_ERR_OK;
}

uv_err_t UVDFLIRTSignatureDB::deinit()
{
	delete m_data;
	m_data = NULL;

	delete m_tree;
	m_tree = NULL;

	delete m_conflicts;
	m_conflicts = NULL;

	return UV_ERR_OK;
}

uv_err_t UVDFLIRTSignatureDB::writeToFile(const std::string &file)
{
	UVDData *data = NULL;
	
	uv_assert_err_ret(writeToData(&data));
	uv_assert_err_ret(data->saveToFile(file));

	return UV_ERR_OK;
}

uv_err_t UVDFLIRTSignatureDB::writeToData(UVDData **out)
{
	UVDFLIRTSignatureDBWriter writer(this);
	
	uv_assert_err_ret(writer.constructBinary(out));
	
	return UV_ERR_OK;
}

uv_err_t UVDFLIRTSignatureDB::loadFromPatFile(const std::string &fileNameIn)
{
	std::string fileContents;
	
	uv_assert_err_ret(readFile(fileNameIn, fileContents));
	uv_assert_err_ret(loadFromPatFileString(fileContents));

	return UV_ERR_OK;
}

uv_err_t UVDFLIRTSignatureDB::loadFromPatFileString(const std::string &in)
{
	UVDPatLoaderCore loader(this);
	
	uv_assert_err_ret(loader.fromString(in));
	
	return UV_ERR_OK;
}

uv_err_t UVDFLIRTSignatureDB::insert(UVDFLIRTFunction *function)
{
#if 0
	UVDFLIRTSignatureTreeLeadingNode *rootNode = NULL;

	uv_assert_ret(function);

	//Base case, empty tree or not in prefix
	if( !m_tree )
	{
		rootNode = new UVDFLIRTSignatureTreeLeadingNode();
		m_tree = rootNode;
		//Need to bootstrap the process or it won't know if its an end node or w/e
		uv_assert_err_ret(function->m_leadingSequence.copyTo(&rootNode->m_bytes));
		//uv_assert_ret(rootNode->m_bytes);
	}
	else
	{
		rootNode = m_tree;
	}
	
	uv_assert_err_ret(rootNode->insert(function/*, function->m_leadingSequence.begin()*/));
#endif
	uv_assert_ret(function);

	//Base case, empty tree or not in prefix
	if( !m_tree )
	{
		//Note: root node doesn't have any leading bytes
		m_tree = new UVDFLIRTSignatureTreeLeadingNode();
	}
	
	uv_assert_err_ret(m_tree->insert(function));
	
	return UV_ERR_OK;
}

uv_err_t UVDFLIRTSignatureDB::size(uint32_t *out)
{
	uv_assert_ret(out);
	uv_assert_err_ret(m_tree->size(out));
	return UV_ERR_OK;
}

