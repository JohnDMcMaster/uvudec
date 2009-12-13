/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#ifndef UVD_ELF_HEADER_H
#define UVD_ELF_HEADER_H

#include "uvd_data.h"
#include "uvd_elf_header.h"
#include "uvd_relocation.h"
#include "uvd_types.h"
#include <string>
#include <elf.h>

//Section names
//The section name for the section string table
#define UVD_ELF_SECTION_SECTION_STRING_TABLE			".shstrtab"
//The section name for the symbol string table
#define UVD_ELF_SECTION_SYMBOL_STRING_TABLE				".strtab"
//Symbol table
#define UVD_ELF_SECTION_SYMBOL_TABLE					".symtab"
//Executable data
#define UVD_ELF_SECTION_EXECUTABLE						".text"
/*
//Uninitialized data at program startup
#define UVD_ELF_SECTION_UNINITIALIZED_DATA				".bss"
//Dynamic linking information
#define UVD_ELF_SECTION_DYNAMIC_LINKING					".dynamic"
*/


/*
Supporting data to a specific section header entry
Ex:
	string table
	relocation table
	etc
*/
class UVDElf;
class UVDElfHeaderEntry
{
public:
	UVDElfHeaderEntry();
	virtual ~UVDElfHeaderEntry();
	
	//virtual uv_err_t getSize(int *size) = 0;
	
	//Initialize based on some raw data
	virtual uv_err_t setHeaderData(const UVDData *data) = 0;
	//Get the raw header data representation
	virtual uv_err_t getHeaderData(UVDData **data) = 0;

	virtual void setFileData(UVDData *data);
	/*
	Get the data we need to save
	offset is the file offset it will start at
	*/
	virtual uv_err_t getFileData(UVDData **data);
	//virtual uv_err_t getFileData(UVDRelocatableDataMemory *data) = 0;
	//virtual uv_err_t getFileData(UVDRelocatableDataMemory **data) = 0;
	virtual uv_err_t getSupportingDataSize(uint32_t *sectionSize) = 0;
	
	//Add a fixup location to the file data
	//void addFileRelocation(UVDRelocatableData *relocation);

	/*
	Add a ELF relocation that will appear in either .rel or .rela
	shtType valid values: SHT_REL, SHT_RELA 
	*/
	//void addElfRelocation(Elf32_Rel rel, int shtType);
	
public:
	//The actual data
	//Note that this contains the size and offset
	//UVDData *m_headerData;
	//Supporting data in the file, if present
	UVDData *m_fileData;
	//Relocations to the file data
	//std::vector<UVDRelocatableData *> m_fileDataRelocations;
	//Needed to do string lookups
	UVDElf *m_elf;
};

#if 0
typedef struct
{
  Elf32_Word	p_type;			/* Segment type */
  Elf32_Off	p_offset;		/* Segment file offset */
  Elf32_Addr	p_vaddr;		/* Segment virtual address */
  Elf32_Addr	p_paddr;		/* Segment physical address */
  Elf32_Word	p_filesz;		/* Segment size in file */
  Elf32_Word	p_memsz;		/* Segment size in memory */
  Elf32_Word	p_flags;		/* Segment flags */
  Elf32_Word	p_align;		/* Segment alignment */
} Elf32_Phdr;
#endif
class UVDElfProgramHeaderEntry : public UVDElfHeaderEntry
{
public:
	UVDElfProgramHeaderEntry();
	~UVDElfProgramHeaderEntry();

	//Raw
	uv_err_t getName(int *name);
	//Easy
	uv_err_t getName(std::string &sName);
	//Raw
	void setName(int name);
	//Easy
	void setName(const std::string &sName);

	uv_err_t getType(int *type);
	void setType(int type);

	virtual uv_err_t getHeaderData(UVDData **data);
	virtual uv_err_t setHeaderData(const UVDData *data);

	uv_err_t getSupportingDataSize(uint32_t *sectionSize);

#if 0
	uv_err_t getFlags(int *flags);
	void setFlags(int flags);

	uv_err_t getVirtualAddress(int *address);
	void setVirtualAddress(int address);

	//Usually section header link, but section dependent
	uv_err_t getSectionLink(int *index);
	void setSectionLink(int index);
	
	uv_err_t getInfo(int *info);
	void setInfo(int info);

	uv_err_t getAlignment(int *alignment);
	void setAlignment(int alignment);

	uv_err_t getTableEntrySize(int *entrySize);
	void setTableEntrySize(int entrySize);
#endif

public:
	//Section name
	//Only used if preparing to save
	std::string m_name;
	//Program header for this element
	Elf32_Phdr m_programHeader;
};

#if 0
Shared attributes
-sh_offset + sh_size / p_offset + p_filesz

typedef struct
{
  Elf32_Word	sh_name;		/*Section name (string tbl index) */
  Elf32_Word	sh_type;		/* Section type */
  Elf32_Word	sh_flags;		/* Section flags */
  Elf32_Addr	sh_addr;		/* Section virtual addr at execution */
  Elf32_Off	sh_offset;		/* Section file offset */
  Elf32_Word	sh_size;		/* Section size in bytes */
  Elf32_Word	sh_link;		/* Link to another section */
  Elf32_Word	sh_info;		/* Additional section information */
  Elf32_Word	sh_addralign;		/* Section alignment */
  Elf32_Word	sh_entsize;		/* Entry size if section holds table */
} Elf32_Shdr;
#endif
class UVDElfSectionHeaderEntry : public UVDElfHeaderEntry
{
public:
	UVDElfSectionHeaderEntry();
	~UVDElfSectionHeaderEntry();
 
 	//Specialized data structures are used for different sections
	static uv_err_t getUVDElfSectionHeaderEntryCore(const std::string &sSection, UVDElfSectionHeaderEntry **sectionHeaderOut);

	uv_err_t getName(std::string &sName);
	void setName(const std::string &sname);
	uv_err_t getName(int *nameIndex);
	void setName(int nameIndex);

	uv_err_t getType(int *type);
	void setType(int type);

	virtual uv_err_t getHeaderData(UVDData **data);
	virtual uv_err_t setHeaderData(const UVDData *data);

	uv_err_t getSupportingDataSize(uint32_t *sectionSize);

#if 0
	uv_err_t getVirtualAddress(int *address);
	void setVirtualAddress(int address);

	uv_err_t getPhysicalAddress(int *address);
	void setPhysicalAddress(int address);

	uv_err_t getMemorySize(int *size);
	void setMemorySize(int size);

	uv_err_t getFlags(int *flags);
	void setFlags(int flags);

	uv_err_t getAlignment(int *alignment);
	void setAligntment(int alignment);
#endif

public:
	//Name of this section
	std::string m_sName;
	//Section header for this element
	Elf32_Shdr m_sectionHeader;
};

/*
Used for string table based section headers:
.strtab
.shstrtab
*/
class UVDElfStringTableSectionHeaderEntry : public UVDElfSectionHeaderEntry
{
public:
	UVDElfStringTableSectionHeaderEntry();
	~UVDElfStringTableSectionHeaderEntry();
	
	//Make sure s is in the string table.  Return the index if specified
	void addString(const std::string &s, unsigned int *index);

	//String table must be recalculated
	uv_err_t getFileData(UVDData **data);
	//Only rebuilt when needed
	uv_err_t ensureCurrentStringTableData();

	uv_err_t getSupportingDataSize(uint32_t *sectionSize);

public:
	std::vector<std::string> m_stringTable;
};

#endif
