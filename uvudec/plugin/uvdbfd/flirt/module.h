/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_FLIRT_PATTERN_BFD_MODULE_H
#define UVD_FLIRT_PATTERN_BFD_MODULE_H

#include "uvdbfd/flirt/relocation.h"
#include <stdint.h>
#include "bfd.h"
#include "uvd/util/string_writer.h"

/*
UVDBFDPatModule
was UVDBFDFunctions
*/
class UVDBFDPatSection;
class UVDBFDPatFunction;
class UVDBFDPatModule
{
public:
	class const_iterator
	{
	public:
		class deref
		{
		public:
			deref();
			deref(UVDBFDPatRelocation *relocation, uint8_t byte);
			
			//these were more important in the raw version for finding branches
			//bool operator==(const deref &other) const;
			//bool operator!=(const deref &other) const;
		
		public:
			//If its a relocation, pointer will be set
			UVDBFDPatRelocation *m_relocation;
			uint8_t m_byte;
		};
		
	public:
		const_iterator();
		const_iterator(const UVDBFDPatModule *module, uint32_t offset);
		uv_err_t primeRelocationIterator();
		
		//Offset between the two
		//other should be <= this
		uint32_t difference(const_iterator other);
		//+= operator
		uv_err_t advance(uint32_t bytes);
	
		int compare(const_iterator other) const;
		//bool operator-(const const_iterator &other) const;
		//bool operator+(const const_iterator &other) const;
		bool operator!=(const_iterator other) const;
		bool operator==(const_iterator other) const;
		//Prefix
		uv_err_t operator++();
		uv_err_t next();
		deref operator*() const;
	
		//How far we are in the function
		//uint32_t functionOffset();
		//Howe far we are in the section
		uint32_t sectionOffset();
	
	public:
		union
		{
			const UVDBFDPatModule *m_module;
			UVDBFDPatModule *m_moduleNoConst;
		};
		//Section offset
		uint32_t m_offset;
		std::vector<UVDBFDPatRelocation *>::const_iterator m_relocIter;
	};
	class iterator : public const_iterator
	{
	public:
		iterator();
		iterator(UVDBFDPatModule *module, uint32_t offset);
	};

public:
	UVDBFDPatModule();
	~UVDBFDPatModule();
	
	uv_err_t add(UVDBFDPatFunction *function);
	uv_err_t print();
	
	const_iterator const_begin();
	//From beginning of function, const_offset_begin(0) is equiv to const_begin()
	const_iterator const_offset_begin(uint32_t functionOffset);
	const_iterator const_end();
	iterator begin();
	iterator end();
	//From section start
	uv_err_t offset(uint32_t *offsetOut);
	uv_err_t size(uint32_t *sizeOut);
	uv_err_t trimSignatures();
	//For internal use by iterator
	uv_err_t getEndAddress(uint32_t *out) const;

public:
	//WARNING: these addresses are relative to the section
	UVDBFDPatRelocations m_relocations;
	//Parent section, do not own
	UVDBFDPatSection *m_section;

	//We own these
	std::vector<UVDBFDPatFunction *> m_functions;
};

#endif

