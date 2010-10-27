/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/flirt/pat/reader.h"
#include "uvd/flirt/sig/sig.h"
#include "uvd/flirt/function.h"
#include "uvd/util/util.h"

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
		uv_assert_err_ret(m_db->debugDumpTree());
	}
	
	return UV_ERR_OK;
}

uv_err_t UVDPatLoaderCore::fileLine(const std::string &in)
{
	UVDFLIRTModule function;

	uv_assert_err_ret(UVDFLIRTPatternGenerator::patLineToFunction(in, &function));
	uv_assert_err_ret(m_db->insert(&function));

	return UV_ERR_OK;
}

