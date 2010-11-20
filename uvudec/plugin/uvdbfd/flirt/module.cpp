/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

/*
UVDBFDPatModule
*/

#include "uvd/config/config.h"
#include "uvdflirt/flirt.h"
#include "uvdflirt/config.h"
#include "uvdbfd/flirt/function.h"
#include "uvdbfd/flirt/module.h"
#include "uvdbfd/flirt/module_printer.h"
#include "uvdbfd/flirt/section.h"

UVDBFDPatModule::UVDBFDPatModule()
{
	m_relocations.m_module = this;
	m_section = NULL;
}

UVDBFDPatModule::~UVDBFDPatModule()
{
	for( std::vector<UVDBFDPatFunction *>::iterator iter = m_functions.begin();
			iter != m_functions.end(); ++iter )
	{
		delete *iter;
	}
	m_functions.clear();
}

uv_err_t UVDBFDPatModule::add(UVDBFDPatFunction *function)
{
	//NOTE: hopefully not an issue, but functions are added when they have an offset, but not a size
	std::vector<UVDBFDPatFunction *>::iterator iter;
	
	//uv_assert_ret(function->m_section);
	//Sorted by function offset
	for( iter = m_functions.begin(); iter != m_functions.end(); ++iter )
	{
		UVDBFDPatFunction *curFunction = NULL;
		
		curFunction = *iter;
		uv_assert_ret(function != curFunction);
		if( function->m_offset < curFunction->m_offset )
		{
			break;
		}
	}
	m_functions.insert(iter, function);
	return UV_ERR_OK;
}

uv_err_t UVDBFDPatModule::offset(uint32_t *offsetOut)
{
	UVDBFDPatFunction *first = NULL;

	uv_assert_ret(!m_functions.empty());

	first = *m_functions.begin();
	uv_assert_ret(first);
	
	uv_assert_ret(offsetOut);
	*offsetOut = first->m_offset;

	return UV_ERR_OK;
}

uv_err_t UVDBFDPatModule::size(uint32_t *sizeOut)
{
	/*
	Depending if we use all of the functions, the module size may be smaller than the section size
	*/
	UVDBFDPatFunction *first = NULL;
	UVDBFDPatFunction *last = NULL;

	uv_assert_ret(!m_functions.empty());

	first = *m_functions.begin();
	uv_assert_ret(first);
	last = *(m_functions.end() - 1);
	uv_assert_ret(last);
	
	/*
	 ----- AAAAA BBBB      CCCC  ----
	      /\                        /\
	       \-Start                   \-End
	*/
	uv_assert_ret(sizeOut);
	*sizeOut = last->m_offset - first->m_offset + last->m_size;
	
	return UV_ERR_OK;
}

uv_err_t UVDBFDPatModule::print()
{
	//printf_flirt_debug("Section: %s, size 0x%X\n",
	//		   s->section->name, (unsigned int)bfd_section_size(m_bfd, s->section));
	for( std::vector<UVDBFDPatFunction *>::iterator iter = m_functions.begin();
			iter != m_functions.end(); ++iter )
	{
		UVDBFDPatModulePrinter printer(this);
		//UVDBFDPatFunction *function = *iter;
	
		uv_assert_err_ret(printer.print());
		return UV_ERR_OK;
	}
	return UV_ERR_OK;
}

/*
UVDBFDPatModule::const_iterator::deref
*/

UVDBFDPatModule::const_iterator::deref::deref()
{
	m_relocation = NULL;
	m_byte = 0;
}

UVDBFDPatModule::const_iterator::deref::deref(UVDBFDPatRelocation *relocation, uint8_t byte)
{
	m_relocation = relocation;
	m_byte = byte;
}

/*
UVDBFDPatModule::const_iterator
*/

UVDBFDPatModule::const_iterator::const_iterator()
{
	m_module = NULL;
}

UVDBFDPatModule::const_iterator::const_iterator(const UVDBFDPatModule *module, uint32_t offset)
{
	m_module = module;
	m_offset = offset;
	UV_DEBUG(primeRelocationIterator());
}

int UVDBFDPatModule::const_iterator::compare(const_iterator other) const
{
	return m_offset - other.m_offset;
}

bool UVDBFDPatModule::const_iterator::operator!=(const_iterator other) const
{
	uint32_t compareRes = compare(other);
	return compareRes != 0;
}

