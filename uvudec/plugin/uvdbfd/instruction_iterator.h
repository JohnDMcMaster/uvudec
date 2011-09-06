/*
UVNet Universal Decompiler (uvudec)
Copyright 2011 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVDBFD_INSTRUCTION_ITERATOR_H
#define UVDBFD_INSTRUCTION_ITERATOR_H

#include "uvd/core/iterator.h"
#include "dis-asm.h"
#include "uvdbfd/instruction.h"

typedef struct
{
	char *buffer;
	size_t pos;
	size_t alloc;
} SFILE;

class UVDBFDObject;
class UVDBFDArchitecture;
class UVDBfdInstructionIterator : public UVDAbstractInstructionIterator
{
public:
	UVDBfdInstructionIterator();	
	virtual ~UVDBfdInstructionIterator();
	
	uv_err_t initByAddress(UVDBFDObject *obj, UVDAddress address);
	uv_err_t initEnd(UVDBFDObject *obj, UVDAddressSpace *addressSpace);
	UVDBFDArchitecture *arch();
	
	virtual uv_err_t getPosition(UVDAddress *out);
	virtual uv_err_t next();

	virtual uv_err_t copy(UVDAbstractInstructionIterator **out) const;
	virtual int compare(const UVDAbstractInstructionIterator &other) const;

	virtual uv_err_t get(UVDInstruction **out) const;

protected:
	//Of current section
	bool isEnd() const;
	uv_err_t makeEnd();
	uv_err_t init();
	uv_err_t initNextValidSection();
	uv_err_t dissassembleCur();
	uv_err_t doNextSection();
	uv_err_t nextSection();
	uv_err_t initCurrentSection();

	virtual uv_err_t check();
	
public:
	/*
	Don't store things that are redundant with disassemble_info as it adds potential for errors
	*/
	UVDBFDObject *m_obj;
	//XXX: not sure if we need this or not since its in m_disasm_info
	//but original code didn't seem to want to rely on that
	asection *m_section;
	//bfd_byte *m_dataBuffer;
	//size_t m_dataBufferSize;
	bfd_vma m_curOffset;
	bfd_vma m_maxOffset;
	bfd_vma m_startOffset;
	bfd_vma m_nextOffset;
	//XXX: this is really more of a safety?  should not exceed because of end()
	//bfd_vma m_maxOffset;
	//unsigned int m_octets_per_byte;
	//unsigned int m_bytes_per_line;
	struct disassemble_info m_disasm_info;
	bfd_byte *m_data;
	bfd_size_type m_datasize;
	//We must disassemble to advance
	//Best to always disassemble and use it if someone cares
	UVDBFDInstruction m_instruction;
	
	SFILE m_sfile;
};

#endif

