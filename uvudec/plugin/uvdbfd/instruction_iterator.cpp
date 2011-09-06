/*
UVNet Universal Decompiler (uvudec)
Copyright 2011 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, seinite COPYING for details
*/

#include <string>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "bfd.h"
#include "dis-asm.h"
#include "uvdbfd/instruction_iterator.h"
#include "uvdbfd/architecture.h"
#include "uvdbfd/object.h"
#include "uvd/core/runtime.h"
#include "uvd/core/uvd.h"


//struct disassemble_info disasm_info;
//bfd *g_bfd = NULL;


static void objdump_print_address(bfd_vma vma, struct disassemble_info *info)
{
	//printf("got an address request: 0x%08X\n", vma);
}

static int objdump_symbol_at_address(bfd_vma vma, struct disassemble_info * info)
{
	//printf("got a symbol request\n");
	return 0;
}

disassembler_ftype g_disassembler_function;



//XXX: if we become multithreaded we will have to lock this
static char g_print_buffer[256];

//static int ATTRIBUTE_PRINTF_2
static int objdump_sprintf(SFILE *f, const char *format, ...)
{
	/*
	Each one of these prints a single token / argument, not the whole line
	*/

printf("print func\n");
	size_t n;
	va_list args;

	va_start (args, format);
	//n = vprintf (format, args);
	n = vsnprintf(g_print_buffer, sizeof(g_print_buffer), format, args);
	//printf("\n");
	va_end (args);

	if (true) {
		va_start (args, format);
		vprintf (format, args);
		n = vsnprintf(g_print_buffer, sizeof(g_print_buffer), format, args);
		printf(" EOL\n");
		va_end (args);
	}

	return n;
}


UVDBfdInstructionIterator::UVDBfdInstructionIterator() {
	m_obj = NULL;
	m_section = NULL;
	m_curOffset = 0;
	m_maxOffset = 0;
	m_startOffset = 0;
	m_nextOffset = 0;
	memset(&m_disasm_info, 0, sizeof(m_disasm_info));
	m_data = NULL;
	m_datasize = 0;
	memset( &m_sfile, 0, sizeof(m_sfile));
}

UVDBfdInstructionIterator::~UVDBfdInstructionIterator() {
}

UVDBFDArchitecture *UVDBfdInstructionIterator::arch() {
	//return m_obj->m_uvd->m_runtime->m_arch;
	return dynamic_cast<UVDBFDArchitecture *>(g_uvd->m_runtime->m_architecture);
}

uv_err_t UVDBfdInstructionIterator::getPosition(UVDAddress *out) {
	uv_assert_ret(out);
	
	printf("Getting position, section: 0x%08X, %s\n", (int)m_section, m_section->name);
	uv_assert_err_ret(m_obj->bfdSectionToAddressSpace( m_section, &out->m_space ));
	out->m_addr = m_section->vma + m_curOffset;
	return UV_ERR_OK;
}

uv_err_t UVDBfdInstructionIterator::initByAddress(UVDBFDObject *obj, UVDAddress address) {
	m_obj = obj;
	
	//FIXME: ignoring address.m_addr
	uv_assert_err_ret(m_obj->addressSpaceToBfdSection( address.m_space, &m_section ));
	
	uv_assert_err_ret(init());
	return UV_ERR_OK;
}

uv_err_t UVDBfdInstructionIterator::initEnd(UVDBFDObject *obj, UVDAddressSpace *addressSpace) {
	m_obj = obj;
	
	//We will compare with the section and the address, so only care about those two
	//get is undefined when at end
	uv_assert_err_ret(m_obj->addressSpaceToBfdSection( addressSpace, &m_section ));
	uv_assert_err_ret(makeEnd());
	uv_assert_ret(m_section);
	return UV_ERR_OK;
}


uv_err_t UVDBfdInstructionIterator::makeEnd() {
	m_curOffset = m_maxOffset;
	return UV_ERR_OK;
}

bool UVDBfdInstructionIterator::isEnd() const {
	return m_curOffset == m_maxOffset;
}

uv_err_t UVDBfdInstructionIterator::next() {
	while( true ) {
		if( isEnd() ) {
			//Loop logic is actually handled here
			uv_assert_err_ret(initNextValidSection());
			break;
		} else {
			m_curOffset = m_nextOffset;
			uv_assert_err_ret(dissassembleCur());
			break;
		}
	}
	return UV_ERR_OK;
}

