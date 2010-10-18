/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvdbfd/flirt/relocation.h"
#include "uvdbfd/flirt/function.h"
#include "uvd/flirt/flirt.h"

/*
UVDBFDPatRelocation
*/

UVDBFDPatRelocation::UVDBFDPatRelocation()
{
	m_address = 0;
	m_size = 0;
	m_offset = 0;
}

UVDBFDPatRelocation::~UVDBFDPatRelocation()
{
}

/*
UVDBFDPatRelocations
*/

UVDBFDPatRelocations::UVDBFDPatRelocations()
{
	m_function = NULL;
}

UVDBFDPatRelocations::~UVDBFDPatRelocations()
{
}

uv_err_t UVDBFDPatRelocations::isApplicable(arelent *bfdRelocation)
{
	//We should be adding after, but within the function
	if( bfdRelocation->address < m_function->m_offset || bfdRelocation->address > (m_function->m_offset + m_function->m_size) )
	{
		//printf_flirt_debug("reloc out of bounds @ 0x%.8X, func %s 0x%.8X - 0x%.8X\n", 
		//		(unsigned int)bfdRelocation->address, bfd_asymbolgenerateByBFD_name(m_sym), m_function->m_offset, m_function->m_offset + m_function->m_size);
		//Don't report: we will do this trying to find the correct function
		return UV_ERR_GENERAL;
	}
	return UV_ERR_OK;
}

uv_err_t UVDBFDPatRelocations::addRelocation(arelent *bfdRelocation)
{
	UVDBFDPatRelocation *uvdRelocation = NULL;

	uv_assert_err_ret(isApplicable(bfdRelocation));
	
	//TODO: make this a std::set (with a custom sort func)
	std::vector<UVDBFDPatRelocation *>::iterator iter;
	//Lowest first
	for( iter = m_relocations.begin(); iter != m_relocations.end(); ++iter )
	{	
		//Did we surpass the lesser elements?
		if( bfdRelocation->address < (*iter)->m_address )
		{
			break;
		}
	}
		
	//we needed add new reloc before prev 
	uvdRelocation = new UVDBFDPatRelocation();
	uv_assert_ret(uvdRelocation);
	uvdRelocation->m_size = bfdRelocation->howto->bitsize / 8;
	uvdRelocation->m_address = bfdRelocation->address;
	uv_assert_ret(bfdRelocation->address >= m_function->m_offset);
	uvdRelocation->m_offset = bfdRelocation->address - m_function->m_offset;
	printf_flirt_debug("adding rel 0x%.8X @ func %s off 0x%.4X from sec off 0x%.4X, func off 0x%.4X\n",
			(int)uvdRelocation, m_function->m_bfdAsymbol->name, (int)uvdRelocation->m_offset, (int)bfdRelocation->address, m_function->m_offset);
	//Zero address relocations don't really make any sense since the opcode is undefined
	//Should indicate an internal error
	//printf_flirt_debug("function symbol name: %s\n", m_function->m_bfdAsymbol->name);
	/*
	FIXME
	This assertion doesn't currently work
	This will fail for example on the .rodata section if something needs to be linked to the front
	What is being called a "function" in BFD FLIRT is actually a symbol
	We do this to more effectivly trim functions based on surrounding symbools, such as global data buffers
	We should rename it to reflect this
	
	It is not advisable to only keep functions as we may later want to do processing on the other types of symbols and it simplifies code and improves error handling
		ex: relocations must always be placed, no special cases for discarding and error if we can't place it
	*/
	//uv_assert_ret(uvdRelocation->m_offset > 0);
	uvdRelocation->m_symbolName = "";
	//Jumps and others will have unnamed relocations
	if( bfdRelocation->sym_ptr_ptr != NULL )
	{
		int flags = (*bfdRelocation->sym_ptr_ptr)->flags;
		if( !(flags & BSF_SECTION_SYM) )
		{
			uvdRelocation->m_symbolName = (*bfdRelocation->sym_ptr_ptr)->name;
		}
	}
	
	m_relocations.insert(iter, uvdRelocation);
	
	return UV_ERR_OK;
}

