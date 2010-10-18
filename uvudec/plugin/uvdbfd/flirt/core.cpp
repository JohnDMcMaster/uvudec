/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/flirt/flirt.h"
#include "uvdbfd/flirt/core.h"
#include <string.h>

UVDBFDPatCore::UVDBFDPatCore()
{
	m_contiguousSymbolTable = NULL;
	m_bfd = NULL;
}

UVDBFDPatCore::~UVDBFDPatCore()
{
	UV_DEBUG(deinit());
}

uv_err_t UVDBFDPatCore::init(bfd *abfd)
{
	m_sections.m_core = this;
	m_bfd = abfd;
	uv_assert_ret(m_bfd);
	return UV_ERR_OK;
}

uv_err_t UVDBFDPatCore::deinit()
{
	free(m_contiguousSymbolTable);
	m_contiguousSymbolTable = NULL;

	return UV_ERR_OK;
}

uv_err_t UVDBFDPatCore::generate()
{
	char **matching = NULL;
	
	printf_flirt_debug("Processing file %s\n", bfd_get_filename(m_bfd));
	uv_assert_ret(bfd_check_format_matches(m_bfd, bfd_object, &matching));
	
	if( !(bfd_get_file_flags(m_bfd) & HAS_SYMS) )
	{
		//printf_warn("No symbols in \"%s\".\n", bfd_get_filename(m_bfd));
		//printf_warn("flags: 0x%.8X\n", bfd_get_file_flags(m_bfd));
		//return UV_DEBUG(UV_ERR_GENERAL);
		return UV_ERR_OK;
	}

	//Figure out where all of the symbols are
	printf_flirt_debug("\n\nbuilding symbol table\n");
	uv_assert_err_ret(buildSymbolTable());
	//Trim areas to create functions
	printf_flirt_debug("\n\nsetting function sizes\n");
	uv_assert_err_ret(setFunctionSizes());
	//Assign relocations to each function
	printf_flirt_debug("\n\nplacing relocations into functions\n");
	uv_assert_err_ret(placeRelocationsIntoFunctions());
	//And print the signatures
	printf_flirt_debug("\n\nprinting\n");
	uv_assert_err_ret(print());

	printf_flirt_debug("\n\noh snap!  finished normally\n");
	return UV_ERR_OK;
}

uv_err_t UVDBFDPatCore::buildSymbolTable()
{
	//less than zero signifies error
	int32_t symbolCount = 0;
	uint32_t symbolTableMemorySize = 0;
	asymbol **currentSymbol = NULL;

	//What should we do if we get 0 syms?
	symbolTableMemorySize = bfd_get_symtab_upper_bound(m_bfd);
	uv_assert_ret(symbolTableMemorySize >= 0);
	if( symbolTableMemorySize )
	{
		m_contiguousSymbolTable = (asymbol **)malloc(symbolTableMemorySize);
	}
	symbolCount = bfd_canonicalize_symtab(m_bfd, m_contiguousSymbolTable);
	
	// < 0: error, == 0: no syms
	//Oddly enough, ref impl had this unsigned long
	uv_assert_ret(symbolCount >= 0);
	//Begin
	currentSymbol = m_contiguousSymbolTable;
	for( uint32_t symbolIndex = 0; symbolIndex < (uint32_t)symbolCount; ++symbolIndex, ++currentSymbol )
	{
		int flags = 0;
		bfd *symbolBfd = NULL;
		asymbol *bfdAsymbol = NULL;

		//Skip externally defined syms?
		//Need to figure out relation between null symbol and null symbol bfd		
		if( currentSymbol == NULL )
		{
			printf_flirt_debug("skipping null symbol\n");
			continue;
		}
		bfdAsymbol = *currentSymbol;
		uv_assert_ret(bfdAsymbol);
		//What is this useful for?
		symbolBfd = bfd_asymbol_bfd(bfdAsymbol);
		if( symbolBfd == NULL )
		{
			printf_error("symbol missing bfd: %s\n", bfd_asymbol_name(bfdAsymbol));
			return UV_DEBUG(UV_ERR_GENERAL);
		}
		
		flags = bfdAsymbol->flags;
		/*
		TODO: can we move this to shouldPrintFunction()?
		//Ignore local symbols (alias?)
		if( flags & BSF_WEAK )
		{
			printf_flirt_debug("skipping weak symbol: %s\n", bfd_asymbol_name(bfdAsymbol));
			continue;
		}
		*/
		printf_flirt_debug("sym name: %s, flags: 0x%X, val: 0x%X, asymbol val: 0x%X\n",
					   bfd_asymbol_name(bfdAsymbol), flags, (unsigned int)bfdAsymbol->value, (unsigned int)bfd_asymbol_value(bfdAsymbol));
		//Skip undefined symbols
		//Is this how we are deciding if its a function?
		if( bfdAsymbol->section == NULL )
		{
			printf_flirt_debug("skipping sectionless symbol: %s\n", bfd_asymbol_name(bfdAsymbol));
			continue;
		}
		
		//Symbols are not per section, so the particular section and function must be invidually added
		{
			UVDBFDPatSection *uvdSection = NULL;
			uint32_t sectionSize = 0;
			
			sectionSize = bfd_section_size(m_bfd, bfdAsymbol->section);
			if( sectionSize <= 0 )
			{
				printf_flirt_debug("skipping sizeless symbol: %s\n", bfd_asymbol_name(bfdAsymbol));
				continue;
			}

			uv_assert_err_ret(m_sections.get(bfdAsymbol->section, sectionSize, &uvdSection));
			uv_assert_ret(uvdSection);
			uv_assert_err_ret(uvdSection->addFunction(bfdAsymbol));
		}
	}
	return UV_ERR_OK;
}