bool UVDBFDPatModule::const_iterator::operator==(const_iterator other) const
{
	return compare(other) == 0;
}

uv_err_t UVDBFDPatModule::const_iterator::primeRelocationIterator()
{
	uv_assert_ret(m_module);
	m_relocIter = m_module->m_relocations.m_relocations.begin();
	//FIXME: shouldn't all relocations sorted into this function be present?
	//we do this because we may not have started at the beginning
	//Prime the iterator if necessary
	//Skip relocations until we are in range
	//printf_flirt_debug("printing pattern bytes, number relocations: %d\n", m_relocations.m_relocations.size());
	while( m_relocIter != m_module->m_relocations.m_relocations.end() )
	{
		UVDBFDPatRelocation *reloc = NULL;
		uint32_t relocAddrMax = 0;

		reloc = *m_relocIter;
		uv_assert_ret(reloc);
		
		relocAddrMax = reloc->m_address + reloc->m_size - 1;
		//In range?
		if( relocAddrMax >= m_offset )
		{
			break;
		}
		//Guess not...advance
		//printf_flirt_debug("skipping relocation b/c not in range, reloc addr: 0%.4X, print start: 0x%.4X\n", relocAddr, start);
		++m_relocIter;
	}
	
	return UV_ERR_OK;
}

uint32_t UVDBFDPatModule::const_iterator::difference(const_iterator other)
{
	return m_offset - other.m_offset;
}

uv_err_t UVDBFDPatModule::const_iterator::advance(uint32_t bytes)
{
	//FIXME: do this directly
	for( uint32_t i = 0; i < bytes; ++i )
	{
		uv_assert_err_ret(operator++());
	}
	return UV_ERR_OK;
}

uv_err_t UVDBFDPatModule::const_iterator::operator++()
{
	UV_DEBUG(next());
	return UV_ERR_OK;
}

