/*
Red Plait`s pattern maker
Copyrightish 2010 John McMaster <JohnDMcMaster@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, 51 Franklin Street - Fifth Floor, Boston,
MA 02110-1301, USA.

Original copyright below
*/

/*
 * Red Plait`s pattern maker
 * Based on objdump from cool library binutils v2.9.1
 *
 * 13-IX-1999   Red Plait (redplait@usa.net)
 * Ilfuck - greedy pig 
 *      (C) I am
 */

/*
Fixups: 
- Corrected headers 
- Added Makefile...a pain to link
- struct sec -> struct bfd_section
	http://sourceware.org/ml/binutils/2003-10/msg00500.html - _raw_size
	http://sourceware.org/ml/binutils/2003-10/msg00562.html Instead of Use (read) sec->_raw_size bfd_section_size (abfd, 
	sec) (or in relaxing targets and just when referring to original size:) bfd_unaltered_section_size (abfd, sec)
- well behaved C does not have "true", used "TRUE" instead 
- don't use anything in the $(BINUTILS_DIR)/include
*/

#include <bfd.h>
// We shouldn't tbe using bucomm
// its linked like ex this for c++filt:
// gcc -W -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror -g -O2 -o cxxfilt cxxfilt.o bucomm.o version.o
// filemode.o ../bfd/.libs/libbfd.a ../libiberty/libiberty.a
// ie its not exposed into the library
// xmalloc, bfd_fatal, bfd_nonfatal
// interestingly enough these funcs seem to be defined and I can't include this file
// #include "bucomm.h"
// #include "demangle.h"
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#if 0
#ifndef DEMANGLE_H
#define DMGL_ANSI	 (1 << 1)	/* Include const, volatile, etc */
#define DMGL_PARAMS	 (1 << 0)	/* Include function args */
#endif
#endif

// #define BYTES_IN_WORD 32
// #include "aout/aout64.h"

#define RP_DEBUG

#ifdef RP_DEBUG
#define printf_debug(format, ...) do { if( verbose ) { printf(format, ## __VA_ARGS__); } } while(0)
#define fprintf_debug(fd, format, ...) do { if( verbose ) { fprintf(fd, format, ## __VA_ARGS__); } } while(0)
#else
#define printf_debug(...)
#define fprintf_debug(...)
#endif

#define printf_error printf
#define printf_help printf

#define printf_verbose(format, ...) do { if( verbose ) { printf(format, ## __VA_ARGS__); } } while(0)

// extern unsigned short crc16(unsigned char *, unsigned short);
#define POLY 0x8408	/* 1021H bit reversed */
uint16_t crc16(const char *data_p, uint32_t length)
{
	uint8_t i;
	uint16_t data;
	uint16_t crc = 0xffff;

	if(length == 0)
		return (~crc);
	do
	{
		for(i = 0, data = (unsigned int) 0xff & *data_p++; i < 8; i++, data >>= 1)
		{
			if((crc & 0x0001) ^ (data & 0x0001))
				crc = (crc >> 1) ^ POLY;
			else
				crc >>= 1;
		}
	} while(--length);

	crc = ~crc;
	data = crc;
	crc = (crc << 8) | (data >> 8 & 0xff);

	return (crc);
}

/*
bucomm compatibility 
*/
char *program_name;

void bfd_nonfatal(const char *string)
{
	const char *errmsg = bfd_errmsg(bfd_get_error());

	if(string)
		printf_error("%s: %s: %s\n", program_name, string, errmsg);
	else
		printf_error("%s: %s\n", program_name, errmsg);
}

void bfd_fatal(const char *string)
{
	bfd_nonfatal(string);
	exit(1);
}

/*
void fatal VPARAMS ((const char *format, ...)) { VA_OPEN (args, format); VA_FIXEDARG (args, const char *, format);

report (format, args); VA_CLOSE (args); xexit (1); } 
*/
void fatal(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	printf("fatal: ");
	vprintf(format, ap);
	printf("\n");
	va_end(ap);

	exit(1);
}

void list_matching_formats(char **p)
{
	printf_error("%s: Matching formats:", program_name);
	while(*p)
		printf_error(" %s", *p++);
	fputc('\n', stderr);
}

/*
The macro TARGET is suppose to be defined by Makefile.  
*/
#define TARGET "i686-pc-linux-gnu"
void set_bfd_target(const char *target)
{
	if(!bfd_set_default_target(target))
		fatal("can't set BFD target to `%s': %s", target, bfd_errmsg(bfd_get_error()));
}





