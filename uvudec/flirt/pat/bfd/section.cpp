/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include <string.h>
#include "flirt/pat/bfd/section.h"
#include "flirt/pat/bfd/core.h"
#include "uvd_config.h"
#include "uvd_flirt.h"

/*
UVDBFDPatSection
*/

UVDBFDPatSection::UVDBFDPatSection()
{
	m_section = NULL;
	m_content = NULL;
	m_core = NULL;
}

UVDBFDPatSection::~UVDBFDPatSection()
{
	free(m_content);
}

uv_err_t UVDBFDPatSection::setFunctionSizes()
{
	int sectionSize = 0;

	sectionSize = bfd_section_size(m_core->m_bfd, m_section);
	//Iterate over all functions within that section
	for( std::vector<UVDBFDPatFunction *>::iterator iter = m_functions.m_functions.begin(); iter != m_functions.m_functions.end(); )
	{
		UVDBFDPatFunction *function = NULL;
		
		function = *iter;
		++iter;
		//Did we reach the section end?
		if( iter == m_functions.m_functions.end() )
		{
			function->m_size = sectionSize - function->m_offset;
			break;
		}
		else
		{
			UVDBFDPatFunction *functionNext = NULL;
			
			functionNext = *iter;
			function->m_size = functionNext->m_offset - function->m_offset;
		}
		printf_debug("early func size %s is 0x%.4X\n", bfd_asymbol_name(function->m_bfdAsymbol), function->m_size);
	}
	return UV_ERR_OK;
}

uv_err_t UVDBFDPatSection::print()
{
	//printf_flirt_debug("Section: %s, size 0x%X\n",
	//		   s->section->name, (unsigned int)bfd_section_size(m_bfd, s->section));
	for( std::vector<UVDBFDPatFunction *>::iterator iter = m_functions.m_functions.begin();
			iter != m_functions.m_functions.end(); ++iter )
	{
		UVDBFDPatFunction *function = *iter;
		uv_assert_err_ret(function->print());
	}
	return UV_ERR_OK;
}

uv_err_t UVDBFDPatSection::trimSignatures()
{
	for( std::vector<UVDBFDPatFunction *>::iterator iter = m_functions.m_functions.begin();
			iter != m_functions.m_functions.end(); ++iter )
	{
		UVDBFDPatFunction *function = *iter;

		if( function->m_size >= g_config->m_flirt.m_patSignatureLengthMax )
		{
			printf_flirt_warning("truncating signature for function %s\n", bfd_asymbol_name(function->m_bfdAsymbol));
			//Hmm why was this max - 1?
			function->m_size = g_config->m_flirt.m_patSignatureLengthMax - 1;
		}
	}
	return UV_ERR_OK;
}

uv_err_t UVDBFDPatSection::addFunction(asymbol *functionSymbol)
{
	UVDBFDPatFunction *function = NULL;
	
	function = new UVDBFDPatFunction();
	uv_assert_ret(function);
	function->m_section = this;
	function->m_bfdAsymbol = functionSymbol;
	function->m_offset = functionSymbol->value;
	//We'll get this by computing deltas
	function->m_size = 0;
	uv_assert_err_ret(m_functions.add(function));	
	
	return UV_ERR_OK;
}

uv_err_t UVDBFDPatSection::assignRelocation(arelent *bfdRelocation)
{
	/*
	Add the relocation to appropriete function within the chain
	Not very efficient...just slams on each entry until we hit the correct one
	But maybe thats best we can do since we are using linked list
	*/
	printf_flirt_debug("finding function within section for arelent 0x%.8X\n", (int)bfdRelocation);
	for( std::vector<UVDBFDPatFunction *>::iterator iter = m_functions.m_functions.begin();
			iter != m_functions.m_functions.end(); ++iter )
	{
		UVDBFDPatFunction *uvdFunction = *iter;
		
		//printf_flirt_debug("assinging relocations, current function: 0x%.8X\n", (int)uvdFunction);
		//Successfully added?  We hit the correct range then
		if( UV_SUCCEEDED(uvdFunction->m_relocations.isApplicable(bfdRelocation)) )
		{
			uv_assert_err_ret(uvdFunction->m_relocations.addRelocation(bfdRelocation));
			printf_flirt_debug("found function within section for arelent 0x%.8X\n", (int)bfdRelocation);
			return UV_ERR_OK;
		}
	}
	/*
	Original code silently ignores this
	Ex why: reloc out of bounds @ 0x00008089, func .debug_info 0x00000000 - 0x00007FFF
	This may be partially due to the lose way func detection is currently done
	*/
	printf_flirt_debug("ignoring out of bounds relocation at offset 0x%.4X\n", (int)bfdRelocation->address);
	return UV_ERR_OK;
}

/*
UVDBFDPatSections
*/

UVDBFDPatSections::UVDBFDPatSections()
{
	m_core = NULL;
}

UVDBFDPatSections::~UVDBFDPatSections()
{
	for( std::vector<UVDBFDPatSection *>::iterator iter = m_sections.begin(); iter != m_sections.end(); ++iter )
	{
		delete *iter;
	}
	m_sections.clear();
}

uv_err_t UVDBFDPatSections::print()
{
	for( std::vector<UVDBFDPatSection *>::iterator iter = m_sections.begin(); iter != m_sections.end(); ++iter )
	{
		UVDBFDPatSection *uvdSection = *iter;
		uv_assert_err_ret(uvdSection->print());
	}
	return UV_ERR_OK;
}

uv_err_t UVDBFDPatSections::find(asection *bfdSectionIn, UVDBFDPatSection **uvdSectionOut)
{
	uv_assert_ret(uvdSectionOut);

	for( std::vector<UVDBFDPatSection *>::iterator iter = m_sections.begin(); iter != m_sections.end(); ++iter )
	{
		UVDBFDPatSection *uvdSection = *iter;
		
		uv_assert_ret(uvdSection->m_section);
		//Section names match or equivilent pointer (for null section matches)
		if( ((uvdSection->m_section->name != NULL) && (bfdSectionIn->name != NULL)
				&& !strcmp(uvdSection->m_section->name, bfdSectionIn->name))
				|| bfdSectionIn == uvdSection->m_section )
		{
			*uvdSectionOut = uvdSection;
			return UV_ERR_OK;
		}
	}
	return UV_ERR_NOTFOUND;
}

uv_err_t UVDBFDPatSections::get(struct bfd_section *bfdSectionIn, uint32_t sectionSize, UVDBFDPatSection **uvdSectionOut)
{
	uv_err_t findRc = UV_ERR_GENERAL;
	
	uv_assert_ret(uvdSectionOut);
	findRc = find(bfdSectionIn, uvdSectionOut);
	if( findRc == UV_ERR_NOTFOUND )
	{
		UVDBFDPatSection *uvdSection = NULL;

		uvdSection = new UVDBFDPatSection();
		uv_assert_ret(uvdSection);
		uvdSection->m_section = bfdSectionIn;
		uvdSection->m_content = (unsigned char *)malloc(sectionSize);
		bfd_get_section_contents(m_core->m_bfd, bfdSectionIn, uvdSection->m_content, 0, sectionSize);
		
		uv_assert_err_ret(add(uvdSection));
		*uvdSectionOut = uvdSection;
	}
	else
	{
		uv_assert_err_ret(findRc);
	}

	return UV_ERR_OK;
}

uv_err_t UVDBFDPatSections::add(UVDBFDPatSection *uvdSection)
{
	uvdSection->m_core = m_core;
	m_sections.push_back(uvdSection);
	return UV_ERR_OK;
}

