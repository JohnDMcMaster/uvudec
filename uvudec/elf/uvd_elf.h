/*
Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

/*
Original use was to store relocatable object files to aid in static analysis
FIXME: class is a mess, needs cleanup
*/

#include "uvd_data.h"
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
	static uv_err_t getUVDElfSectionHeaderEntry(const std::string &sSection, UVDElfSectionHeaderEntry **sectionHeaderOut);

	uv_err_t getName(std::string &sName);
	void setName(const std::string &sname);
	uv_err_t getName(int *nameIndex);
	void setName(int nameIndex);

	uv_err_t getType(int *type);
	void setType(int type);

	virtual uv_err_t getHeaderData(UVDData **data);
	virtual uv_err_t setHeaderData(const UVDData *data);

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

public:
	std::vector<std::string> m_stringTable;
};

/*
Constant data
Things like strings (const char *), program constants, lookup tables, etc
*/
class UVDElfRomData
{
public:
	//The actual data
	UVDData *m_data;
	//Relocatable element so we can figure out where it was placed
	UVDRelocatableElement *m_relocatable;	
};

class UVDElfSymbol;
class UVDElfRelocation
{
public:
	UVDElfRelocation();
	~UVDElfRelocation();
	
	/*
	Reloctions are done on an absolute basis of the data in the file
	If there was another chunk of data before this one, it must be accounted for
	Essentially this makes a relocation on a relocation
	For now, only a single symbol is stored in object files, making this factor 0 for now
	*/
	uv_err_t getFileOffset(uint32_t *elfSymbolFileOffset);
	uv_err_t updateRelocationTypeByBits(unsigned int nBits);
	//raw index into the symbol table
	//in practice might do this by a UVD relocation into m_relocation
	uv_err_t updateSymbolIndex(unsigned int symbolIndex);

	//Given a relocatable element, setup relocations on this object
	uv_err_t setupRelocations(UVDRelocationFixup *originalFixup);

	//What we preform the relocation on
	void setSymbol(UVDElfSymbol *symbol);
	
	//Raw access, r_info is non-trivial to access
	int getSymbolTableIndex();
	void setSymbolTableIndex(int index);
	int getRelocationType();
	void setRelocationType(int type);

public:
	//The symbol we will be relocating against
	UVDElfSymbol *m_symbol;
	//Switch to Elf32_Rela if needed
	Elf32_Rel m_relocation;
	//The symbol index for the relocation is dynamic and can only be figured out after symbols are placed
	UVDRelocationFixup m_symbolIndexRelocation;
};

/*
Some sort of globally visible symbol
All symbols occur in a single table and each symbol has a reference to the relavent section
There will be an assumption for now that since all relocations I want are in functions,
all relocations will be done in symbols' data
*/
class UVDElfSymbol
{
public:
	UVDElfSymbol();
	~UVDElfSymbol();

	//Set symbol name
	void setSymbolName(const std::string &sName);
	//Get symbol name
	std::string getSymbolName();

	uv_err_t getData(UVDData **data);
	void setData(UVDData *data);

public:
	//The symbol's (function's/variable's) name
	std::string m_sName;
	//The actual data this symbol represents, if its resolved
	//If the symbol is not resolved, data will be NULL
	UVDData *m_data;
	//The ELF symbol structure
	Elf32_Sym m_symbol;
	//Offsets are taken from here and stored in file
	//UVDRelocatableElement *m_relocatable;
};

/*
Section that holds actual symbol data
*/
class UVDElfSymbolSectionHeaderEntry : public UVDElfSectionHeaderEntry
{
public:
	UVDElfSymbolSectionHeaderEntry();
	~UVDElfSymbolSectionHeaderEntry();

	uv_err_t addSymbol(UVDElfSymbol *symbol);
	uv_err_t findSymbol(const std::string &sName, UVDElfSymbol **symbol);
	uv_err_t getSymbol(const std::string &sName, UVDElfSymbol **symbol);
	
public:
	std::vector<UVDElfSymbol *> m_symbols;
};

/*
A global variable
*/
class UVDElfVariable : public UVDElfSymbol
{
public:
	UVDElfVariable();
	~UVDElfVariable();

public:
	//Size in bits
	int m_size;
};

/*
A peice of binary data identified as a function
For convienence only 
*/
class UVDElfFunction : public UVDElfSymbol
{
public:
	UVDElfFunction();
	~UVDElfFunction();

public:
	//Binary function data
	UVDData *m_data;
	//Position in the file written out, not in memory
	//Note this is not used to relocate data during ELF file writting, but rather to generate relocation table
	UVDRelocatableData *m_fileRelocatableData;
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
	/*
	Factory function to create a blank conforming item
	Fills in basic header information and other peices
	*/
	static uv_err_t getUVDElf(UVDElf **out);
	//Setup with default stuffs
	uv_err_t init();
	uv_err_t initHeader();
	