static long symcount = 0;	/* Number of symbols in `syms'.  */
static int do_demangle = 0;	/* Demangle names */
static int skip_z = 0;	/* Skip zeroes */

const char *noname = "(NULL)";
const char *version = "0.3.1-uv";

//FLAIR imposed restriction on max function size 
#define FLAIR_MAXLEN	0x8000
#define DEFAULT_PATLEN	32
static long pat_len = DEFAULT_PATLEN;

int is_depend = 1;	/* include reference info */
int verbose = 0;	/* verbose messaged */
int function_only = 1;	/* include info on functions only */
FILE *out = NULL;

struct reloc_chain
{
	struct reloc_chain *next;
	char *name;
	int address;
	int size;
	int offset;	/* offset relative function but not section ! */
};

struct function_chain
{
	struct function_chain *next;
	asymbol *sym;
	struct reloc_chain *reloc;
	int offset;
	int size;
};

struct section_chain
{
	struct section_chain *next;
	struct bfd_section *section;
	struct function_chain *funcs;
	unsigned char *content;
};

#define check_name(s)	((NULL != s) && s[0])

void print_name(bfd * abfd, FILE * out, const char *name)
{
#if DO_DEMANGLE
	if(do_demangle)	/* Demangle the name.  */
	{
		char *alloc = NULL;
		if(bfd_get_symbol_leading_char(abfd) == name[0])
			++name;
		//This is libiterty internal
		alloc = cplus_demangle(name, DMGL_ANSI | DMGL_PARAMS);
		if(alloc != NULL)
		{
			int i;
			for(i = strlen(alloc); i; i--)
				if(' ' == alloc[i - 1])
					alloc[i - 1] = '_';
			name = alloc;

			free(alloc);
		}
	}
#endif
	fprintf(out, "%s ", name);
}

struct section_chain *find_section(struct section_chain *first, struct bfd_section *s)
{
	for(; first != NULL; first = first->next)
	{
		if((NULL != first->section->name) && (NULL != s->name) && !strcmp(first->section->name, s->name))
			return first;
		if(s == first->section)
			return first;
	}
	return NULL;
}

/*
Add section "s" to chain "first"
Returns a pointer to our linked list entry with section "s"
If the section already exists, it is not re-added
*/
struct section_chain *apply_section(bfd * abfd, struct section_chain **first, struct bfd_section *s, long section_size)
{
	struct section_chain *ptr;

	if(NULL == *first)
	{
		*first = (struct section_chain *) malloc(sizeof(struct section_chain));
		(*first)->next = NULL;
		(*first)->funcs = NULL;
		(*first)->section = s;
		// xmalloc is a malloc wrapper that exits upon an alloc error
		// Also apparantly makes as allocate at least 1 byte to avoid malloc returning null for 0 bytes in
		(*first)->content = (unsigned char *) malloc(section_size);
		bfd_get_section_contents(abfd, s, (PTR) (*first)->content, 0, section_size);
		return *first;
	}
	for(ptr = *first;; ptr = ptr->next)
	{
		if((NULL != ptr->section->name) && (NULL != s->name) && !strcmp(ptr->section->name, s->name))
			return ptr;
		if(s == ptr->section)
			return ptr;
		if(NULL == ptr->next)
			break;
	}
	ptr->next = (struct section_chain *) malloc(sizeof(struct section_chain));
	ptr = ptr->next;
	ptr->next = NULL;
	ptr->funcs = NULL;
	ptr->section = s;
	ptr->content = (unsigned char *) malloc(section_size);
	bfd_get_section_contents(abfd, s, (PTR) ptr->content, 0, section_size);
	return ptr;
}

struct reloc_chain *free_reloc(struct reloc_chain *r)
{
	struct reloc_chain *res = NULL;
	if(NULL == r)
		return (struct reloc_chain *) NULL;
	res = r->next;
	if(r->name != NULL)
		free(r->name);
	free(r);
	return res;
}

void kill_relocs(struct reloc_chain *first_r)
{
	while(first_r != NULL)
		first_r = free_reloc(first_r);
}

void kill_functions(struct function_chain *first)
{
	struct function_chain *ptr;
	while(first != NULL)
	{
		ptr = first->next;
		if(NULL != first->reloc)
			kill_relocs(first->reloc);
		free(first);
		first = ptr;
	}
}

void kill_sections(struct section_chain *first)
{
	struct section_chain *ptr;
	while(NULL != first)
	{
		ptr = first->next;
		if(first->funcs)
			kill_functions(first->funcs);
		if(NULL != first->content)
			free(first->content);
		free(first);
		first = ptr;
	}
}