uv_err_t UVDBfdInstructionIterator::copy(UVDAbstractInstructionIterator **out) const {
	UVDBfdInstructionIterator *iter = NULL;
	
	iter = new UVDBfdInstructionIterator();
	uv_assert_ret(iter);
	
	//safest to just re-init
	iter->m_obj = m_obj;
	iter->m_startOffset = m_curOffset;
	uv_assert_err_ret(iter->init());
	
	/*
	//Most things are the same
	*iter = *this;
	//Buffer needs to be copied
	
	iter->m_data = (bfd_byte *)malloc(iter->m_datasize);
	uv_assert_ret(iter->m_data);
	iter->m_disasm_info.data = iter->m_data;
	*/
	
	return UV_ERR_OK;
}

int UVDBfdInstructionIterator::compare(const UVDAbstractInstructionIterator &otherIn) const {
	int rc = 0;
	const UVDBfdInstructionIterator *other = dynamic_cast<const UVDBfdInstructionIterator *>(&otherIn);

	uv_assert_err_ret(((UVDBfdInstructionIterator *)this)->check());
	printf("other: 0x%08X, otherIn: 0x%08X\n", (int)other, (int)&otherIn);
	fflush(stdout);

	//uv_assert_ret(other);
	rc = (int)m_section - (int)other->m_section;
	if( rc ) {
		return rc;
	}
	
	return m_curOffset - other->m_curOffset;
}

uv_err_t UVDBfdInstructionIterator::get(UVDInstruction **out) const {
	UVDInstruction *ret = NULL;
	
	uv_assert_ret(!isEnd());
	ret = new UVDBFDInstruction();
	
	uv_assert_ret(ret);
	*ret = m_instruction;
	return UV_ERR_OK;
}

