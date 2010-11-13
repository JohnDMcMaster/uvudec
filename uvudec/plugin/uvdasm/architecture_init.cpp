/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/architecture/architecture.h"
#include "uvd/util/util.h"
#include "uvd/core/uvd.h"
#include "uvdasm/architecture.h"
#include "uvdasm/config.h"

#if 0
/* Internal RAM */
#define UV_DISASM_MEM_RAM_INT			1
/* External RAM */
#define UV_DISASM_MEM_RAM_EXT			2
/* Some sort of read only memory */
#define UV_DISASM_MEM_RAM_ROM			3
/* Flash storage */
#define UV_DISASM_MEM_RAM_FLASH			4

/* Capabilities */
/* Read */
#define UV_DISASM_MEM_R					0x01
/* Write */
#define UV_DISASM_MEM_W					0x02
/* Execute */
#define UV_DISASM_MEM_X					0x04
/* Nonvalitile */
#define UV_DISASM_MEM_NV				0x08
#endif

/*
Often times memory one memory space will intersect with another
Controls mapping of spaces
Mapping size must line up: if not, will assume error for now.
	Will change if a needed exception arises
	Ex: Bit in main address space corresponds to ZFLAG
	Could map to intermediate BRAM and then to the flag to avoid issue of byte to single bit

Ex:
	Mapping from 8-51 bit RAM to main address space
	Source
		BRAM (Bit RAM)
		8 bytes, 64 bits total
		word size: 1
		start address: 0
		end address: 63
	Dest
		IRAM (Internal RAM)
		8 bytes, 64 biits total
		word size: 8
		start adddress: 32
		end address: 39
Ex:
	Mapping 8051 direct addressing to main IRAM
	Source
		Direct addressing
		128 bytes
		word size: 8
		start address: 0
		end address: 0x7F
	Dest
		Indirect addressing space (or could use instead a main phsyiscal memory)
		256 bytes
		word size: 8
		start address: 0
		end address: 0xFF
*/

