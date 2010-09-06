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

uv_err_t UVDPatLoaderCore::fileLine(const std::string &in)
{
	UVDFLIRTFunction function;

	uv_assert_err_ret(UVDFLIRTPatternGenerator::patLineToFunction(in, &function));
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
	
	uv_assert_ret(!file.empty());
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