//Insert into list sorted by f's value / chain entrie's offset member
struct function_chain *insert_function(struct section_chain *s, asymbol * f)
{
	struct function_chain *prev = NULL;
	struct function_chain *ptr, *f2;

	for(ptr = s->funcs; NULL != ptr; ptr = ptr->next)
	{
		if(f->value < ptr->offset)
			break;
		prev = ptr;
	}
	/*
	we needed add this function before prev 
	*/
	f2 = (struct function_chain *) malloc(sizeof(struct function_chain));
	f2->sym = f;
	f2->offset = f->value;
	f2->reloc = NULL;
	f2->size = 0;
	f2->next = ptr;
	if(prev != NULL)
		prev->next = f2;
	else
		s->funcs = f2;
	return f2;
}

//Sort by address
//Returns NULL on failed add, the reloc chain entry we added in event of success
struct reloc_chain *add_reloc(struct function_chain *func, arelent * rel)
{
	struct reloc_chain *prev = NULL;
	struct reloc_chain *ptr, *r2;

	if(rel->address < func->offset || rel->address > (func->offset + func->size))
		return NULL;
	for(ptr = func->reloc; NULL != ptr; ptr = ptr->next)
	{
		if(rel->address < ptr->address)
			break;
		prev = ptr;
	}
	/*
	we needed add new reloc before prev 
	*/
	r2 = (struct reloc_chain *) malloc(sizeof(struct reloc_chain));
	r2->size = rel->howto->bitsize >> 3;
	r2->address = rel->address;
	r2->offset = rel->address - func->offset;
	r2->name = NULL;
	if(rel->sym_ptr_ptr != NULL)
	{
		int flags = (*rel->sym_ptr_ptr)->flags;
		if(!(flags & BSF_SECTION_SYM))
			r2->name = strdup((*rel->sym_ptr_ptr)->name);
	}
	r2->next = ptr;
	if(prev != NULL)
		prev->next = r2;
	else
		func->reloc = r2;
	return r2;
}

/*
Remove zeroed data at start of data in section_chain s
We must adjust all realloc's in the function list to point to correct locations 
*/
int skip_zeros(struct section_chain *s, struct function_chain *f)
{
	int i;
	struct reloc_chain *rr;

	if(!skip_z)
		return 0;
	//Count down bytes we still have left, stopping when we have non-zero (valid) data
	for(i = f->size; i; i--)
	{
		if(s->content[f->offset + i - 1])
			break;
	}
	f->size = i;
	//Kill any reallocs before our non-zero data starts
	for(i = 0; i < f->size;)
	{
		if((f->reloc != NULL) && f->reloc->offset == i)
		{
			i += f->reloc->size;
			f->reloc = free_reloc(f->reloc);
			continue;
		}
		if(s->content[f->offset + i])
			break;
		i++;
	}
	//Did we finish everything?
	if(!i)
		return 0;
	/*
	aha - we must shrink size and readjust relocs 
	*/
	f->size -= i;
	f->offset += i;
	for(rr = f->reloc; rr != NULL; rr = rr->next)
	{
		rr->offset -= i;
	}
	return 0;
}

/*
Assign function sizes
*/
void assign_sizes(bfd * abfd, struct section_chain *first)
{
	struct section_chain *ptr;
	struct function_chain *f_ptr;
	int section_size;

	//Iterate over all sections
	for(ptr = first; NULL != ptr; ptr = ptr->next)
	{
		section_size = bfd_section_size(abfd, ptr->section);
		//Iterate over all functions within that section
		for(f_ptr = ptr->funcs; NULL != f_ptr; f_ptr = f_ptr->next)
		{
			if(NULL == f_ptr->next)
			{
				f_ptr->size = section_size - f_ptr->offset;
			}
			else
			{
				f_ptr->size = f_ptr->next->offset - f_ptr->offset;
			}
		}
		/*
		and now slowdown our appetite
		If any functions hit our max function length, we must truncate their signature 
		*/
		for(f_ptr = ptr->funcs; NULL != f_ptr; f_ptr = f_ptr->next)
		{
			if(f_ptr->size >= FLAIR_MAXLEN)
				f_ptr->size = FLAIR_MAXLEN - 1;
		}
	}
}