/*
Assign function sizes
Hmm does differential thing...shouldn't we do it by symbol size?
*/
uv_err_t UVDBFDPatCore::setFunctionSizes()
{
	printf_flirt_debug("sig max length: 0x%.4X\n", g_config->m_flirt.m_patSignatureLengthMax);

	//Iterate over all sections
	for( std::vector<UVDBFDPatSection *>::iterator sectionIter = m_sections.m_sections.begin();
			sectionIter != m_sections.m_sections.end(); ++sectionIter )
	{
		UVDBFDPatSection *section = *sectionIter;
		uv_assert_err_ret(section->setFunctionSizes());
		uv_assert_err_ret(section->trimSignatures());
	}
	return UV_ERR_OK;
}

uv_err_t UVDBFDPatCore::placeRelocationsIntoFunctions()
{
	asection *bfdAsection = NULL;
	
	printf_flirt_debug("sorting relocations into functions\n");
	//Sort relocations into function buckets
	for( bfdAsection = m_bfd->sections; bfdAsection != NULL; bfdAsection = bfdAsection->next )
	{
		uint32_t relocationSectionSize = 0;
		uint32_t relocationCount = 0;
		arelent **relocationArray = NULL;
		arelent **relocationArrayPointer = NULL;
		UVDBFDPatSection *uvdSection = NULL;

		//What is a com section?
		if( bfd_is_abs_section(bfdAsection) || bfd_is_und_section(bfdAsection) || bfd_is_com_section(bfdAsection) )
		{
			printf_flirt_debug("skipping section %s: bad type\n", bfdAsection->name);
			continue;
		}
		//Obviously has to be a relocation section
		if( !(bfdAsection->flags & SEC_RELOC) )
		{
			printf_flirt_debug("skipping section %s: is reloc\n", bfdAsection->name);
			continue;
		}
		relocationSectionSize = bfd_get_reloc_upper_bound(m_bfd, bfdAsection);
		uv_assert_ret(relocationSectionSize >= 0);
		if( relocationSectionSize == 0 )
		{
			printf_flirt_debug("skipping section %s: 0 relocation size\n", bfdAsection->name);
			continue;
		}
		uv_assert_err_ret(m_sections.find(bfdAsection, &uvdSection));
		if( uvdSection == NULL )
		{
			printf_flirt_debug("skipping section %s: couldn't find section to apply to\n", bfdAsection->name);
			continue;
		}
		printf_flirt_debug("Reloc section name is %s\n", bfdAsection->name);
		if( !strcmp(".debug_info", bfdAsection->name) )
		{
			printf_flirt_debug("skipping section %s: test\n", bfdAsection->name);
			continue;
		}
		relocationArray = (arelent **)malloc(relocationSectionSize);
		relocationCount = bfd_canonicalize_reloc(m_bfd, bfdAsection, relocationArray, m_contiguousSymbolTable);
		if( relocationCount <= 0 )
		{
			printf_flirt_debug("skipping section %s: couldn't canonicalize relocations\n", bfdAsection->name);
			free(relocationArray);
			continue;
		}
		
		printf_flirt_debug("Looping over relocations, number: %d\n", relocationCount);
		//is the array null terminated?  Is the relocation count just an extra safety?
		for( relocationArrayPointer = relocationArray; relocationCount && *relocationArrayPointer != NULL; ++relocationArrayPointer, --relocationCount )
		{
			printf_flirt_debug("Looping over relocations, remaining: %d\n", relocationCount);
			arelent *currentRelocation = *relocationArrayPointer;
			uv_assert_err_ret(uvdSection->assignRelocation(currentRelocation));
			
			/*
			if( currentRelocation->sym_ptr_ptr != NULL )
			{
				printf_flirt_debug("Reloc: name %s vma %ld, flags 0x%X\n",
					   (*currentRelocation->sym_ptr_ptr)->name,
					   bfd_asymbol_value((*currentRelocation->sym_ptr_ptr)), (unsigned int)(*currentRelocation->sym_ptr_ptr)->flags);
			}
			printf_flirt_debug("Reloc: addres 0x%X, addend 0x%X, bitsize %d\n",
					(unsigned int)currentRelocation->address, (unsigned int)currentRelocation->addend, currentRelocation->howto->bitsize);
			*/
		}
		free(relocationArray);
	}
	
	return UV_ERR_OK;
}

uv_err_t UVDBFDPatCore::print()
{
	printf_flirt_debug("Beginning print\n");
	
	uv_assert_err_ret(m_sections.print());

	return UV_ERR_OK;
}

