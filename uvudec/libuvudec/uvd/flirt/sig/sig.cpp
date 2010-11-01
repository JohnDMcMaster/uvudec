/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/flirt/args_property.h"
#include "uvd/flirt/flirt.h"
#include "uvd/flirt/pat/pat.h"
#include "uvd/flirt/pat/reader.h"
#include "uvd/flirt/sig/reader.h"
#include "uvd/flirt/sig/sig.h"
#include "uvd/flirt/sig/tree/tree.h"
#include "uvd/flirt/sig/format.h"
#include "uvd/flirt/sig/writer.h"
#include "uvd/util/debug.h"
#include "uvd/util/util.h"
#include <string>
#include <string.h>

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
UVDFLIRTSignatureDB
*/

UVDFLIRTSignatureDB::UVDFLIRTSignatureDB()
{
	m_flirt = NULL;
	m_data = NULL;
	m_tree = NULL;
	m_conflicts = NULL;
	memset(&m_header, 0, sizeof(struct UVD_IDA_sig_header_t));
}

UVDFLIRTSignatureDB::~UVDFLIRTSignatureDB()
{
	UV_DEBUG(deinit());
}

uv_err_t UVDFLIRTSignatureDB::init()
{
	m_libraryName = g_config->m_flirt.m_libName;

	m_header.version = g_config->m_flirt.m_sigVersion;
	m_header.feature_flags = g_config->m_flirt.m_sigFeatures;
	m_header.pad = g_config->m_flirt.m_sigPad;
	m_header.processor = g_config->m_flirt.m_sigProcessorID;
	m_header.OS_types = g_config->m_flirt.m_sigOSTypes;
	m_header.app_types = g_config->m_flirt.m_sigAppTypes;
	m_header.file_types = g_config->m_flirt.m_sigFileTypes;

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

uv_err_t UVDFLIRTSignatureDB::insert(UVDFLIRTModule *function)
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
		uv_assert_ret(m_tree);
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

uv_err_t UVDFLIRTSignatureDB::loadSigFile(const std::string &fileNameIn)
{
	UVDFLIRTSigReader sigReader(this);
	
	uv_assert_err_ret(sigReader.load(fileNameIn));
	
	return UV_ERR_OK;
}


static std::string hexstr(const char *in, int sz)
{
	std::string ret;
	for( int i = 0; i < sz; ++i)
	{
		char buff[3];
		sprintf(buff, "%02X", in[i]);
		ret += buff;
	}
	return ret;
}

static std::string safestr(const char *in, int sz)
{
	std::string ret;
	
	for( int i = 0; i < sz; ++i )
	{
		if( in[i] == 0)
		{
			break;
		}
		if( isprint(in[i]) )
		{
			ret += in[i];
		}
		else
		{
			ret += '.';
		}
	}
	return ret;
}

uv_err_t UVDFLIRTSignatureDB::dump()
{
	printf("magic: %s\n", hexstr(m_header.magic, sizeof(m_header.magic)).c_str());	
	printf("version: %d\n", m_header.version);
	printf("processor: %s (0x%02X)\n", UVDIDASigArchToString(m_header.processor).c_str(), m_header.processor);
	printf("file_types: %s (0x%08X)\n", UVDIDASigFileToString(m_header.file_types).c_str(), m_header.file_types);
	printf("OS_types: %s (0x%04X)\n", UVDIDASigOSToString(m_header.OS_types).c_str(), m_header.OS_types);
	printf("app_types: %s (0x%04X)\n", UVDIDASigApplicationToString(m_header.app_types).c_str(), m_header.app_types);
	printf("feature_flags: %s (0x%02X)\n", UVDIDASigFeaturesToString(m_header.feature_flags).c_str(), m_header.feature_flags);
	printf("unknown (pad): 0x%02X\n", m_header.pad);
	printf("old_number_modules: 0x%04X\n", m_header.old_number_modules);
	printf("crc16: 0x%04X\n", m_header.crc16);	
	//Make sure its null terminated
	printf("ctype: %s\n", safestr(m_header.ctype, sizeof(m_header.ctype)).c_str());	
	printf("library_name_sz: 0x%02X\n", m_header.library_name_sz);	
	printf("alt_ctype_crc: 0x%04X\n", m_header.alt_ctype_crc);	
	printf("n_modules: 0x%08X (%d)\n", m_header.n_modules, m_header.n_modules);

	//Name is immediatly after header
	printf("library name: %s\n", m_libraryName.c_str());
		
	if( m_tree )
	{
		uv_assert_err_ret(m_tree->dump("", false));
	}
	
	return UV_ERR_OK;
}

uv_err_t UVDFLIRTSignatureDB::debugDumpTree()
{
	if( !UVDGetDebugFlag(UVD_DEBUG_TYPE_FLIRT) )
	{
		return UV_ERR_OK;
	}

	if( m_tree )
	{
		printf_flirt_debug("Signature DB tree (root = 0x%08X):\n", (int)m_tree);
		uv_assert_err_ret(m_tree->dump(g_config->m_flirt.m_debugDumpTab, true));
	}
	else
	{
		printf("Missing tree!\n");
	}
	return UV_ERR_OK;
}

