/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/core/runtime.h"
#include "uvd/core/uvd.h"
#include "uvdstrings/plugin.h"
#include "uvdstrings/strings.h"
#include <ctype.h>

UVDStringsAnalyzerImpl::UVDStringsAnalyzerImpl(UVDStringsPlugin *plugin)
{
	m_plugin = plugin;
}

UVDStringsAnalyzerImpl::~UVDStringsAnalyzerImpl()
{
}

//uv_err_t UVDAnalyzer::analyzeStrings()
uv_err_t UVDStringsAnalyzerImpl::appendAllStrings(std::vector<UVDString> &out)
{
	/*
	Do strings(3) like processing to find ROM/string data
	For some formats like ELF, it is possible to get this string table more directly
	*/
	UVDAddressSpaces *addressSpaces = NULL;
	
	addressSpaces = &m_plugin->m_uvd->m_runtime->m_addressSpaces;

	for( std::vector<UVDAddressSpace *>::iterator iter = addressSpaces->m_addressSpaces.begin();
			iter != addressSpaces->m_addressSpaces.end(); ++iter )
	{
		UVDAddressSpace *addressSpace = *iter;

		uv_assert_err_ret(doAppendAllStrings(addressSpace, out));
	}
	
	return UV_ERR_OK;
}

uv_err_t UVDStringsAnalyzerImpl::doAppendAllStrings(UVDAddressSpace *addressSpace, std::vector<UVDString> &out)
{
	UVDData *data = NULL;
	
	uv_assert_ret(addressSpace);
	data = addressSpace->m_data;
	
	//Not a real address space (section)?
	if( !data )
	{
		return UV_ERR_OK;
	}
	
	//Do a C/ASCII string table analysis
	for( unsigned int i = 0; i < data->size(); )
	{
		unsigned int j = 0;
		unsigned int nPrintables = 0;
		
		for( j = i; j <= data->size(); )
		{
			char c = 0;
			
			uv_assert_err_ret(data->readData(j, &c, sizeof(c)));
			++j;
			
			//null terminator
			if( c == 0 )
			{
				if( nPrintables > m_minLength )
				{
					//Add the string
					uv_addr_t endAddr = j - 1;

					//Just insert one reference to each string for now
					//A map
					//m_analyzer->m_stringAddresses[i] = new UVDAnalyzedMemoryRange(i, endAddr);
					//m_analyzer->m_stringAddresses[i]->insertReference(i, UVD_MEMORY_REFERENCE_CONSTANT | UVD_MEMORY_REFERENCE_STRING);
					out.push_back(UVDString(UVDAddressRange(i, endAddr, addressSpace)));
				}
				
				break;
			}
			else if( isprint(c) )
			{
				++nPrintables;
			}
			else
			{
				break;
			}
		}
		
		//Start next where we left off			
		i = j;
	}

	return UV_ERR_OK;
}