uv_err_t UVDBFDPatModule::const_iterator::next()
{
	//printf_flirt_debug("start: 0x%.8X, end: 0x%.8X, m_size: 0x%.8X\n", start, end, m_size);
	uint32_t end = 0;
	
	uv_assert_ret(m_module);
	uv_assert_err_ret(m_module->getEndAddress(&end));

	if( m_offset >= end )
	{
		printf_flirt_debug("assert failed\n");
		//printf_flirt_debug("end: 0x%.8X, function offset: 0x%.8X, function size: 0x%.8X\n", end, lastFunction->m_offset, lastFunction->m_size);
		printf_flirt_debug("end: 0x%.8X\n", end);
		printf_flirt_debug("current offset: 0x%.8X\n", m_offset);
		UVD_PRINT_STACK();
		//exit(1);
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	uv_assert_ret(m_offset < end);
	
	//Then one way (no relocation) or the other (have relocation), we are advancing
	++m_offset;
	
	//But, we should figure out if we need to advance the reloc iter as well
	while( m_relocIter != m_module->m_relocations.m_relocations.end() )
	{
		UVDBFDPatRelocation *reloc = NULL;

		reloc = *m_relocIter;
		uv_assert_ret(reloc);
		//relocations are bumped off during next, so we should never get more than slightly beyond it
		//printf_flirt_debug("print relocation iteration on 0x%.8X from iter 0x%.8X, function (this) 0x%.8X\n", (int)reloc, (int)&relocIter, (int)this);
		printf_flirt_debug("print relocation, iter m_offset 0x%.4X, reloc m_address 0x%.4X, reloc m_size 0x%.4X\n", m_offset, reloc->m_address, reloc->m_size);
		uv_assert_ret(m_offset <= reloc->m_address + reloc->m_size);
		//Did we go past the end of this reloc?
		if( m_offset >= reloc->m_address + reloc->m_size )
		{
			printf_flirt_debug("Advancing reloc iter\n");
			//Then advance to the next one
			//Since we assume all relocations are non-zero and non-intersecting, this should give us a valid pos
			++m_relocIter;
		}
		else
		{
			break;
		}
	}
	return UV_ERR_OK;
}

UVDBFDPatModule::const_iterator::deref UVDBFDPatModule::const_iterator::operator*() const
{
	deref ret;

	if( m_relocIter != m_module->m_relocations.m_relocations.end() )
	{
		UVDBFDPatRelocation *reloc = NULL;

		reloc = *m_relocIter;
		//printf_flirt_debug("print relocation iteration on 0x%.8X from iter 0x%.8X, function (this) 0x%.8X\n", (int)reloc, (int)&relocIter, (int)this);
		//uv_assert_ret(reloc);

		//In range?
		//relocations are bumped off during next, so we should never get beyond it
		//uv_assert_ret(m_offset < reloc->m_address + reloc->m_size);
		if( m_offset >= reloc->m_address )
		{
			ret.m_relocation = reloc;
		}
	}
	//If we aren't shadowed by a reloc, ante up the byte
	if( !ret.m_relocation )
	{
		//ick don't use this its relative to function
		//ret.m_byte = m_function->readByte(m_offset);
		ret.m_byte = m_module->m_section->m_content[m_offset];
	}

	return ret;
}

/*
uint32_t UVDBFDPatModule::const_iterator::functionOffset()
{
	return m_offset - m_function->m_offset;
}
*/

uint32_t UVDBFDPatModule::const_iterator::sectionOffset()
{
	return m_offset;
}

/*
UVDBFDPatModule::iterator
*/

UVDBFDPatModule::iterator::iterator()
{
	//Not needed, base class will do this on union
	//m_functionNoConst = NULL;
}

UVDBFDPatModule::iterator::iterator(UVDBFDPatModule *module, uint32_t offset)
: UVDBFDPatModule::const_iterator()
{
	m_moduleNoConst = module;
	m_offset = offset;
	m_relocIter = module->m_relocations.m_relocations.begin();
	UV_DEBUG(primeRelocationIterator());
}

UVDBFDPatModule::const_iterator UVDBFDPatModule::const_begin()
{
	//return const_iterator(this, m_offset);
	return const_iterator(this, m_functions[0]->m_offset);
}

UVDBFDPatModule::const_iterator UVDBFDPatModule::const_offset_begin(uint32_t functionOffset)
{
	//Equal to is .end()
	/*
	if( functionOffset > m_size )
	{
		printf_error("bad begin, size exceeded\n");
		printf_error("size: 0x%.4X, offset: 0x%.4X\n", m_size, functionOffset);
		UVD_PRINT_STACK();
	}
	*/
	//We internally store iter pos as section offset
	//return const_iterator(this, m_offset + functionOffset);
	return const_iterator(this, functionOffset);
}

UVDBFDPatModule::const_iterator UVDBFDPatModule::const_end()
{
	uint32_t endPos = 0;
	
	//endPos = m_offset + m_size;
	UV_DEBUG(getEndAddress(&endPos));
	return const_iterator(this, endPos);
}

UVDBFDPatModule::iterator UVDBFDPatModule::begin()
{
	uint32_t offset = 0;
	if( !m_functions.empty() )
	{
		offset = m_functions[0]->m_offset;
	}
	return iterator(this, offset);
}

UVDBFDPatModule::iterator UVDBFDPatModule::end()
{
	uint32_t endPos = 0;
	
	//endPos = m_offset + m_size;
	UV_DEBUG(getEndAddress(&endPos));
	return iterator(this, endPos);
}

uv_err_t UVDBFDPatModule::getEndAddress(uint32_t *endOut) const
{
	//Returns the first invalid address
	UVDBFDPatFunction *lastFunction = NULL;

	//End is highest of all of the function endings
	//Since last is at highest address, its that one
	uv_assert_ret(!m_functions.empty());
	lastFunction = m_functions[m_functions.size() - 1];
	*endOut = lastFunction->m_offset + lastFunction->m_size;

	return UV_ERR_OK;
}

uv_err_t UVDBFDPatModule::trimSignatures()
{
	for( std::vector<UVDBFDPatFunction *>::iterator iter = m_functions.begin();
			iter != m_functions.end(); ++iter )
	{
		UVDBFDPatFunction *function = *iter;

		if( function->m_size >= g_UVDFLIRTConfig->m_patSignatureLengthMax )
		{
			printf_flirt_warning("truncating signature for function %s\n", bfd_asymbol_name(function->m_bfdAsymbol));
			//Hmm why was this max - 1?
			function->m_size = g_UVDFLIRTConfig->m_patSignatureLengthMax - 1;
		}
	}
	return UV_ERR_OK;
}