/*
Add the relocation to appropriete function within the chain
Not very efficient...just slams on each entry until we hit the correct one
But maybe thats best we can do since we are using linked list
*/
void assign_rel(struct section_chain *s, arelent * r)
{
	struct function_chain *f;

	for(f = s->funcs; NULL != f; f = f->next)
	{
		//Successfully added?  We hit the correct range then
		if(NULL != add_reloc(f, r))
			return;
	}
}

void process_file(bfd * abfd)
{
	//The symbol table
	asymbol **syms = NULL;
	char **matching;
	long storage;
	int count;
	asymbol **current;
	struct section_chain *first_s = NULL, *s;
	struct function_chain *f;
	asection *a;
	struct reloc_chain *rr;
	int curr_len, up_to;

	printf_verbose("Processing file %s\n", bfd_get_filename(abfd));
	if(!bfd_check_format_matches(abfd, bfd_object, &matching))
	{
		bfd_nonfatal(bfd_get_filename(abfd));
		if(bfd_get_error() == bfd_error_file_ambiguously_recognized)
		{
			list_matching_formats(matching);
			free(matching);
		}
		return;
	}
	if( verbose )
	{
		if( !matching )
		{
			printf_error("Matching list NULL?\n");
		}
		else
		{
			char **cur_match;
			
			for( cur_match = matching; *matching; ++cur_match )
			{
				printf_error("matched format: %s\n", *matching);
			}
		}
	}
	
	if(!(bfd_get_file_flags(abfd) & HAS_SYMS))
	{
		printf_error("No symbols in \"%s\".\n", bfd_get_filename(abfd));
		printf_error("flags: 0x%.8X\n", bfd_get_file_flags(abfd));
		return;
	}
	storage = bfd_get_symtab_upper_bound(abfd);
	if(storage < 0)
		bfd_fatal(bfd_get_filename(abfd));
	if(storage)
		syms = (asymbol **) malloc(storage);
	symcount = bfd_canonicalize_symtab(abfd, syms);
	if(symcount < 0)
	{
		bfd_fatal(bfd_get_filename(abfd));
		if(NULL != syms)
			free(syms);
		return;
	}
	if(symcount == 0)
	{
		printf_error("%s: No symbols\n", bfd_get_filename(abfd));
		if(NULL != syms)
			free(syms);
		return;
	}
	current = syms;
	for(count = 0; count < symcount; count++, current++)
	{
		if(*current)
		{
			int flags;
			long section_size;

			bfd *cur_bfd = bfd_asymbol_bfd(*current);
			if(cur_bfd != NULL)
			{
				flags = (*current)->flags;
				if(flags & BSF_WEAK)	/* I don`t need aliases */
					continue;
				printf_verbose(":Name %s flags 0x%X, value 0x%X\n",
							   (*current)->name, flags, (unsigned int)(*current)->value);
				if(NULL == (*current)->section)
					continue;
				section_size = bfd_section_size(abfd, (*current)->section);
				if(section_size <= 0)
					continue;
				s = apply_section(abfd, &first_s, (*current)->section, section_size);
				insert_function(s, *current);
			}
		}
	}
	assign_sizes(abfd, first_s);
	/*
	   next I had to process relocations 
	 */
	for(a = abfd->sections; a != NULL; a = a->next)
	{
		long relsize, relcount;
		arelent **relpp, **p;
		struct section_chain *apply_sec;

		if(bfd_is_abs_section(a))
			continue;
		if(bfd_is_und_section(a))
			continue;
		if(bfd_is_com_section(a))
			continue;
		if(!(a->flags & SEC_RELOC))
			continue;
		relsize = bfd_get_reloc_upper_bound(abfd, a);
		if(relsize < 0)
			bfd_fatal(bfd_get_filename(abfd));
		if(!relsize)
			continue;
		apply_sec = find_section(first_s, a);
		if(NULL == apply_sec)
			continue;
		printf_verbose("Reloc section name is %s\n", a->name);
		relpp = (arelent **) malloc(relsize);
		relcount = bfd_canonicalize_reloc(abfd, a, relpp, syms);
		if(relcount <= 0)
		{
			free(relpp);
			continue;
		}
		for(p = relpp; relcount && *p != (arelent *) NULL; p++, relcount--)
		{
			arelent *q = *p;
			assign_rel(apply_sec, q);
			/*
			   RP 
			 */
			if(q->sym_ptr_ptr != NULL)
			{
				printf_debug("Reloc: name %s vma %ld, flags 0x%X\n",
					   (*q->sym_ptr_ptr)->name ? (*q->sym_ptr_ptr)->name : noname,
					   bfd_asymbol_value((*q->sym_ptr_ptr)), (unsigned int)(*q->sym_ptr_ptr)->flags);
			}
			printf_debug("Reloc: addres 0x%X, addend 0x%X, bitsize %d\n", (unsigned int)q->address, (unsigned int)q->addend, q->howto->bitsize);
			/*
			   -RP- 
			 */
		}
		free(relpp);
	}
	/*
	   main processing - I want a result ! 
	 */
	for(s = first_s; NULL != s; s = s->next)
	{
		int flags;
		printf_verbose("Section: %s, size 0x%X\n",
				   s->section->name ? s->section->name : noname, (unsigned int)bfd_section_size(abfd, s->section));
		for(f = s->funcs; NULL != f; f = f->next)
		{
			struct reloc_chain *r;
			int dots;
			/*
			   we must filter here - to don`t lose sizes ! 
			 */
			if(!check_name(f->sym->name))
				continue;
			flags = f->sym->flags;
			if(BSF_EXPORT != (flags & BSF_EXPORT))
				continue;
			if((BSF_SECTION_SYM & flags) || (BSF_FILE & flags))
				continue;	/* I don`t need signature on whole section and files */
			if(!(!function_only || ((flags & BSF_FUNCTION) || (flags & BSF_OBJECT))))
				continue;
			/*
			   now we have exported function 
			 */
			printf_debug("Section name %s, size %d, vma = 0x%X\n",
				   (*current)->section->name, (unsigned int)bfd_section_size(abfd, (*current)->section), (unsigned int)(*current)->section->vma);

			printf_verbose("Name: %s, base %d value 0x%X, flags 0x%X, size 0x%X\n",
					   bfd_asymbol_name(f->sym), (unsigned int)bfd_asymbol_base(f->sym),
					   (unsigned int)bfd_asymbol_value(f->sym), f->sym->flags, f->size);
			if(verbose)
				for(r = f->reloc; NULL != r; r = r->next)
				{
					printf_verbose("Reloc: name %s size %d address 0x%X offset 0x%X\n",
						   r->name ? r->name : noname, r->size, r->address, r->offset);
				}
			curr_len = skip_zeros(s, f);
			if(f->size < pat_len)	/* bad - we have too short functions */
			{
				printf_verbose("Too short - skipping...\n");
				continue;
			}
			rr = f->reloc;
			if(rr != NULL)
				up_to = rr->offset;
			else
				up_to = f->size;
			while(curr_len < pat_len)
			{
				if(curr_len < up_to)
				{
					fprintf(out, "%02X", s->content[curr_len++ + f->offset]);
					continue;
				}
				for(dots = 0; dots < rr->size; curr_len++, dots++)
				{
					if(!(curr_len < pat_len))
						break;
					fprintf(out, "..");
				}
				if(!(curr_len < pat_len))
					break;
				rr = rr->next;
				if(rr != NULL)
					up_to = rr->offset;
				else
					up_to = f->size;
			}
			fprintf(out, " ");
			fprintf_debug(out, "curr_len 0x%X, up_to 0x%X\n", curr_len, up_to);
			if((up_to < curr_len) && NULL != rr && ((up_to + rr->size) > curr_len))
				fprintf(out, "00 0000 ");
			else
			{
				unsigned short crc_len;
				if(up_to < curr_len && NULL != rr)
				{
					rr = rr->next;
					if(NULL != rr)
						up_to = rr->offset;
					else
						up_to = f->size;
				}
				else if(NULL == rr)
					up_to = f->size;
				crc_len = (unsigned short) ((up_to - curr_len) & 0xffff);
				printf_debug("CRC_LEN %02x", crc_len);
				if(crc_len > 0xff)
					crc_len = 0xff;
				fprintf(out, "%02X %04X ", crc_len, crc16((char *)(s->content + curr_len), crc_len));
				curr_len += crc_len;
			}
			fprintf(out, "%04X :0000 ", f->size);
			print_name(abfd, out, bfd_asymbol_name(f->sym));
			if(is_depend)
			{
				for(r = f->reloc; NULL != r; r = r->next)
				{
					if(check_name(r->name))
					{
						fprintf(out, "^%04X ", r->offset);
						print_name(abfd, out, r->name);
					}
				}
			}
			if(up_to < curr_len)
			{
				for(; (dots < rr->size) && (curr_len < f->size); dots++, curr_len++)
					fprintf(out, "..");
				rr = rr->next;
				if(rr != NULL)
					up_to = rr->offset;
				else
					up_to = f->size;
			}
			fprintf_debug(out, "curr_len 0x%X rr %p up_to 0x%X\n", curr_len, rr, up_to);

			while(curr_len < f->size)
			{
				if(curr_len < up_to)
				{
					fprintf(out, "%02X", s->content[curr_len + f->offset]);
					curr_len++;
					continue;
				}
				for(dots = 0; dots < rr->size && curr_len < f->size; curr_len++, dots++)
					fprintf(out, "..");
				rr = rr->next;
				if(rr != NULL)
					up_to = rr->offset;
				else
					up_to = f->size;
			}
			fprintf(out, "\n");
		}
	}
	if(NULL != syms)
		free(syms);
	kill_sections(first_s);
}

