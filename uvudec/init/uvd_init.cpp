/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

/*
FIXME:
This is probably the file most needing cleanup in the project
Do something instead like create a list of optional and required members, with pointers if necessary for easy setup
*/

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>
#include <sys/stat.h>
#include <vector>
#include <algorithm>
#include "uvd_debug.h"
#include "uvd_error.h"
#include "uvd_log.h"
#include "uvd_util.h"
#include "uvd.h"
#include "uvd_address.h"
#include "uvd_analysis.h"
#include "uvd_benchmark.h"
#include "uvd_config_symbol.h"
#include "uvd_data.h"
#include "uvd_format.h"
#include "uvd_instruction.h"
#include "uvd_register.h"
#include "uvd_types.h"
#include "uvd_cpu_vector.h"

/*
file: data file to target
architecture: hint about what we are trying to disassemble
*/
uv_err_t UVD::init(const std::string &file, int architecture)
{
	uv_err_t rcTemp = UV_ERR_GENERAL;
	UVDData *data;
	
	UV_ENTER();
		
	rcTemp = UVDDataFile::getUVDDataFile(&data, file);
	if( UV_FAILED(rcTemp) || !data )
	{
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	return init(data, architecture);
}

//int init_count = 0;
uv_err_t UVD::init(UVDData *data, int architecture)
{
	uv_err_t rc = UV_ERR_GENERAL;
	m_data = data;
	m_architecture = architecture;
	std::string configFile;
	
	UV_ENTER();
	
	printf_debug_level(UVD_DEBUG_PASSES, "UVD::init(): initializing engine...\n");
	UVDBenchmark engineInitBenchmark;
	engineInitBenchmark.start();

	/*
	++init_count;
	if( init_count > 1 )
	{
		*((unsigned int *)0) = 0;
	}
	*/
		
	switch( architecture )
	{
	default:
		configFile = "arch/8051/8051.op";
	};
	
	uv_assert_ret(m_config);
		
	m_config->m_verbose = m_config->m_verbose_init;

	/*
	m_CPU = new UVDCPU();
	uv_assert_ret(m_CPU);
	uv_assert_err_ret(m_CPU->init());
	*/

	m_opcodeTable = new UVDOpcodeLookupTable();
	//printf_debug("Initializing opcode table, address: 0x%.8X\n", (unsigned int)m_opcodeTable);
	uv_assert(m_opcodeTable);
	//m_CPU->m_opcodeTable = m_opcodeTable;
	
	m_symMap = new UVDSymbolMap();
	uv_assert(m_symMap);
	uv_assert_err_ret(m_symMap->init());
	//m_interpreter = new UVDConfigExpressionInterpreter();
	UVDConfigExpressionInterpreter::getConfigExpressionInterpreter(&m_interpreter);
	uv_assert(m_interpreter);
	m_analyzer = new UVDAnalyzer();
	uv_assert(m_analyzer);
	m_analyzer->m_uvd = this;
	uv_assert_err_ret(m_analyzer->init());
	//Default to our global config, which should have already been initialized since its program dependent
	m_config = g_config;
	uv_assert_ret(m_config);
	m_format = new UVDFormat();
	uv_assert(m_format);

	printf_debug("Initializing config...\n");
	if( UV_FAILED(init_config(configFile)) )
	{
		printf_error("failed 8051 init\n");
		return UV_ERR(UV_ERR_GENERAL);
	}
	/*
	Read file
	This is raw dat, NOT null terminated string
	*/

	printFormatting();
	printf_debug("UVD: init OK!\n\n\n");

	m_config->m_verbose = m_config->m_verbose_processing;
	
	engineInitBenchmark.stop();
	printf_debug_level(UVD_DEBUG_PASSES, "engine init time: %s\n", engineInitBenchmark.toString().c_str());

	rc = UV_ERR_OK;
error:
	return UV_DEBUG(rc);
}

//Initialize the opcode tables
uv_err_t UVD::init_config(const std::string &configFile)
{
	uv_err_t rc = UV_ERR_GENERAL;
	const std::string config_file = "arch/8051/8051.op";
	UVDConfigSection **sections = NULL;
	unsigned int n_sections = 0;
	unsigned int cur_section = 0;
		
	UVDConfigSection *op_section = NULL;
	UVDConfigSection *mem_section = NULL;
	UVDConfigSection *misc_section = NULL;
	UVDConfigSection *reg_section = NULL;
	UVDConfigSection *pre_section = NULL;
	UVDConfigSection *vec_section = NULL;

	UV_ENTER();

	printf_debug("Reading file...\n");
	if( UV_FAILED(UVDConfig::uvd_read_sections(config_file, &sections, &n_sections)) )
	{
		UV_ERR(rc);
		goto error;
	}
	
	for( cur_section = 0; cur_section < n_sections; ++cur_section )
	{
		if( sections[cur_section]->m_name == "OP" )
		{
			op_section = sections[cur_section];
		}
		else if( sections[cur_section]->m_name == "MEM" )
		{
			mem_section = sections[cur_section];
		}
		else if( sections[cur_section]->m_name == "MISC" )
		{
			misc_section = sections[cur_section];
		}
		else if( sections[cur_section]->m_name == "REG" )
		{
			reg_section = sections[cur_section];
		}
		else if( sections[cur_section]->m_name == "PRE" )
		{
			pre_section = sections[cur_section];
		}
		else if( sections[cur_section]->m_name == "VEC" )
		{
			vec_section = sections[cur_section];
		}
		else
		{
			printf_debug("Unrecognized section: <%s>\n", sections[cur_section]->m_name.c_str());
			UV_ERR(rc);
			goto error;
		}
	}
	
	if( UV_FAILED(init_misc(misc_section)) )
	{
		UV_ERR(rc);
		goto error;
	}
	
	/* Because of register memory mapping, memory should be initialized first */
	if( UV_FAILED(init_memory(mem_section)) )
	{
		UV_ERR(rc);
		goto error;
	}

	if( UV_FAILED(m_opcodeTable->init_opcode(op_section)) )
	{
		UV_ERR(rc);
		goto error;
	}

	if( UV_FAILED(init_reg(reg_section)) )
	{
		UV_ERR(rc);
		goto error;
	}

	if( UV_FAILED(init_prefix(pre_section)) )
	{
		UV_ERR(rc);
		goto error;
	}
	
	if( UV_FAILED(init_vectors(vec_section)) )
	{
		UV_ERR(rc);
		goto error;
	}

	rc = UV_ERR_OK;

error:
	for( unsigned int i = 0; i < n_sections; ++i )
	{
		delete sections[i];
	}
	free(sections);
	
	return UV_DEBUG(rc);
}

int g_format_debug = false;
 
uv_err_t UVD::init_misc(UVDConfigSection *misc_section)
{
	/* 
	The misc section is non-grouped directives.  
	As such, parsing it is much easier than most sections 
	*/
	uv_err_t rc = UV_ERR_GENERAL;
	unsigned int cur_line = 0;
	//char **lines = NULL;

	std::string value_name;
	std::string value_desc;
	std::string value_endian;
	std::string value_prefix;
	std::string value_prefix_hex;
	std::string value_postfix_hex;
	std::string value_suffix;

	UV_ENTER();
	printf_debug("Initializing misc data\n");
	if( misc_section == NULL )
	{
		printf_debug(".MISC section required\n");
		UV_ERR(rc);
		goto error;
	}

	uv_assert_ret(m_config);

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
		if( UV_FAILED(rc_temp) )
		{
			UV_ERR(rc);
			goto error;
		}
		else if( rc_temp == UV_ERR_BLANK )
		{
			continue;
		}

		if( !strcmp(key.c_str(), "MCU_NAME") )
		{
			value_name = value;
		}
		else if( !strcmp(key.c_str(), "MCU_DESC") )
		{
			value_desc = value;
		}
		else if( !strcmp(key.c_str(), "MCU_ENDIAN") )
		{
			value_endian = value;
		}
		else if( !strcmp(key.c_str(), "ASM_IMM_PREFIX") )
		{
			value_prefix = value;
		}
		else if( !strcmp(key.c_str(), "ASM_IMM_PREFIX_HEX") )
		{
			value_prefix_hex = value;
		}
		else if( !strcmp(key.c_str(), "ASM_IMM_POSTFIX_HEX") )
		{
			value_postfix_hex = value;
		}
		else if( !strcmp(key.c_str(), "ASM_IMM_SUFFIX") )
		{
			value_suffix = value;
		}
		else
		{
			printf_debug("Invalid key\n");
			UV_ERR(rc);
			goto error;
		}
	}
	
	if( value_name.empty() )
	{
		printf("MCU name not present\n");
		UV_ERR(rc);
		goto error;
	}
	m_config->m_mcu_name = value_name;
	
	//MCU desc is optional
	m_config->m_mcu_desc = value_desc;
	m_config->m_asm_imm_prefix = value_prefix;
	m_config->m_asm_imm_prefix_hex = value_prefix_hex;
	m_config->m_asm_imm_postfix_hex = value_postfix_hex;
	m_config->m_asm_imm_suffix = value_suffix;

	printf_debug("Misc init OK\n");

	rc = UV_ERR_OK;
error:
	return UV_DEBUG(rc);
}