uv_err_t UVDBfdInstructionIterator::init() {
	bfd *a_bfd = NULL;
	
	uv_assert_ret(m_obj);
	uv_assert_ret(m_obj->m_bfd);
	a_bfd = m_obj->m_bfd;
	
	//Needs to be an object or an archive
	if( !bfd_check_format(a_bfd, bfd_object)
			&& !bfd_check_format(a_bfd, bfd_archive) )
	{
		//bfd_close(a_bfd);
		printf("bad file\n");
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	printf("BFD open and is object\n");
	
	/*
	Standard disassemblers.  Disassemble one instruction at the given
   	target address.  Return number of octets processed.
	typedef int (*disassembler_ftype) (bfd_vma, disassemble_info *);
	*/
	g_disassembler_function = disassembler(a_bfd);
	if( !g_disassembler_function ) {
		printf("failed \n");
		return UV_DEBUG(UV_ERR_GENERAL);
	}

	memset(&m_disasm_info, 0, sizeof(m_disasm_info));
	init_disassemble_info(&m_disasm_info, stdout, (fprintf_ftype) fprintf);
	m_disasm_info.flavour = bfd_get_flavour (a_bfd);
	m_disasm_info.arch = bfd_get_arch (a_bfd);
	m_disasm_info.mach = bfd_get_mach (a_bfd);
	m_disasm_info.disassembler_options = NULL;
	m_disasm_info.octets_per_byte = bfd_octets_per_byte(a_bfd);
	m_disasm_info.skip_zeroes = 0;
	m_disasm_info.skip_zeroes_at_end = 0;
	m_disasm_info.disassembler_needs_relocs = FALSE;
	m_disasm_info.display_endian = m_disasm_info.endian = BFD_ENDIAN_BIG;

	//Don't care
	m_disasm_info.application_data = NULL;
	m_disasm_info.print_address_func = objdump_print_address;
	m_disasm_info.symbol_at_address_func = objdump_symbol_at_address;

	disassemble_init_for_target(&m_disasm_info);

	//Wonder if these can be NULL?
	m_disasm_info.symtab = NULL;
	m_disasm_info.symtab_size = 0;

	/*
	This won't work since we want to step on our own accord
	bfd_map_over_sections(a_bfd, disassemble_section, &disasm_info);
	*/
#if 0
void
bfd_map_over_sections (bfd *abfd,
		       void (*operation) (bfd *, asection *, void *),
		       void *user_storage)
{
  asection *sect;
  unsigned int i = 0;

  for (sect = abfd->sections; sect != NULL; i++, sect = sect->next)
    (*operation) (abfd, sect, user_storage);

  if (i != abfd->section_count)	/* Debugging */
    abort ();
}
#endif

	//Start walking the sections linked list if we weren't given a starting position
	if( m_section == NULL ) {
		m_section = m_obj->m_bfd->sections;
	}
	uv_assert_err_ret(initNextValidSection());
	
	return UV_ERR_OK;
}


uv_err_t UVDBfdInstructionIterator::doNextSection() {
	m_section = m_section->next;
	return UV_ERR_OK;
}

uv_err_t UVDBfdInstructionIterator::nextSection() {
	uv_err_t rcTemp = UV_ERR_GENERAL;
	/*
	if( m_section == NULL ) {
		return UV_ERR_DONE;
	}
	*/
	uv_assert_err_ret(doNextSection());
	rcTemp = initNextValidSection();
	if( rcTemp == UV_ERR_DONE ) {
		return rcTemp;
	}
	uv_assert_err_ret(rcTemp);
	return UV_ERR_OK;
}

uv_err_t UVDBfdInstructionIterator::initNextValidSection() {
	while( true ) {
  		//bfd_size_type datasize = 0;
  		uv_err_t rcTemp = UV_ERR_GENERAL;
  
  		//end() should be at end of last valid section
  		//it would have been an error to try to advance this
  		uv_assert_ret(m_section);
  		
		//Only keep going as long as there are more candidates
		if( m_section == NULL ) {
			return UV_ERR_DONE;
		}
		
		printf("init next valid, trying %s\n", m_section->name);
		rcTemp = initCurrentSection();
		if( rcTemp == UV_ERR_DONE ) {	
			//Not a good section, try again
			uv_assert_err_ret(doNextSection());
			continue;
		}
		//Make sure it didn't truely error
		uv_assert_err_ret(rcTemp);
		//Otherwise we are ready!
		break;
	}
	
	return UV_ERR_OK;
}

uv_err_t UVDBfdInstructionIterator::initCurrentSection()
{
	bfd *abfd = m_obj->m_bfd;
	asection *section = m_section;

	//const struct elf_backend_data * bed;
	struct disassemble_info *    pinfo = &m_disasm_info;
	unsigned int                 opb = pinfo->octets_per_byte;
	bfd_byte *                   data = NULL;
	bfd_size_type                datasize = 0;
	//long                         place = 0;
	//unsigned long                addr_offset;

	/* Sections that do not contain machine
	code are not normally disassembled.  */
	if ((section->flags & (SEC_CODE | SEC_HAS_CONTENTS)) != (SEC_CODE | SEC_HAS_CONTENTS)) {
		return UV_ERR_DONE;
	}

	datasize = bfd_get_section_size (section);
	m_datasize = datasize;
	if (datasize == 0) {
		return UV_ERR_DONE;
	}

	data = (bfd_byte *)malloc(datasize);
	uv_assert_ret(data);
	m_data = data;
	bfd_get_section_contents (abfd, section, data, 0, datasize);

	pinfo->buffer = data;
	pinfo->buffer_vma = section->vma;
	pinfo->buffer_length = datasize;
	pinfo->section = section;

	m_startOffset = 0;
	m_curOffset = m_startOffset;
	m_maxOffset = datasize / opb;

	printf ("\nDisassembly of section %s\n", section->name);

	//m_curOffset = start_offset;
	
	
	
	
	
	
	//bfd_vma start_offset = m_curOffset;
	//printf("section: 0x%08X:0X%08x\n", start_offset, m_maxOffset);

	//section = aux->sec;

	m_sfile.alloc = 120;
	m_sfile.buffer = (char *) malloc(m_sfile.alloc);
	m_sfile.pos = 0;

	pinfo->insn_info_valid = 0;
	
	
	uv_assert_err_ret(dissassembleCur());
	
	//free (data);
	return UV_ERR_OK;
}

uv_err_t UVDBfdInstructionIterator::dissassembleCur()
{
	struct disassemble_info * info = &m_disasm_info;

	//asection *section;
	//bfd_vma addr_offset;
	unsigned int opb = info->octets_per_byte;
	int octets = opb;

	uv_assert_ret( m_curOffset < m_maxOffset );

	octets = 0;

	m_sfile.pos = 0;
	info->fprintf_func = (fprintf_ftype) objdump_sprintf;
	info->stream = &m_sfile;
	info->bytes_per_line = 0;
	info->bytes_per_chunk = 0;
	info->flags = DISASSEMBLE_DATA;

	unsigned int target_address = m_section->vma + m_curOffset;
	//target_address = 0x8049558;
	//printf("target address: 0x%08X\n", target_address);
	octets = (*g_disassembler_function) (target_address, info);
	//printf("\noctets: %d\n", octets);
	info->fprintf_func = (fprintf_ftype) fprintf;
	info->stream = stdout;
	if (octets < 0)
	{
		//error for now figure out what its needed for
		if (m_sfile.pos) {
			printf (" SFILE print: %s\n", m_sfile.buffer);
		}
		return UV_DEBUG(UV_ERR_GENERAL);
	}

	m_instruction.m_disassembly = g_print_buffer;

	printf("\n");

	m_nextOffset = m_curOffset + octets / opb;

	//free (sfile.buffer);
	return UV_ERR_OK;
}

uv_err_t UVDBfdInstructionIterator::check() {
	printf("BFD checking\n");
	uv_assert_ret(m_obj);
	uv_assert_ret(m_section);
	return UV_ERR_OK;
}

