/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd_flirt_pattern_bfd_relocation.h"
#include "uvd_flirt_pattern_bfd_function.h"
#include "uvd_flirt.h"

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

uv_err_t UVDBFDPatRelocations::addRelocation(arelent *bfdRelocation)
{
	UVDBFDPatRelocation *uvdRelocation = NULL;

	//We should be adding after, but within the function
	if( bfdRelocation->address < m_function->m_offset || bfdRelocation->address > (m_function->m_offset + m_function->m_size) )
	{
		//printf_flirt_debug("reloc out of bounds @ 0x%.8X, func %s 0x%.8X - 0x%.8X\n", 
		//		(unsigned int)bfdRelocation->address, bfd_asymbolgenerateByBFD_name(m_sym), m_function->m_offset, m_function->m_offset + m_function->m_size);
		//Don't report: we will do this trying to find the correct function
		return UV_ERR_GENERAL;
	}
	
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
	uvdRelocation->m_offset = bfdRelocation->address - m_function->m_offset;
	printf_flirt_debug("adding relocation @ function offset 0x%.4X from section offset 0x%.4X and function offset 0x%.4X\n",
			(int)uvdRelocation->m_offset, (int)bfdRelocation->address, m_function->m_offset);
	//Zero address relocations don't really make any sense since the opcode is undefined
	uv_assert_ret(uvdRelocation->m_offset > 0);
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

