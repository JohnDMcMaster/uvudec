/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvdasm/architecture.h"
#include "uvdasm/config.h"
#include "uvd/assembly/instruction.h"
#include "uvd/core/iterator.h"

UVDDisasmArchitecture::UVDDisasmArchitecture()
{
#ifdef USING_VECTORS
	m_CPU = NULL;
#endif
	m_opcodeTable = NULL;
	m_symMap = NULL;
	m_interpreter = NULL;
}

UVDDisasmArchitecture::~UVDDisasmArchitecture()
{
	UV_DEBUG(deinit());
}

uv_err_t UVDDisasmArchitecture::init()
{
	if( !g_asmConfig->m_architectureFileName.empty() )
	{
		m_architectureFileName = g_asmConfig->m_architectureFileName;
	}
	else if( UV_FAILED(g_asmConfig->getDefaultArchitectureFile(m_architectureFileName)) || m_architectureFileName.empty() )
	{
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	
	printf_plugin_debug("config file: %s\n", m_architectureFileName.c_str());
	
	
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
		UVDAddressSpace *memoryShared = (UVDAddressSpace *)(*iter).second;	
		std::string addressSpaceName;
		
		uv_assert_ret(memoryShared);
		addressSpaceName = (*iter).first;
		names.push_back(addressSpaceName);
	}
	return UV_ERR_OK;
}

