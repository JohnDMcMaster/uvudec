/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include <string.h>
#include "uvdbfd/flirt/core.h"
#include "uvdbfd/flirt/module.h"
#include "uvdbfd/flirt/section.h"
#include "uvd/config/config.h"
#include "uvdflirt/config.h"
#include "uvdflirt/flirt.h"

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

uv_err_t UVDBFDPatSection::size(uint32_t *out)
{
	uv_assert_ret(m_core);
	uv_assert_ret(m_section);
	
	*out = bfd_section_size(m_core->m_bfd, m_section);
	return UV_ERR_OK;
}

uv_err_t UVDBFDPatSection::setFunctionSizes()
{
	//Functions were sorted into modules, but we need a contiguous list to determine sizes
	std::vector<UVDBFDPatFunction *> allSectionFunctions;
	int sectionSize = 0;

	for( std::vector<UVDBFDPatModule *>::iterator iter = m_modules.begin();
			iter != m_modules.end(); ++iter )
	{
		UVDBFDPatModule *module = *iter;
		
		uv_assert_ret(module);
		//std::vector<UVDBFDPatFunction *> m_functions
		allSectionFunctions.insert(allSectionFunctions.end(), module->m_functions.begin(), module->m_functions.end());
	}

	sectionSize = bfd_section_size(m_core->m_bfd, m_section);
	//Iterate over all functions within that section
	for( std::vector<UVDBFDPatFunction *>::iterator iter = allSectionFunctions.begin(); iter != allSectionFunctions.end(); )
	{
		UVDBFDPatFunction *function = NULL;
		std::vector<UVDBFDPatFunction *>::iterator iterStart = iter;
		
		/*
		What if we have a 0 sized symbol in the section?
			Special case: symbol is at the end of the section
		*/
		do
		{
			function = *iter;
			++iter;
			//Get the next size
			//Did we reach the section end?
			if( iter == allSectionFunctions.end() )
			{
				function->m_size = sectionSize - function->m_offset;
				//Even if we still have 0 size, we need to break
				break;
			}
			else
			{
				UVDBFDPatFunction *functionNext = NULL;
			
				functionNext = *iter;
				function->m_size = functionNext->m_offset - function->m_offset;
			}
		}
		while( function->m_size == 0 );
		
		//And set any previously skipped elements from same symbol position
		while( iterStart != iter )
		{
			(*iterStart)->m_size = function->m_size;
			++iterStart;
		}
		
		printf_flirt_debug("early func size %s is 0x%04X\n", bfd_asymbol_name(function->m_bfdAsymbol), function->m_size);
	}
	return UV_ERR_OK;
}

uv_err_t UVDBFDPatSection::print()
{
	for( std::vector<UVDBFDPatModule *>::iterator iter = m_modules.begin();
			iter != m_modules.end(); ++iter )
	{
		UVDBFDPatModule *module = *iter;
		
		uv_assert_ret(module);
		uv_assert_err_ret(module->print());
	}
	return UV_ERR_OK;
}

uv_err_t UVDBFDPatSection::trimSignatures()
{
	for( std::vector<UVDBFDPatModule *>::iterator iter = m_modules.begin();
			iter != m_modules.end(); ++iter )
	{
		UVDBFDPatModule *module = *iter;
		
		uv_assert_ret(module);
		uv_assert_err_ret(module->trimSignatures());
	}
	return UV_ERR_OK;
}

uv_err_t UVDBFDPatSection::addFunction(asymbol *functionSymbol)
{
	UVDBFDPatFunction *function = NULL;
	UVDBFDPatModule *module = NULL;
				
	function = new UVDBFDPatFunction();
	uv_assert_ret(function);
	function->m_bfdAsymbol = functionSymbol;
	function->m_offset = functionSymbol->value;
	//We'll get this by computing deltas
	function->m_size = 0;

	//Should we group functions into single modules or each its own module?
	if( g_UVDFLIRTConfig->m_functionsAsModules )
	{
		module = new UVDBFDPatModule();
		uv_assert_ret(module);
		module->m_section = this;
		uv_assert_err_ret(module->add(function));	
	
		m_modules.push_back(module);
	}
	else
	{
		//Create the single module if it didn't already exist
		if( m_modules.empty() )
		{
			module = new UVDBFDPatModule();
			uv_assert_ret(module);
			module->m_section = this;
			m_modules.push_back(module);
		}
		else
		{
			module = m_modules[0];
			uv_assert_ret(module);
		}
		uv_assert_ret(m_modules.size() == 1);
		uv_assert_err_ret(module->add(function));	
	}
	function->m_module = module;

	return UV_ERR_OK;
}

uv_err_t UVDBFDPatSection::assignRelocation(arelent *bfdRelocation)
{
	/*
	Add the relocation to appropriete function within the chain
	Not very efficient...just slams on each entry until we hit the correct one
	But maybe thats best we can do since we are using linked list
	*/
	printf_flirt_debug("finding function within section for arelent 0x%08X @ offset 0x%04X\n", (int)bfdRelocation, bfdRelocation->address);
	//We do this because multiple at the same address need duplicate relocations
	//FIXME: this is a quick hack, we should probaly group the symbols into one function object
	bool previousSuccess = false;
	for( std::vector<UVDBFDPatModule *>::iterator iter = m_modules.begin();
			iter != m_modules.end(); ++iter )
	{
		UVDBFDPatModule *module = *iter;
		
		uv_assert_ret(module);

		//printf_flirt_debug("module functions: %d\n", module->m_functions.size());
		//printf_flirt_debug("assinging relocations, current function: 0x%08X\n", (int)uvdFunction);
		//Successfully added?  We hit the correct range then
		if( UV_SUCCEEDED(module->m_relocations.isApplicable(bfdRelocation)) )
		{
			previousSuccess = true;
			uv_assert_err_ret(module->m_relocations.addRelocation(bfdRelocation));
			//printf_flirt_debug("0x%08X new relocs: %d\n", (int)uvdFunction, uvdFunction->m_relocations.m_relocations.size());
			//printf_flirt_debug("found function within section for arelent 0x%08X\n", (int)bfdRelocation);
		}
		else if( previousSuccess )
		{
			return UV_ERR_OK;
		}
	}
	if( previousSuccess )
	{
		return UV_ERR_OK;
	}
	/*
	Original code silently ignores this
	Ex why: reloc out of bounds @ 0x00008089, func .debug_info 0x00000000 - 0x00007FFF
	This may be partially due to the lose way func detection is currently done
	*/
	printf_flirt_debug("ignoring out of bounds relocation at offset 0x%04X\n", (int)bfdRelocation->address);
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