void process_filez(const char *filename)
{
	bfd *file;
	bfd *arfile = (bfd *) NULL;

	file = bfd_openr(filename, "default");
	if(NULL == file)
	{
		bfd_nonfatal(filename);
		return;
	}
	//If we get an archive, we must recurse
	if(bfd_check_format(file, bfd_archive) == TRUE)
	{
		bfd *last_arfile = NULL;
		//Recursive for each file in the archive
		for(;;)
		{
			bfd_set_error(bfd_error_no_error);

			arfile = bfd_openr_next_archived_file(file, arfile);
			if(arfile == NULL)
			{
				if(bfd_get_error() != bfd_error_no_more_archived_files)
				{
					bfd_nonfatal(bfd_get_filename(file));
				}
				break;
			}
			printf_verbose("File: %s\n", arfile->filename);
			process_file(arfile);
			if(last_arfile != NULL)
				bfd_close(last_arfile);
			last_arfile = arfile;
		}
		if(last_arfile != NULL)
			bfd_close(last_arfile);
	}
	//Otherwise, process
	else
		process_file(file);
	bfd_close(file);
}

void displayTargets()
{
	const char **targetList;
	
	printf_help("Supported formats:");
	for( targetList = bfd_target_list(); *targetList; ++targetList )
	{
		printf_help(" %s", *targetList);				
	}
	printf_help("\n");
}