//Initialize the opcode tables
uv_err_t UVDDisasmArchitecture::init_config()
{
	std::vector<UVDConfigSection> sections;
		
	UVDConfigSection *op_section = NULL;
	UVDConfigSection *mem_section = NULL;
	UVDConfigSection *misc_section = NULL;
	UVDConfigSection *reg_section = NULL;
	UVDConfigSection *pre_section = NULL;
	UVDConfigSection *vec_section = NULL;

	printf_debug("Reading file...\n");
	if( UV_FAILED(UVDAsmUtil::readSections(m_architectureFileName, sections)) )
	{
		printf_error("Could not read config file: %s\n", g_asmConfig->m_architectureFileName.c_str());
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	
	for( std::vector<UVDConfigSection>::size_type curSectionIndex = 0; curSectionIndex < sections.size(); ++curSectionIndex )
	{
		UVDConfigSection &section = sections[curSectionIndex];
		if( section.m_name == "OP" )
		{
			op_section = &sections[curSectionIndex];
		}
		else if( section.m_name == "MEM" )
		{
			mem_section = &sections[curSectionIndex];
		}
		else if( section.m_name == "MISC" )
		{
			misc_section = &sections[curSectionIndex];
		}
		else if( section.m_name == "REG" )
		{
			reg_section = &sections[curSectionIndex];
		}
		else if( section.m_name == "PRE" )
		{
			pre_section = &sections[curSectionIndex];
		}
		else if( section.m_name == "VEC" )
		{
			vec_section = &sections[curSectionIndex];
		}
		else
		{
			printf_debug("Unrecognized section: <%s>\n", section.m_name.c_str());
			return UV_DEBUG(UV_ERR_GENERAL);
		}
	}
	
	uv_assert_err_ret(init_misc(misc_section));
	/* Because of register memory mapping, memory should be initialized first */
	uv_assert_err_ret(init_memory(mem_section));
	uv_assert_err_ret(m_opcodeTable->init_opcode(op_section));
	uv_assert_err_ret(init_reg(reg_section));
	uv_assert_err_ret(init_prefix(pre_section));
	uv_assert_err_ret(init_vectors(vec_section));

	return UV_ERR_OK;	
}

int g_format_debug = false;
 
uv_err_t UVDDisasmArchitecture::init_misc(UVDConfigSection *misc_section)
{
	/* 
	The misc section is non-grouped directives.  
	As such, parsing it is much easier than most sections 
	*/
	unsigned int cur_line = 0;
	//char **lines = NULL;

	std::string value_name;
	std::string value_desc;
	std::string value_endian;
	std::string value_prefix;
	std::string value_prefix_hex;
	std::string value_postfix_hex;
	std::string value_suffix;

	printf_debug("Initializing misc data\n");
	if( misc_section == NULL )
	{
		printf_debug(".MISC section required\n");
		return UV_DEBUG(UV_ERR_GENERAL);
	}

	uv_assert_ret(g_config);

	//lines = misc_section->m_lines;
	//n_lines = misc_section->m_n_lines;
		
	printf_debug("Processing misc lines...\n");
	
	for( cur_line = 0; cur_line < misc_section->m_lines.size(); ++cur_line )
	{
		std::string key;
		std::string value;
		std::string line = misc_section->m_lines[cur_line];
		uv_err_t rc_temp = UV_ERR_GENERAL;

		printf_debug("Line: <%s>\n", line.c_str());
		rc_temp = uvdParseLine(line, key, value);
		uv_assert_err_ret(rc_temp);
		if( rc_temp == UV_ERR_BLANK )
		{
			continue;
		}

		if( key == "MCU_NAME" )
		{
			value_name = value;
		}
		else if( key == "MCU_DESC" )
		{
			value_desc = value;
		}
		else if( key == "MCU_ENDIAN" )
		{
			value_endian = value;
		}
		else if( key == "ASM_IMM_PREFIX" )
		{
			value_prefix = value;
		}
		else if( key == "ASM_IMM_PREFIX_HEX" )
		{
			value_prefix_hex = value;
		}
		else if( key == "ASM_IMM_POSTFIX_HEX" )
		{
			value_postfix_hex = value;
		}
		else if( key == "ASM_IMM_SUFFIX" )
		{
			value_suffix = value;
		}
		else
		{
			printf_debug("Invalid key\n");
			return UV_DEBUG(UV_ERR_GENERAL);
		}
	}
	
	if( value_name.empty() )
	{
		printf_error("MCU name not present\n");
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	g_asmConfig->m_mcu_name = value_name;
	
	//MCU desc is optional
	g_asmConfig->m_mcu_desc = value_desc;
	g_asmConfig->m_asm_imm_prefix = value_prefix;
	g_asmConfig->m_asm_imm_prefix_hex = value_prefix_hex;
	g_asmConfig->m_asm_imm_postfix_hex = value_postfix_hex;
	g_asmConfig->m_asm_imm_suffix = value_suffix;

	printf_debug("Misc init OK\n");

	return UV_ERR_OK;
}

uv_err_t UVDDisasmArchitecture::init_memory(UVDConfigSection *mem_section)
{
	std::vector< std::vector<std::string> > memSectionParts;
	
	printf_debug("Initializing memory data\n");
	if( mem_section == NULL )
	{
		printf_debug(".MEM section required\n");
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	
	printf_debug("Processing mem lines...\n");
	uv_assert_err_ret(splitConfigLinesVector(mem_section->m_lines, "NAME=", memSectionParts));
	printf_debug("Memory sections: %d\n", memSectionParts.size());
	for( std::vector< std::vector<std::string> >::size_type memSectionPartsIndex = 0; memSectionPartsIndex < memSectionParts.size(); ++memSectionPartsIndex )
	{
		std::vector<std::string> cur_section = memSectionParts[memSectionPartsIndex];

		/* Its easier to parse some things before others */
		std::string value_name;
		std::string value_type;
		std::string value_min;
		std::string value_max;
		std::string value_prefix;
		std::string value_suffix;
		std::string value_cap;
		std::string value_word_size;
		std::string value_word_alignment;
		std::vector<std::string> memoryMappingEntries;
		//std::vector<UVDAddressSpaceMapper *> memoryMappingEntries;
		
		UVDAddressSpace *memoryShared = NULL;
		//Mapping addresses
		std::vector< std::vector<std::string> > memoryMappingEntriesParts;

		//printf_debug("\nLooking for next memory block at index %d\n", cur_line);
		/* Start by extracting key/value pairs */
		for( unsigned int cur_line = 0; cur_line < cur_section.size(); ++cur_line )
		{
			uv_err_t rc_temp = UV_ERR_GENERAL;
			std::string key;
			std::string value;
			std::string line = cur_section[cur_line];

			printf_debug("Line: <%s>\n", line.c_str());
			rc_temp = uvdParseLine(line, key, value);
			uv_assert_err_ret(rc_temp);
			if( rc_temp == UV_ERR_BLANK )
			{
				continue;
			}
	
			if( value_name.empty() && key != "NAME" )
			{
				printf_debug("NAME must go first\n");
				return UV_DEBUG(UV_ERR_GENERAL);
			}
			
			if( key == "NAME" )
			{
				value_name = value;
			}
			else if( key == "TYPE" )
			{
				value_type = value;
			}
			else if( key == "CAP" )
			{
				value_cap = value;
			}
			else if( key == "MIN" )
			{
				value_min = value;
			}
			else if( key == "MAX" )
			{
				value_max = value;
			}
			else if( key == "ASM_PREFIX" )
			{
				value_prefix = value;
			}
			else if( key == "ASM_SUFFIX" )
			{
				value_suffix = value;
			}
			else if( key == "WORD_SIZE" )
			{
				value_word_size = value;
			}
			else if( key == "WORD_ALIGNMENT" )
			{
				value_word_alignment = value;
			}
			//Mapping related
			else if( key.find("MAPPING_") == 0 )
			{
				memoryMappingEntries.push_back(line);
			}
			else
			{
				printf_error("Invalid memory section key: %s\n", key.c_str());
				return UV_DEBUG(UV_ERR_GENERAL);
			}
		}

		/*
		NAME=RAM_EXT
		TYPE=RAM
		# Largest address possible
		MAX=0xFFFF
		ASM_PREFIX=@
		*/

		if( value_name.empty() )
		{
			printf_debug("Name missing\n");
			return UV_DEBUG(UV_ERR_GENERAL);
		}

		memoryShared = new UVDAddressSpace();
		uv_assert_ret(memoryShared);

		printf_debug("Allocated new memory, new: 0x%.8X\n", (unsigned int)memoryShared);	

		printf_debug("std::string value_name = %s\n", value_name.c_str());
		printf_debug("std::string value_type = %s\n", value_type.c_str());
		printf_debug("std::string value_cap = %s\n", value_cap.c_str());
		printf_debug("std::string value_min = %s\n", value_min.c_str());
		printf_debug("std::string value_max = %s\n", value_max.c_str());
		printf_debug("std::string value_prefix = %s\n", value_prefix.c_str());
		printf_debug("std::string value_suffix = %s\n", value_suffix.c_str());
		printf_debug("std::string value_word_size = %s\n", value_word_size.c_str());
		printf_debug("std::string value_word_alignment = %s\n", value_word_alignment.c_str());

		if( value_type.empty() )
		{
			printf_debug("Type missing\n");
			return UV_DEBUG(UV_ERR_GENERAL);
		}

		printf_debug("Parsing memory\n");		
		
		memoryShared->m_name = value_name;

		/*
		Memory types should probably be configured by flags and such 
		Allow direct capabilities config, default for certain memories
		*/
		if( !value_cap.empty() )
		{
			std::vector<std::string> parts;
			
			//memoryShared->m_cap = 0;
			parts = UVDSplit(value_cap, ',', 1);
			for( std::vector<std::string>::size_type i = 0; i < parts.size(); ++i )
			{
				const std::string &part = parts[i];
				
				if( part == "R" )
				{
					memoryShared->m_R |= UVD_TRI_TRUE;
				}
				else if( part == "W" )
				{
					memoryShared->m_W |= UVD_TRI_TRUE;
				}
				else if( part == "X" )
				{
					memoryShared->m_X |= UVD_TRI_TRUE;
				}
				//Eh this is vague, we should get rid of it
				else if( part == "NV" )
				{
					//memoryShared->m_cap |= UV_DISASM_MEM_NV;
					memoryShared->m_R |= UVD_TRI_TRUE;
					memoryShared->m_X |= UVD_TRI_TRUE;
				}
				else
				{
					printf_debug("Unrecognized memory capability: %s\n", part.c_str());
					return UV_DEBUG(UV_ERR_GENERAL);
				}
			}
		}
		else
		{
			/* 
			Need to decide how much I should differentiate memory types.
			Ex: could knowing its DRAM vs SRAM help at all during analysis?
			If programmer could eaisly program without knowing which, it might not be worth differentiating
			
			Execute guesses are sloppy since memory technology has nothing to do with capabilities
			Don't rely on them
			*/
			
			/* Temporary: read and write, unitialized at start */
			if( value_type == "RAM"
					|| value_type == "SRAM"
					|| value_type == "DRAM"
					|| value_type == "SDRAM" )
			{
				memoryShared->m_R = UVD_TRI_TRUE;
				memoryShared->m_W = UVD_TRI_TRUE;
				memoryShared->m_X = UVD_TRI_UNKNOWN;
				memoryShared->m_NV = UVD_TRI_FALSE;
			}
			/* Read only, initialized at start */
			/* ROM is often used for execution/is allowed, should this be defined here by default? */
			else if( value_type == "ROM"
					|| value_type == "EPROM"
					/* 
					Should this have the write flag?  
					Sometimes just there for storage, like on PICs
						 What type of memory do PICs store their code in?
					*/
					|| value_type == "EEPROM" )
			{
				memoryShared->m_R |= UVD_TRI_TRUE;
				memoryShared->m_W |= UVD_TRI_FALSE;
				memoryShared->m_X |= UVD_TRI_UNKNOWN;
				memoryShared->m_NV = UVD_TRI_TRUE;
			}
			/* Nonvolatile: readable and writtable by MCU, preserved at startup */
			else if( value_type == "NV"
					|| value_type == "flash" )
			{
				memoryShared->m_R |= UVD_TRI_TRUE;
				memoryShared->m_W |= UVD_TRI_TRUE;
				memoryShared->m_X |= UVD_TRI_UNKNOWN;
				memoryShared->m_NV = UVD_TRI_TRUE;
			}
			/* Memory mapped special func registers */
			else if( value_type == "SFR" )
			{
				memoryShared->m_R |= UVD_TRI_TRUE;
				memoryShared->m_W |= UVD_TRI_TRUE;
				memoryShared->m_X |= UVD_TRI_FALSE;
				memoryShared->m_NV = UVD_TRI_FALSE;
			}
			else
			{
				printf_debug("Unknown memory type: %s, cap flags must be specified manually\n", value_type.c_str());
				return UV_DEBUG(UV_ERR_GENERAL);
			}
		}

		printf_debug("cap done\n");

		/* Addressing ranges */
		if( !value_min.empty() )
		{
			memoryShared->m_min_addr = strtol(value_min.c_str(), NULL, 0);
		}
		else
		{
			memoryShared->m_min_addr = 0;
		}
		if( !value_max.empty() )
		{
			memoryShared->m_max_addr = strtol(value_max.c_str(), NULL, 0);
		}
		else
		{
			memoryShared->m_max_addr = 0;
		}
		if( memoryShared->m_min_addr > memoryShared->m_max_addr )
		{
			printf_debug("Minimum is greater than maximum address\n");
			return UV_DEBUG(UV_ERR_GENERAL);
		}
		
		if( !value_word_size.empty() )
		{
			memoryShared->m_word_size = strtol(value_word_size.c_str(), NULL, 0);
		}
		else
		{
			/* Default to one byte */
			memoryShared->m_word_size = 8;
		}
		if( !value_word_alignment.empty() )
		{
			memoryShared->m_word_alignment = strtol(value_word_alignment.c_str(), NULL, 0);
		}
		else
		{
			/* Default to word size */
			memoryShared->m_word_alignment = memoryShared->m_word_size;
		}
		
#if 0
		//FIXME: this code has been deactivated since it wasn't being used and memory architecture rewritten
		uv_assert_err_ret(splitConfigLinesVector(memoryMappingEntries, "MAPPING_DST=", memoryMappingEntriesParts));
		//Loop for each mapping
		for( std::vector< std::vector<std::string> >::size_type i = 0; i < memoryMappingEntriesParts.size(); ++i )
		{
			UVDAddressSpaceMapper *memoryMapper = NULL;
			std::vector<std::string> lastMappingEntry = memoryMappingEntriesParts[i];
			
			uv_assert_ret(lastMappingEntry.size() >= 1);
			
			std::string value_src_min;
			std::string value_src_max;
			std::string value_dst;
			std::string value_dst_min;
			std::string value_dst_max;

			//Loop for each individual mapping item
			for( std::vector<std::string>::size_type j = 0; j < lastMappingEntry.size(); ++j )
			{
				std::string line = lastMappingEntry[j];
				std::string key;
				std::string value;
				
				//printf_debug("Line: <%s>\n", line.c_str());
				if( UV_FAILED(uvdParseLine(line, key, value)) )
				{
					UV_ERR(rc);
					goto error;
				}

				if( j == 0 )
				{
					//Must go first
					if( key != "MAPPING_DST" )
					{
						UV_ERR(rc);
						goto error;
					}
					
					value_dst = value;
					
					memoryMapper = new UVDAddressSpaceMapper();
					uv_assert(memoryMapper);
					memoryShared->m_mappers.push_back(memoryMapper);
				}
				else if( key == "MAPPING_SRC_MIN" )
				{
					value_src_min = value;
				}
				else if( key == "MAPPING_SRC_MAX" )
				{
					value_src_max = value;					
				}
				else if( key == "MAPPING_DST_MIN" )
				{
					value_dst_min = value;
				}
				else if( key == "MAPPING_DST_MAX" )
				{
					value_dst_max = value;
				}
				else
				{
					printf_error("Invalid key: %s\n", key.c_str());
					UV_ERR(rc);
					goto error;
				}
			}

			uv_assert(memoryMapper);

			//Link in source and dest address spaces
			UVDSymbol *sym_temp = NULL;
			memoryMapper->m_src_shared = memoryShared;
			uv_assert_err(m_symMap->getSym(value_dst, &sym_temp));		
			uv_assert(sym_temp);
			memoryMapper->m_dst_shared = sym_temp->m_mem;

			if( value_dst_min.empty() )
			{
				value_dst_min = "0";
			}
			memoryMapper->m_dst_min_addr = strtol(value_dst_min.c_str(), NULL, 0);

			if( value_dst_max.empty() )
			{
				value_dst_max = "0";
			}					
			memoryMapper->m_dst_max_addr = strtol(value_dst_max.c_str(), NULL, 0);

			if( value_src_min.empty() )
			{
				value_src_min = "0";
			}
			memoryMapper->m_src_min_addr = strtol(value_src_min.c_str(), NULL, 0);

			if( value_src_max.empty() )
			{
				value_src_max = "0";
			}
			memoryMapper->m_src_max_addr = strtol(value_src_max.c_str(), NULL, 0);
		
			//Perform fixup and fix bad ranges
			uv_assert_err(memoryMapper->finalizeConfig());
		}
#endif

		/* Print formating */
		memoryShared->m_print_prefix = value_prefix;
		memoryShared->m_print_suffix = value_suffix;
		
		//Register so can be used for memory mapping
		{
			UVDSymbol *sym_temp = NULL;

			printf_debug("Adding sym\n");
			sym_temp = new UVDSymbol();
			uv_assert_ret(sym_temp);
			sym_temp->m_type = UVD_SYMBOL_MEM;
			sym_temp->m_key = memoryShared->m_name;
			sym_temp->m_mem = memoryShared;
			
			printf_debug("Adding sym call\n");
			uv_assert_err_ret(m_symMap->setSym(memoryShared->m_name, sym_temp, NULL));		
		}
	}
	
	m_symMap->print();

	printf_debug("Memory init OK\n");

	return UV_ERR_OK;
}

uv_err_t UVDDisasmArchitecture::init_reg(UVDConfigSection *reg_section)
{
	uv_err_t rc = UV_ERR_GENERAL;
	std::vector< std::vector<std::string> > regSectionParts;
	
	printf_debug("Initializing register data\n");
	if( reg_section == NULL )
	{
		printf_debug(".RET section required\n");
		UV_ERR(rc);
		goto error;
	}
	
	//uv_assert_err(uvd_init_equiv_mem());

	printf_debug("Processing reg lines...\n");
	uv_assert_err_ret(splitConfigLinesVector(reg_section->m_lines, "NAME=", regSectionParts));
	printf_debug("Reg sections: %d\n", regSectionParts.size());
	for( std::vector< std::vector<std::string> >::size_type regSectionPartsIndex = 0; regSectionPartsIndex < regSectionParts.size(); ++regSectionPartsIndex )
	{
		std::vector<std::string> cur_section = regSectionParts[regSectionPartsIndex];	
	
		/* Its easier to parse some things before others */
		std::string value_name;
		std::string value_desc;
		std::string value_addr;
		std::string value_size;
		
		UVDRegisterShared *reg_shared = NULL;
		
		//printf_debug("\nLooking for next reg block at index %d\n", cur_line);
		/* Start by extracting key/value pairs */
		for( std::vector<std::string>::size_type cur_line = 0; cur_line < cur_section.size(); ++cur_line )
		{
			uv_err_t rc_temp = UV_ERR_GENERAL;
			std::string key;
			std::string value;
			std::string line = cur_section[cur_line];

			printf_debug("Line: <%s>\n", line.c_str());
			rc_temp = uvdParseLine(line, key, value);
			if( UV_FAILED(rc_temp) )
			{
				UV_ERR(rc);
				goto error;
			}
			else if( rc_temp == UV_ERR_BLANK )
			{
				continue;
			}
	
			if( cur_line == 0 && key != "NAME" )
			{
				printf_debug("NAME must go first\n");
				UV_ERR(rc);
				goto error;
			}
			
			if( key == "NAME" )
			{
				value_name = value;
			}
			else if( key == "DESC" )
			{
				value_desc = value;
			}
			else if( key == "ADDR" )
			{
				value_addr = value;
			}
			else if( key == "SIZE" )
			{
				value_size = value;
			}
			else
			{
				printf_debug("Invalid key\n");
				UV_ERR(rc);
				goto error;
			}
		}

		/*
		NAME=A
		DESC=Accumulator
		ADDR=RAM_INT(0xE0)
		SIZE=0x08
		*/

		reg_shared = new UVDRegisterShared();
		uv_assert_all(reg_shared);

		printf_debug("Allocated new memory, new: 0x%X\n", (unsigned int)reg_shared);	
		
		printf_debug("std::string value_name = %s\n", value_name.c_str());
		printf_debug("std::string value_desc = %s\n", value_desc.c_str());
		printf_debug("std::string value_addr = %s\n", value_addr.c_str());
		printf_debug("std::string value_size = %s\n", value_size.c_str());

		printf_debug("Parsing register\n");
		
		reg_shared->m_name = value_name;
		reg_shared->m_desc = value_desc;
	
		if( value_size.empty() )
		{
			reg_shared->m_size = 8;
		}
		else
		{
			reg_shared->m_size = strtol(value_size.c_str(), NULL, 0);
		}

		if( !value_addr.empty() )
		{
			std::string func_name;
			std::string func_content;
			UVDAddressSpace *memoryShared = NULL;
			std::string spaceName;
			uint32_t spaceAddress = 0;
			
			/* RAM_INT(0xE0) */
			uv_assert_err(parseFunc(value_addr, func_name, func_content));

			spaceName = func_name;
			spaceAddress = strtol(func_content.c_str(), NULL, 0);
			
			//reg_shared->m_mem_addr = strtol(func_content.c_str(), NULL, 0);
			uv_assert_err(m_symMap->getSym(func_name, &memoryShared));

			/* Register the register as an equivilent memory location */
			uv_assert_err(memoryShared->setEquivMemName(spaceAddress, value_name));
		}
		else
		{
			/* Not memory mapped */
			//reg_shared->m_mem = NULL;
		}
		
		//Save for future use
		m_registers[value_name] = reg_shared;
	}
	
	printf_debug("Register init OK\n");

	rc = UV_ERR_OK;

error:
	return UV_DEBUG(rc);
}

uv_err_t UVDDisasmArchitecture::init_prefix(UVDConfigSection *pre_section)
{
	uv_err_t rc = UV_ERR_GENERAL;
	
	printf_debug("Initializing prefix data\n");
	if( pre_section == NULL )
	{
		printf_debug("Optional .PRE section not found\n");
		rc = UV_ERR_OK;
		goto error;
	}

	rc = UV_ERR_OK;
error:
	return UV_DEBUG(rc);
}

uv_err_t UVDDisasmArchitecture::init_vectors(UVDConfigSection *section)
{
	uv_err_t rc = UV_ERR_GENERAL;

#if USING_VECTORS
	std::vector< std::vector<std::string> > sectionParts;
	
	printf_debug("Initializing vectors data\n");
	if( section == NULL )
	{
		printf_debug("Optional .VEC section not found\n");
		rc = UV_ERR_OK;
		goto error;
	}

	printf_debug("Processing lines...\n");
	uv_assert_err_ret(splitConfigLinesVector(section->m_lines, "NAME=", sectionParts));
	printf_debug("Sections: %d\n", sectionParts.size());
	for( std::vector< std::vector<std::string> >::size_type sectionPartsIndex = 0; sectionPartsIndex < sectionParts.size(); ++sectionPartsIndex )
	{
		UVDCPUVector *vector = NULL;
		std::vector<std::string> cur_section = sectionParts[sectionPartsIndex];	
	
		/* Its easier to parse some things before others */
		std::string value_name;
		std::string value_description;
		std::string value_offset;
		
		/* Start by extracting key/value pairs */
		for( std::vector<std::string>::size_type cur_line = 0; cur_line < cur_section.size(); ++cur_line )
		{
			uv_err_t rc_temp = UV_ERR_GENERAL;
			std::string key;
			std::string value;
			std::string line = cur_section[cur_line];

			printf_debug("Line: <%s>\n", line.c_str());
			rc_temp = uvdParseLine(line, key, value);
			uv_assert_err(rc_temp);
			if( rc_temp == UV_ERR_BLANK )
			{
				continue;
			}
	
			if( cur_line == 0 && key != "NAME" )
			{
				printf_debug("NAME must go first\n");
				UV_ERR(rc);
				goto error;
			}
			
			if( key == "NAME" )
			{
				value_name = value;
			}
			else if( key == "DESC" )
			{
				value_description = value;
			}
			else if( key == "OFFSET" )
			{
				value_offset = value;
			}
			else
			{
				printf_debug("Invalid key\n");
				UV_ERR(rc);
				goto error;
			}
		}

		/*
		NAME=START
		DESCRIPTION=Power on execution address
		OFFSET=0x0000
		*/

		vector = new UVDCPUVector();
		uv_assert_all(vector);

		printf_debug("Allocated new memory, new: 0x%X\n", (unsigned int)vector);	
		
		printf_debug("std::string value_name = %s\n", value_offset.c_str());
		printf_debug("std::string value_description = %s\n", value_description.c_str());
		printf_debug("std::string value_offset = %s\n", value_offset.c_str());

		printf_debug("Parsing register\n");
		
		vector->m_name = value_name;
		vector->m_description = value_description;
		vector->m_offset = strtol(value_offset.c_str(), NULL, 0);
		
		m_CPU->m_vectors.push_back(vector);
	}
#endif	
	
	printf_debug("Vectors init OK\n");

	rc = UV_ERR_OK;
//error:
	return UV_DEBUG(rc);
}

