/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvdasm/architecture.h"
#include "uvd_instruction.h"
#include "uvd_iterator.h"

static uv_err_t getDefaultCPUFile(std::string &ret)
{
	ret = DEFAULT_CPU_FILE;
	//This should be relative to installed directory if not absolute
	if( ret[0] != '/' )
	{
		ret = g_config->m_installDir + "/" + ret;
	}
	return UV_ERR_OK;
}

UVDDisasmArchitecture::UVDDisasmArchitecture()
{
#ifdef USING_VECTORS
	m_CPU = NULL;
#endif
	m_opcodeTable = NULL;
	m_symMap = NULL;
	m_interpreter = NULL;
	m_architecture = 0;
}

UVDDisasmArchitecture::~UVDDisasmArchitecture()
{
	UV_DEBUG(deinit());
}

uv_err_t UVDDisasmArchitecture::init()
{
	if( g_config->m_architectureFileName.empty() )
	{
		//In theory if not specified, should search dir
		//Doing en mass brute force analysis to find best matching file is an interesting topic by itself
		switch( m_architecture )
		{
		default:
			uv_assert_err_ret(getDefaultCPUFile(g_config->m_architectureFileName));
		};
	}
	
	m_opcodeTable = new UVDDisasmOpcodeLookupTable();
	//printf_debug("Initializing opcode table, address: 0x%.8X\n", (unsigned int)m_opcodeTable);
	uv_assert_ret(m_opcodeTable);
	//m_CPU->m_opcodeTable = m_opcodeTable;
	
	m_symMap = new UVDSymbolMap();
	uv_assert_ret(m_symMap);
	uv_assert_err_ret(m_symMap->init());
	//m_interpreter = new UVDConfigExpressionInterpreter();
	UVDConfigExpressionInterpreter::getConfigExpressionInterpreter(&m_interpreter);
	uv_assert_ret(m_interpreter);

	printf_debug("Initializing config...\n");
	if( UV_FAILED(init_config()) )
	{
		printf_error("failed 8051 init\n");
		return UV_ERR(UV_ERR_GENERAL);
	}

	return UV_ERR_OK;
}

uv_err_t UVDDisasmArchitecture::deinit()
{
	delete m_opcodeTable;
	m_opcodeTable = NULL;

	delete m_symMap;
	m_symMap = NULL;

	delete m_interpreter;
	m_interpreter = NULL;

	for( std::map<std::string, UVDRegisterShared *>::iterator iter = m_registers.begin(); iter != m_registers.end(); ++iter )
	{
		UVDRegisterShared *regShared = (*iter).second;

		if( !regShared )
		{
			printf_warn("bad regShared entry\n");
		}
		else
		{
			delete regShared;
		}
	}
	m_registers.clear();

	return UV_ERR_OK;
}

uv_err_t UVDDisasmArchitecture::getInstruction(UVDInstruction **out)
{
	uv_assert_ret(out);
	*out = new UVDDisasmInstruction();
	uv_assert_ret(*out);
	return UV_ERR_OK;
}

void UVDDisasmArchitecture::updateCache(uint32_t address, const UVDVariableMap &analysisResult)
{
	printf_debug("Caching analysis of address %d\n", address);
	m_analysisCache[address] = analysisResult;
}

uv_err_t UVDDisasmArchitecture::readCache(uint32_t address, UVDVariableMap &analysisResult)
{
	if( m_analysisCache.find(address) == m_analysisCache.end() )
	{
		return UV_ERR_GENERAL;
	}
	analysisResult = m_analysisCache[address];
	return UV_ERR_OK;
}

uv_err_t UVDDisasmArchitecture::getAddresssSpaceNames(std::vector<std::string> &names)
{
	for( UVDSymbolMap::SymbolMapMap::iterator iter = m_symMap->m_map.begin(); 
			iter != m_symMap->m_map.end(); ++iter )
	{
		//std::string, UVDSymbol *
		UVDMemoryShared *memoryShared = (UVDMemoryShared *)(*iter).second;	
		std::string addressSpaceName;
		
		uv_assert_ret(memoryShared);
		addressSpaceName = (*iter).first;
		names.push_back(addressSpaceName);
	}
	return UV_ERR_OK;
}

uv_err_t UVDDisasmArchitecture::parseCurrentInstruction(UVDIteratorCommon &iterCommon)
{
	//Reduce errors from stale data
	if( !iterCommon.m_instruction )
	{
		//iterCommon.m_instruction = new UVDDisasmInstruction();
		uv_assert_err_ret(getInstruction(&iterCommon.m_instruction));
		uv_assert_ret(iterCommon.m_instruction);
	}
	uv_assert_err_ret(iterCommon.m_instruction->parseCurrentInstruction(iterCommon));

	return UV_ERR_OK;
}