	static uv_err_t getFromFile(const std::string &file, UVDElf **elf);
	uv_err_t loadFromFile(const std::string &file);
	//Get the raw binary image of this object
	uv_err_t constructBinary(UVDData **data);
	uv_err_t saveToFile(const std::string &file);
	//Construct from a UVD relocatable
	//Assumes in the .text section
	static uv_err_t getFromRelocatableData(UVDRelocatableData *relocatableData,
			const std::string &rawDataSymbolName, UVDElf **out);
	//Doest not make section assumptions
	static uv_err_t getFromRelocatableDataCore(UVDRelocatableData *relocatableData,
			const std::string &rawDataSymbolName,
			const std::string &sDataSection, const std::string &sRelocationSection,
			UVDElf **out);
	
	//Add another symbol along with supporting data
	uv_err_t addRelocatableData(UVDRelocatableData *relocatableData,
			const std::string &rawDataSymbolName);
	uv_err_t addRelocatableDataCore(UVDRelocatableData *relocatableData,
			const std::string &rawDataSymbolName,
			const std::string &sDataSection, const std::string &sRelocationSection);

	//Set the architecture
	//Should throw error on unknown architecture?
	uv_err_t getArchitecture(int *archOut);
	void setArchitecture(int archIn);

	int getProgramHeaderTableSize();
	int getSectionHeaderTableSize();

	uv_err_t addProgramHeaderSection(UVDElfProgramHeaderEntry *section);
	uv_err_t addSectionHeaderSection(UVDElfSectionHeaderEntry *section);
	
	//WARNING: this is currently find, not get (ie won't create on not found)
	uv_err_t getSectionHeaderByName(const std::string &sName, UVDElfSectionHeaderEntry **section);
	
	
	/*
	Generic string table functions
	Will hopefully return an error if you feed it a table that isn't a string table
	*/
	//Gaurantee string is in the table
	//If index is specified, it will be returned
	uv_err_t addStringCore(const std::string &sSection, const std::string &s, unsigned int *index = NULL);
	uv_err_t getStringIndexCore(const std::string &sSection, const std::string &s, unsigned int *index);
	//For a given string, get an object to find its final address
	uv_err_t getStringRelocatableElementCore(const std::string &sSection, const std::string &s, UVDRelocatableElement **relocatable, UVDRelocationManager *relocationManager);
	

	/*
	.shstrtab utilities
	Should be used for section header names
	*/
	uv_err_t addSectionHeaderString(const std::string &s, unsigned int *index = NULL);
	uv_err_t getSectionHeaderStringIndex(const std::string &s, unsigned int *index);
	uv_err_t getSectionHeaderStringRelocatableElement(const std::string &s, UVDRelocatableElement **relocatable, UVDRelocationManager *relocationManager);
	//Get the section header for .shstrtab
	uv_err_t getSectionHeaderStringTableEntry(UVDElfStringTableSectionHeaderEntry **sectionOut);


	/*
	.strtab
	Used for symbol names
	*/
	uv_err_t addSymbolString(const std::string &s, unsigned int *index = NULL);
	uv_err_t getSymbolStringIndex(const std::string &s, unsigned int *index);
	uv_err_t getSymbolStringRelocatableElement(const std::string &s, UVDRelocatableElement **relocatable, UVDRelocationManager *relocationManager);
	//Get the section header for .strtab
	uv_err_t getSymbolStringTableSectionHeaderEntry(UVDElfStringTableSectionHeaderEntry **sectionOut);

	/*
	.symtab
	Used for symbols
	*/
	//Get the table holding the actual symbols
	uv_err_t getSymbolTableSectionHeaderEntry(UVDElfSymbolSectionHeaderEntry **sectionOut);
	//Higher level functions
	//Some arbitrary symbol, it is not know what it represents
	uv_err_t addSymbol(UVDElfSymbol *symbol);
	uv_err_t findSymbol(const std::string &sName, UVDElfSymbol **symbol);
	uv_err_t getSymbol(const std::string &sName, UVDElfSymbol **symbol);
	
	/*
	.text
	Holds executable information
	Stores the analyzed functions
	*/
	
	//Raw data adds
	uv_err_t addSectionHeaderData(const std::string &sSection, UVDRelocatableData *relocatableData);
	uv_err_t addSectionHeaderData(int sectionIndex, UVDRelocatableData *relocatableData);
	
		
	//Adds constant data to .rodata
	//Strings and what not
	//Considering adding to .text instead to make processing easier...but this is more proper
	//uv_err_t addROMData(UVDRomData *romData)

	//Program data
	//Add arbitrary data to the text section
	//uv_err_t addTextData(UVDElfVariable *variable);
	uv_err_t addFunction(UVDElfFunction *function);
	//Register a variable to current ELF file
	uv_err_t addVariable(UVDElfVariable *variable);

	void printDebug();

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
	//Fixups we must apply to various data structures before writting out the final file
	std::vector<UVDSimpleRelocationFixup *> m_relocationFixups;
};