uv_err_t UVD::init_memory(UVDConfigSection *mem_section)
{
	uv_err_t rc = UV_ERR_GENERAL;
	std::vector< std::vector<std::string> > memSectionParts;
	
	UV_ENTER();

	printf_debug("Initializing memory data\n");
	if( mem_section == NULL )
	{
		printf_debug(".MEM section required\n");
		UV_ERR(rc);
		goto error;
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
		//std::vector<UVDMemorySharedMapper *> memoryMappingEntries;
		
		UVDMemoryShared *memoryShared = NULL;
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
			if( UV_FAILED(rc_temp) )
			{
				UV_ERR(rc);
				goto error;
			}
			else if( rc_temp == UV_ERR_BLANK )
			{
				continue;
			}
	
			if( value_name.empty() && key != "NAME" )
			{
				printf_debug("NAME must go first\n");
				UV_ERR(rc);
				goto error;
			}
			
			if( !strcmp(key.c_str(), "NAME") )
			{
				value_name = value;
			}
			else if( !strcmp(key.c_str(), "TYPE") )
			{
				value_type = value;
			}
			else if( !strcmp(key.c_str(), "CAP") )
			{
				value_cap = value;
			}
			else if( !strcmp(key.c_str(), "MIN") )
			{
				value_min = value;
			}
			else if( !strcmp(key.c_str(), "MAX") )
			{
				value_max = value;
			}
			else if( !strcmp(key.c_str(), "ASM_PREFIX") )
			{
				value_prefix = value;
			}
			else if( !strcmp(key.c_str(), "ASM_SUFFIX") )
			{
				value_suffix = value;
			}
			else if( !strcmp(key.c_str(), "WORD_SIZE") )
			{
				value_word_size = value;
			}
			else if( !strcmp(key.c_str(), "WORD_ALIGNMENT") )
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
				printf("Invalid key: %s\n", key.c_str());
				UV_ERR(rc);
				goto error;
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
			UV_ERR(rc);
			goto error;
		}

		memoryShared = new UVDMemoryShared();
		uv_assert_all(memoryShared);

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
			UV_ERR(rc);
			goto error;
		}

		printf_debug("Parsing memory\n");		
		
		memoryShared->m_name = value_name;

		/*
		Memory types should probably be configured by flags and such 
		Allow direct capabilities config, default for certain memories
		*/
		if( !value_cap.empty() )
		{
			char **parts = NULL;
			unsigned int n_parts = 0;
			unsigned int i = 0;
			
			memoryShared->m_cap = 0;
			parts = uv_split_core(value_cap.c_str(), ',', &n_parts, 1);
			for( i = 0; i < n_parts; ++i )
			{
				std::string part = parts[i];
				
				if( part == "R" )
				{
					memoryShared->m_cap |= UV_DISASM_MEM_R;
				}
				else if( part == "W" )
				{
					memoryShared->m_cap |= UV_DISASM_MEM_W;
				}
				else if( part == "X" )
				{
					memoryShared->m_cap |= UV_DISASM_MEM_X;
				}
				else if( part == "NV" )
				{
					memoryShared->m_cap |= UV_DISASM_MEM_NV;
				}
				else
				{
					printf_debug("Unrecognized memory capability: %s\n", part.c_str());
					UV_ERR(rc);
					goto error;
				}
			}
		}
		else
		{
			/* 
			Need to decide how much I should differentiate memory types.
			Ex: could knowing its DRAM vs SRAM help at all during analysis?
			If programmer could eaisly program without knowing which, it might not be worth differentiating
			*/
			
			/* Temporary: read and write, unitialized at start */
			if( value_type == "RAM"
					|| value_type == "SRAM"
					|| value_type == "DRAM"
					|| value_type == "SDRAM" )
			{
				memoryShared->m_cap = UV_DISASM_MEM_R 
						| UV_DISASM_MEM_W;
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
				memoryShared->m_cap = UV_DISASM_MEM_R 
						| UV_DISASM_MEM_X
						| UV_DISASM_MEM_NV;
			}
			/* Nonvolatile: readable and writtable by MCU, preserved at startup */
			else if( value_type == "NV"
					|| value_type == "flash" )
			{
				memoryShared->m_cap = UV_DISASM_MEM_R 
						| UV_DISASM_MEM_W
						| UV_DISASM_MEM_NV;
			}
			/* Memory mapped special func registers */
			else if( value_type == "SFR" )
			{
				memoryShared->m_cap = UV_DISASM_MEM_R 
						| UV_DISASM_MEM_W;
			}
			/*
			else if( !strcmp(value_type, "IMM") )
			{
				
			}
			*/
			else
			{
				printf_debug("Unknown memory type: %s, cap flags must be specified manually\n", value_type.c_str());
				UV_ERR(rc);
				goto error;
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
			UV_ERR(rc);
			goto error;
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
		
		
		uv_assert_err_ret(splitConfigLinesVector(memoryMappingEntries, "MAPPING_DST=", memoryMappingEntriesParts));
		//Loop for each mapping
		for( std::vector< std::vector<std::string> >::size_type i = 0; i < memoryMappingEntriesParts.size(); ++i )
		{
			UVDMemorySharedMapper *memoryMapper = NULL;
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
					
					memoryMapper = new UVDMemorySharedMapper();
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
					printf("Invalid key: %s\n", key.c_str());
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

		/* Print formating */
		memoryShared->m_print_prefix = value_prefix;
		memoryShared->m_print_suffix = value_suffix;
		
		//Register so can be used for memory mapping
		{
			UVDSymbol *sym_temp = NULL;

			printf_debug("Adding sym\n");
			sym_temp = new UVDSymbol();
			uv_assert_all(sym_temp);
			sym_temp->m_type = UVD_SYMBOL_MEM;
			sym_temp->m_key = memoryShared->m_name;
			sym_temp->m_mem = memoryShared;
			
			printf_debug("Adding sym call\n");
			uv_assert_err(m_symMap->setSym(memoryShared->m_name, sym_temp, NULL));		
		}
	}
	
	m_symMap->print();

	printf_debug("Memory init OK\n");

	rc = UV_ERR_OK;

error:
	return UV_DEBUG(rc);
}

uv_err_t UVD::init_reg(UVDConfigSection *reg_section)
{
	uv_err_t rc = UV_ERR_GENERAL;
	std::vector< std::vector<std::string> > regSectionParts;
	
	UV_ENTER();

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
			UVDMemoryShared *memoryShared = NULL;
			//UVDMemoryLocation *mem_loc = NULL;			
			std::string spaceName;
			uint32_t spaceAddress = 0;
			
			/* RAM_INT(0xE0) */
			uv_assert_err(parseFunc(value_addr, func_name, func_content));

			spaceName = func_name;
			spaceAddress = strtol(func_content.c_str(), NULL, 0);
			
			//reg_shared->m_mem_addr = strtol(func_content.c_str(), NULL, 0);
			uv_assert_err(m_symMap->getSym(func_name, &memoryShared));

			/*
			mem_loc.space = memoryShared;
			mem_loc.min_addr = reg_shared->m_mem_addr;
			mem_loc.max_addr = mem_loc.min_addr + reg_shared->m_size - 1;
			*/
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

uv_err_t UVD::init_prefix(UVDConfigSection *pre_section)
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

uv_err_t UVD::init_vectors(UVDConfigSection *section)
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
