/*
Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under the terms of the BSD license.  See LICENSE for details.
*/


/*
Original use was to store relocatable object files to aid in static analysis
*/

#include "uvd_data.h"
#include "uvd_relocation.h"
#include "uvd_types.h"
#include <string>
#include <elf.h>

class UVDElf;

/*
Supporting data to a specific section header entry
Ex:
	string table
	relocation table
	etc
*/
class UVDElfHeaderEntry
{
public:
	UVDElfHeaderEntry();
	virtual ~UVDElfHeaderEntry();
	
	//virtual uv_err_t getSize(int *size) = 0;
	
	virtual void setHeaderData(UVDData *data);
	//Get the header data representation
	virtual uv_err_t getHeaderData(UVDData **data);

	virtual void setFileData(UVDData *data);
	/*
	Get the data we need to save
	offset is the file offset it will start at
	*/
	virtual uv_err_t getFileData(UVDData **data);
	//virtual uv_err_t getFileData(UVDRelocatableDataMemory *data) = 0;
	//virtual uv_err_t getFileData(UVDRelocatableDataMemory **data) = 0;
	
public:
	//The actual data
	//Note that this contains the size and offset
	UVDData *m_headerData;
	//Supporting data in the file, if present
	UVDData *m_fileData;
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

	uv_err_t getName(std::string &sName);
	void setName(const std::string &sname);
	uv_err_t getName(int *nameIndex);
	void setName(int nameIndex);

	uv_err_t getType(int *type);
	void setType(int type);

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

public:
	//Name of this section
	std::string m_sName;
	//Section header for this element
	Elf32_Shdr m_sectionHeader;
};

class UVDElfStringTableSectionHeaderEntry : public UVDElfSectionHeaderEntry
{
public:
	UVDElfStringTableSectionHeaderEntry();
	~UVDElfStringTableSectionHeaderEntry();
	
	//Make sure s is in the string table.  Return the index if specified
	void addString(const std::string &s, unsigned int *index);

public:
	std::vector<std::string> m_stringTable;
};

/*
An ELF (Executable and Linker Format) reader/writter for working with relocatable object files
Focus will be on section header since original idea was only to do object file storing/loading for reloc
*/
class UVDElf
{
public:
	/*
	Creating and saving ELF objects
	*/
	//Blank ELF file
	UVDElf();
	~UVDElf();
	static uv_err_t getFromFile(const std::string &file, UVDElf **elf);
	uv_err_t loadFromFile(const std::string &file);
	//Get the raw binary image of this object
	uv_err_t constructBinary(UVDData **data);
	uv_err_t saveToFile(std::string &file);
	
	//Set the architecture
	//Should throw error on unknown architecture?
	uv_err_t getArchitecture(int *archOut);
	void setArchitecture(int archIn);

	int getProgramHeaderTableSize();
	int getSectionHeaderTableSize();

	uv_err_t addProgramHeaderSection(UVDElfProgramHeaderEntry *section);
	uv_err_t addSectionHeaderSection(UVDElfSectionHeaderEntry *section);
	
	uv_err_t getSectionHeaderByName(const std::string &sName, UVDElfSectionHeaderEntry **section);
	
	//Gaurantee string is in the table
	//If index is specified, it will be returned
	uv_err_t addString(const std::string &s, unsigned int *index = NULL);
	uv_err_t getStringIndex(const std::string &s, unsigned int *index);
	//For a given string, get an object to find its final address
	uv_err_t getStringRelocatableElement(const std::string &s, UVDRelocatableElement **relocatable, UVDRelocationManager *relocationManager);

	/*
	Factory function to create a blank conforming item
	Fills in basic header information and other peices
	*/
	static uv_err_t getUVDElf(UVDElf **out);
	
	uv_err_t getStringTableSectionHeaderEntry(UVDElfStringTableSectionHeaderEntry **sectionOut);

private:
	//If we loaded this, pointer to the raw data source
	UVDData *m_data;
	//Program header sections
	std::vector<UVDElfProgramHeaderEntry *> m_programHeaderEntries;
	//Section headers
	std::vector<UVDElfSectionHeaderEntry *> m_sectionHeaderEntries;
	//If m_e_ehsize is larger than sizeof(Elf32_Ehdr), we have extra reserved junk
	UVDData *m_elfHeaderExtra;
	//Raw elf header structure
	Elf32_Ehdr m_elfHeader;
};