void usage()
{
	printf_help("Usage: pat [-p pattern_len] [-o filename] [-a] file ...\n");
	printf_help("\t-C Demangle names\n");
	printf_help("\t-a Append to output file\n");
	printf_help("\t-g Include all global object (not functions only)\n");
	printf_help("\t-o Sets output file name\n");
	printf_help("\t-p Pattern length\n");
	printf_help("\t-r Dont include reference to other functions\n");
	printf_help("\t-v Verbose mode (useful for developers only)\n");
	printf_help("\t-z Skip zeroes\n");
	printf_help("Version: %s\n", version);
	displayTargets();
	exit(3);
}

int main(int argc, char **argv)
{
	const char *openflags[] = {
		"w",
		"a"
	};

	int c;
	int is_append = 0;
	char *outfilename = NULL;
	char target[32] = "";

	program_name = *argv;

	bfd_init();

	while(EOF != (c = getopt(argc, argv, "b:p:o:Carvgz")))
	{
		switch (c)
		{
		case 'b':
			strncpy(target, optarg, sizeof(target));
			break;
		case 'C':
			do_demangle = 1;
			break;
		case 'a':
			is_append = 1;
			break;
		case 'g':
			function_only = 0;
			break;
		case 'p':
			pat_len = atoi(optarg);
			if(pat_len <= 0)
			{
				printf_error("Waring - pattern length has bad value, changed to %d\n", DEFAULT_PATLEN);
				pat_len = DEFAULT_PATLEN;
			}
			break;
		case 'o':
			outfilename = optarg;
			break;
		case 'r':
			is_depend = 0;
			break;
		case 'v':
			verbose = 1;
			break;
		case 'z':
			skip_z = 1;
			break;
		default:
			usage();
		}
	}
	if(!(optind < argc))
		usage();
	if( target[0] )
	{
		set_bfd_target(target);
	}
	else
	{
		set_bfd_target(TARGET);
	}
	if(outfilename != NULL)
	{
		if(NULL == (out = fopen(outfilename, openflags[is_append])))
		{
			printf_error("Cannot open output file %s on %s\n", outfilename, openflags[is_append]);
			exit(2);
		}
	}
	else
		out = stdout;
	for(; optind < argc; optind++)
	{
		process_filez(argv[optind]);
	}
	fprintf(out, "---\n");
	if(out != NULL && out != stdout)
		fclose(out);
	return 0;
}
