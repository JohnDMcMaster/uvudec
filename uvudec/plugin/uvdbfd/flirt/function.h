/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_FLIRT_PATTERN_BFD_FUNCTION_H
#define UVD_FLIRT_PATTERN_BFD_FUNCTION_H

#include "uvdbfd/flirt/relocation.h"
#include <stdint.h>
#include "bfd.h"
#include "uvd/util/string_writer.h"

/*
UVDBFDFunction
*/
class UVDBFDPatSection;
class UVDBFDPatFunction
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
		const_iterator(const UVDBFDPatFunction *function, uint32_t offset);
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
		uint32_t functionOffset();
		//Howe far we are in the section
		uint32_t sectionOffset();
	
	public:
		union
		{
			const UVDBFDPatFunction *m_function;
			UVDBFDPatFunction *m_functionNoConst;
		};
		//Section offset
		uint32_t m_offset;
		std::vector<UVDBFDPatRelocation *>::const_iterator m_relocIter;
	};
	class iterator : public const_iterator
	{
	public:
		iterator();
		iterator(UVDBFDPatFunction *function, uint32_t offset);
	};

public:	
	UVDBFDPatFunction();
	~UVDBFDPatFunction();
	
	uv_err_t print();
	//There is NO error handling on this
	//Relative to function start
	uint8_t readByte(uint32_t offset);

	/*
	For various reasons, we may trim off various things:
	-Relocations at the end?
		Since they are wildcard, they are not part of the signature?
		Old code did this, but seems incorrect
		
	-Non-recursive descent reachable bytes
	-Compiler alignment fill bytes (typically 0xFF or 0x00.  I've also seen 0x7F and 0xCC)
	allowRelocations: only allow relocation free data
	*/
	uv_err_t getEffectiveEndPosition(uint32_t *endPos, uint32_t allowRelocations);

	const_iterator const_begin();
	//From beginning of function, const_offset_begin(0) is equiv to const_begin()
	const_iterator const_offset_begin(uint32_t functionOffset);
	const_iterator const_end();
	iterator begin();
	iterator end();

public:
	asymbol *m_bfdAsymbol;
	//WARNING: these addresses are relative to the section
	UVDBFDPatRelocations m_relocations;
	//Parent section, do not own
	UVDBFDPatSection *m_section;
	//Offset within the section
	uint32_t m_offset;
	//Size of function in bytes
	uint32_t m_size;
};

/*
UVDBFDFunctions
*/
class UVDBFDPatModule
{
public:
	UVDBFDPatModule();
	~UVDBFDPatModule();
	
	uv_err_t add(UVDBFDPatFunction *function);
	
public:
	//We own these
	std::vector<UVDBFDPatFunction *> m_functions;
};

#endif

