/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvdstrings/strings.h"
#include <ctype.h>

UVDStringsAnalyzerImpl::UVDStringsAnalyzerImpl(UVDStringsAnalyzerImplPlugin *plugin)
{
	m_plugin = plugin;
}

UVDStringsAnalyzerImpl::~UVDStringsAnalyzerImpl()
{
}

//uv_err_t UVDAnalyzer::analyzeStrings()
uv_err_t UVDStringsAnalyzerImpl::appendAllStrings(std::vector<UVDAddressRange> &out)
{
	/*
	Do strings(3) like processing to find ROM/string data
	For some formats like ELF, it is possible to get this string table more directly
	*/

	for( std::vector<UVDSection *>::iterator iter = object->m_sections.begin();
			iter != object->m_sections.end(); ++iter )
	{
		UVDSection *section = *iter;

		uv_assert_err_ret(doAppendAllStrings(section, out));
	}
	
	return UV_ERR_OK;
}

uv_err_t UVDStringsAnalyzerImpl::doAppendAllStrings(UVDSection *section, std::vector<UVDString> &out)
{
	UVDData *data = NULL;
	
	uv_assert_ret(section);
	data = section->m_data;
	
	//Not a real section?
	if( !data )
	{
		return UV_ERR_OK;
	}
	
	//Do a C/ASCII string table analysis
	for( unsigned int i = 0; i < data->size(); )
	{
		unsigned int j = 0;
		unsigned in nPrintables = 0;
		
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
					out.push_back(UVDString(UVDAddressRange(i, endAddr, section)));
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

