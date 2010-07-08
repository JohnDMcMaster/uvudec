/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
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
//"No section", although this does exist as a section presumably for efficency reasons
#define UVD_ELF_SECTION_NULL					""
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
	
	virtual uv_err_t init();
	virtual uv_err_t initRelocatableData();

	//virtual uv_err_t getSize(int *size) = 0;
	
	//Initialize based on some raw data
	virtual uv_err_t setHeaderData(const UVDData *data) = 0;
	//Get the raw header data representation
	//It is callers responsibility to free the data
	//but it should probably be changed to be this objects responsibility
	virtual uv_err_t getHeaderData(UVDData **data) = 0;

	virtual void setFileData(UVDData *data);
	/*
	Get the data we need to save
	data will be free'd when this object is free'd
	Should only be called by UVDElfWritter
	*/
	virtual uv_err_t getFileData(UVDData **data);
	virtual uv_err_t getFileRelocatableData(UVDRelocatableData **supportingData);
	//On many sections its easier to return a collection of relocatable data
	//supportingData will not be cleared
	//use a UVDMultiRelocatableData instead
	//virtual uv_err_t getFileRelocatableData(std::vector<UVDRelocatableData *> &supportingData);
	//virtual uv_err_t getFileData(UVDRelocatableDataMemory *data) = 0;
	//virtual uv_err_t getFileData(UVDRelocatableDataMemory **data) = 0;
	virtual uv_err_t getSupportingDataSize(uint32_t *sectionSize) = 0;
	//Update data so a standard above function can be used
	//This should give enough information, in particular sizing, so that offsets can be calculated to place sections
	//virtual uv_err_t updateData();
	//virtual uv_err_t syncDataAfterUpdate();
	//After all sections have been placed, fill in file offsets
	//virtual uv_err_t applyRelocations();
	
	//Add a fixup location to the file data
	//void addFileRelocation(UVDRelocatableData *relocation);

	/*
	Add a ELF relocation that will appear in either .rel or .rela
	shtType valid values: SHT_REL, SHT_RELA 
	*/
	//void addElfRelocation(Elf32_Rel rel, int shtType);
	
	//Final update after updateData() has been called on all headers to fill in relocation information
	virtual uv_err_t updateForWrite();
	virtual uv_err_t constructForWrite();
	virtual uv_err_t applyRelocationsForWrite();
	
	/*
	Issue 1: UVDMultiRelocatableData is automatically synced and cannot have the data set on it
	Issue 2: could manually sync at end of constructForWrite(), but error prone to forget and should always be done
	Possible solution : call from uvd_elf_write.cpp to ensure sync after update.  Will be filtered out as subclass thinks appropriete
	Solution for now: requiresDataSync() function
	*/
	//virtual uv_err_t syncDataAfterConstruct();
	
protected:
	//virtual uv_err_t updateDataCore();	
	
public:
	//The actual data
	//Note that this contains the size and offset
	//UVDData *m_headerData;
	//Supporting data in the file, if present
	UVDData *m_fileData;
	//With fixups, if needed
	//Responsibility of this object to free this, must be pointer for polymorphic reasons
	UVDRelocatableData *m_fileRelocatableData;
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
class UVDElfRelocationSectionHeaderEntry;
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

	//The section that will calculate sh_link
	uv_err_t getLinkSection(UVDElfSectionHeaderEntry **relevantSectionHeader);
	void setLinkSection(UVDElfSectionHeaderEntry *relevantSectionHeader);

	uv_err_t getSupportingDataSize(uint32_t *sectionSize);

	//Make sure we have a relocatable section
	//It will be named .rel.<section name>
	virtual uv_err_t useRelocatableSection();
	//null if its not a relocatable section
	virtual uv_err_t getRelocatableSection(UVDElfRelocationSectionHeaderEntry **entry);
	
	uv_err_t updateForWrite();

public:
	//Name of this section
	std::string m_name;
	//Section header for this element
	Elf32_Shdr m_sectionHeader;
	//If there is a .rel.<section name> section, pointer to it
	UVDElfRelocationSectionHeaderEntry *m_relocationSectionHeader;
	//sh_link target, if applicable
	UVDElfSectionHeaderEntry *m_relevantSectionHeader;
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
	virtual uv_err_t init();
	
	void addString(const std::string &s);
	uv_err_t getStringOffset(const std::string &s, uint32_t *offsetOut);
	//How big is the string table?
	uv_err_t getSupportingDataSize(uint32_t *sectionSize);

	//Only rebuilt when needed
	//turned to updateData()
	//uv_err_t ensureCurrentStringTableData();
	//During updateForWrite, we simply wait for new string table elements
	//Here we build the table
	//During relocateForWrite(), we simply push out offsets as requested
	virtual uv_err_t constructForWrite();

public:
	std::vector<std::string> m_stringTable;
};

/*
Section that holds executable data
*/
class UVDElfTextSectionHeaderEntry : public UVDElfSectionHeaderEntry
{
public:
	UVDElfTextSectionHeaderEntry();
	~UVDElfTextSectionHeaderEntry();
	uv_err_t init();

	//This is a compilation of all of the symbols
	//virtual uv_err_t getFileData(UVDData **data);
	//virtual uv_err_t getSupportingDataSize(uint32_t *sectionSize);
	virtual uv_err_t constructForWrite();

public:
};

#endif
