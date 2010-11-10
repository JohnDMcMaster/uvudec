/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/util/util.h"
#include "uvd/assembly/address.h"
#include "uvd/config/config.h"
#include "uvdasm/config_symbol.h"
#include "uvd/language/format.h"
#include "uvd/assembly/instruction.h"
#include "uvd/util/types.h"
#include "uvdasm/instruction.h"
#include "uvdasm/opcode_table.h"
#include <string>
#include <vector>
#include <string.h>
#include <stdio.h>

UVDDisasmOpcodeLookupTable::UVDDisasmOpcodeLookupTable()
{
	memset(m_lookupTable, 0, sizeof(m_lookupTable));
	memset(m_lookupTable, 0, sizeof(m_lookupTableHits));
}

UVDDisasmOpcodeLookupTable::~UVDDisasmOpcodeLookupTable()
{
	deinit();
}

/*
Register a lookup for opcode to return newElement

If old opcode is present, it will be assinged to oldElement if non-null
Otherwise, old opcode will be deleted

A value of NULL indicates an non-coding opcode.  
It will be an error to lookup an opcode with such a value
*/
uv_err_t UVDDisasmOpcodeLookupTable::registerOpcode(unsigned char opcode, UVDDisasmInstructionShared newElement, UVDDisasmInstructionShared **oldElement)
{
	return UV_DEBUG(UV_ERR_GENERAL);
}

/*
TODO: branch to sub tables
*/
void UVDDisasmOpcodeLookupTable::usageStats(void)
{
	int i = 0;
	int used = 0;
	int unused = 0;
	
	for( i = 0; i < 256; ++i )
	{
		if( m_lookupTable[i] )
		{
			++used;
		}
		else
		{
			printf_warn("opcode 0x%.2X is non-encoding\n", i);
			++unused;
		}
	}
	printf_debug("Used opcodes: %d, unused opcodes: %d\n", used, unused);
}

void UVDDisasmOpcodeLookupTable::usedStats(void)
{
	int i = 0;
	int used = 0;
	int unused = 0;
	
	for( i = 0; i < 256; ++i )
	{
		std::string sOp = "non-encoding";
		if( m_lookupTable[i] )
		{
			sOp = m_lookupTable[i]->getHumanReadableUsage();
		}
		
		printf_debug("hits[0x%.2X] (%s): %d\n", i, sOp.c_str(), m_lookupTableHits[i]);
		if( m_lookupTableHits[i] )
		{
			++used;
		}
		else
		{
			++unused;
		}
	}
	printf_debug("Used opcodes: %d, unused opcodes: %d\n", used, unused);
}

/*
Opcodes are not allowed here as opposed to uvd_parse_syntax
This function also recursivly calls itself to handle function arguments
*/
uv_err_t UVDDisasmOpcodeLookupTable::uvd_parse_syntax_operand(UVDDisasmOperandShared **op_shared_in, const std::string cur)
{
	uv_err_t rc = UV_ERR_GENERAL;
	UVDConfigValue parsed_type;


	if( !op_shared_in )
	{
		UV_ERR(rc);
		goto error;
	}
	
	/* The bulk of the work is done here as data is identified */
	printf_debug("Parse syntax operand, parsing type: %s\n", cur.c_str());
	if( UV_FAILED(UVDConfigValue::parseType(cur, &parsed_type)) )
	{
		UV_ERR(rc);
		goto error;
	}
	
	printf_debug("Parse syntax operand, converting to operand struct: %s\n", cur.c_str());
	if( UV_FAILED(UVDDisasmOperandShared::uvd_parsed2opshared(&parsed_type, op_shared_in)) )
	{
		UV_ERR(rc);
		goto error;
	}
	rc = UV_ERR_OK;
	
error:
	return UV_DEBUG(rc);
}

/* 
Syntax
How the instruction looks during assembly
-Should not 
	-imply anything about how much actual space they use
	-Have opcodes
-Should
	-Have immediates
		-Must have name matching USAGE, as such will have size implied
		-Does not imply size because it is possible for examle to have ADD 0x10 be a single opcode
	-Have registers
	-Have memory usage
	-Not have to be in the same order as the USAGE
	-Setup operand structures as all of them appear during syntax
SYNTAX=RAM_INT(u8_0),u8_1
*/
uv_err_t UVDDisasmOpcodeLookupTable::uvd_parse_syntax(UVDDisasmInstructionShared *inst_shared, const std::string value_syntax)
{
	uv_err_t rc = UV_ERR_GENERAL;
	std::vector<std::string> syntaxParts;
	
	
	if( !inst_shared )
	{
		UV_ERR(rc);
		goto error;
	}
	
	/* Occasionally we may have no syntax */
	if( value_syntax.empty() )
	{
		inst_shared->m_operands.clear();
		rc = UV_ERR_OK;
		goto error;	
	}

	uv_assert_err_ret(getArguments(value_syntax, syntaxParts));
						
	/* Collect immediates and operands */
	for( std::vector<std::string>::size_type syntaxPartsIndex = 0; syntaxPartsIndex < syntaxParts.size(); ++syntaxPartsIndex )
	{
		UVDConfigValue parsed_type;
		std::string cur;

		cur = syntaxParts[syntaxPartsIndex];
		printf_debug("Current syntax part: %s\n", cur.c_str());
		if( UV_FAILED(UVDConfigValue::parseType(cur, &parsed_type)) )
		{
			UV_ERR(rc);
			goto error;
		} 
		//uv_assert(parsed_type);
		printf_debug("Current syntax part, post parse: %s\n", cur.c_str());

		UVDDisasmOperandShared *op_shared = NULL;

		if( UV_FAILED(uvd_parse_syntax_operand(&op_shared, cur)) )
		{
			UV_ERR(rc);
			goto error;
		}

		//Why would it be okay for this to be null?
		//Are there some non-encoding syntax parts?
		//Why not error instead?
		uv_assert(op_shared);
		inst_shared->m_operands.push_back(op_shared);
	}
	rc = UV_ERR_OK;
	
error:
	return UV_DEBUG(rc);
}

uv_err_t UVDDisasmOpcodeLookupTable::uvd_match_syntax_usage_core(std::vector<UVDDisasmOperandShared *> op_shareds,
		UVDConfigValue *parsed_type, 
		UVDDisasmOperandShared **op_shared_out)
{
	uv_err_t rc = UV_ERR_GENERAL;
	UVDDisasmOperandShared *op_shared = NULL;


	printf_debug("match syntax core\n");
	if( !op_shared_out )
	{
		UV_ERR(rc);
		goto error;
	}
	for( unsigned int i = 0; i < op_shareds.size(); ++i )
	{
		op_shared = op_shareds[i];
		printf_debug("Looking for operand match: <%s> ?= <%s>\n", op_shared->m_name.c_str(), parsed_type->m_name.c_str());
		if( op_shared->m_type == UV_DISASM_DATA_FUNC )
		{
			unsigned int i = 0;
			printf_debug("Function detected, recursing.  n args: %d\n", op_shared->m_func->m_args.size());
			for( i = 0; i < op_shared->m_func->m_args.size(); ++i )
			{
				if( UV_SUCCEEDED(uvd_match_syntax_usage_core(op_shared->m_func->m_args, parsed_type, op_shared_out)) )
				{
					printf_debug("Matched func arg\n");
					rc = UV_ERR_OK;
					goto error;
				}
				else
				{
					printf_debug("no match arg\n");
				}
			}
		}
		else
		{
			printf_debug("Checking..\n");
			if( !strcmp(op_shared->m_name.c_str(), parsed_type->m_name.c_str()) )
			{
				printf_debug("Match\n");
				break;
			}
			else
			{
				printf_debug("no match\n");
			}
		}
		op_shared = NULL;
	}

	if( op_shared == NULL )
	{
		//UV_ERR(rc);
		goto error;
	}
	*op_shared_out = op_shared;
	rc = UV_ERR_OK;
					
error:
	//It is not unusual to fail to match args when looking for correct one
	//return UV_DEBUG(rc);
	return rc;
}

uv_err_t UVDDisasmOpcodeLookupTable::uvd_match_syntax_usage(UVDDisasmInstructionShared *inst_shared,
		UVDConfigValue *parsed_type, 
		UVDDisasmOperandShared **op_shared_out)
{
	uv_err_t rc = UV_ERR_GENERAL;


	if( !inst_shared )
	{
		UV_ERR(rc);
		goto error;
	}
	
	if( UV_FAILED(uvd_match_syntax_usage_core(inst_shared->m_operands, parsed_type, op_shared_out)) )
	{
		UV_ERR(rc);
		goto error;
	} 
	rc = UV_ERR_OK;
	
error:
	return UV_DEBUG(rc);
}


/* 
USAGE field
How the instruction looks in binary
The SYNTAX field must have already been parsed before this to setup the operand structs
The main job of this field is to
-Map immediates to arguments as seen in an assembler
-Figure out how long the entire instruction is so the next instruction class can be deciphered
*/
uv_err_t UVDDisasmOpcodeLookupTable::uvd_parse_usage(UVDDisasmInstructionShared *inst_shared, const std::string value_usage)
{
	//Ignore for now...
	//FIXME: we need to know how many bytes this took at a minimum
	/* USAGE=0x53 i0 i1 */
	std::vector<std::string> usageParts;
	
	usageParts =  UVDSplit(value_usage, ',', TRUE);
	
	/* Collect immediates and operands */
	for( std::vector<std::string>::size_type curUsagePart = 0; curUsagePart < usageParts.size(); ++curUsagePart )
	{
		const std::string &cur = usageParts[curUsagePart];
		/* Iterate over the fields already setup by usage */
		UVDDisasmOperandShared *op_shared = NULL;
		UVDConfigValue parsed_type;

		printf_debug("Current operand: %s\n", cur.c_str());
		uv_assert_err_ret(UVDConfigValue::parseType(cur, &parsed_type));		
		printf_debug("Type: %d\n", parsed_type.m_operand_type);
		
		if( parsed_type.m_operand_type == UV_DISASM_DATA_OPCODE )
		{
			if( inst_shared->m_opcode_length >= MAX_OPCODE_SIZE )
			{
				printf_debug("Maximum opcode length exceeded\n");
				return UV_DEBUG(UV_ERR_GENERAL);
			}
			inst_shared->m_opcode[inst_shared->m_opcode_length] = parsed_type.m_value;
			inst_shared->m_opcodeRangeOffset[inst_shared->m_opcode_length] = parsed_type.m_opcodeRangeOffset;
			inst_shared->m_bitmask[inst_shared->m_opcode_length] = parsed_type.m_bitmask;
			++inst_shared->m_opcode_length;
			inst_shared->m_total_length += 1;
			continue;
		}
		
		/*
		Find the syntax field that matches parsed_type from the list of
		syntax fields in inst_shared and place it in op_shared
		*/
		uv_assert_err_ret(uvd_match_syntax_usage(inst_shared, &parsed_type, &op_shared));
		
		if( !op_shared )
		{
			printf_debug("Could not match up SHARED and USAGE fields\n");
			return UV_DEBUG(UV_ERR_GENERAL);
		}
		if( op_shared->m_type != parsed_type.m_operand_type )
		{
			printf_debug("USAGE/SYNTAX type mismatch\n");
			printf_debug("shared: %s=%s(%d), parsed: %s=%s(%d)\n", op_shared->m_name.c_str(), uvd_data_str(op_shared->m_type), op_shared->m_type, 
					parsed_type.m_name.c_str(), uvd_data_str(parsed_type.m_operand_type), parsed_type.m_operand_type);
			return UV_DEBUG(UV_ERR_GENERAL);
		}
		
		if( parsed_type.m_operand_type == UV_DISASM_DATA_IMMS
			|| parsed_type.m_operand_type == UV_DISASM_DATA_IMMU )
		{
			/* It is possible that DPTR may cause a few 16 bit immediates, but syntax does not yet support that */
			if( op_shared->m_immediate_size != parsed_type.m_num_bits )
			{
				printf_error("USAGE/SYNTAX size mismatch\n");
				return UV_DEBUG(UV_ERR_GENERAL);
			}
			inst_shared->m_total_length += op_shared->m_immediate_size / 8;
		}
		else if( parsed_type.m_operand_type == UV_DISASM_DATA_REG )
		{
			/* Mayber later some sort of Intel /r thing */
			printf_error("No registers during usage\n");
			return UV_DEBUG(UV_ERR_GENERAL);
		}
		else if( parsed_type.m_operand_type == UV_DISASM_DATA_FUNC )
		{
			/* Likely this will involve a fork onto the linked list */
			printf_error("Special modifier, not yet supported\n");
			return UV_DEBUG(UV_ERR_GENERAL);
		}
		else
		{
			printf_error("Unknown operand type\n");
			return UV_DEBUG(UV_ERR_GENERAL);
		}
	}
		
	return UV_ERR_OK;
}

uv_err_t UVDDisasmOpcodeLookupTable::init_opcode(UVDConfigSection *op_section)
{
	uv_err_t rc = UV_ERR_GENERAL;
	unsigned int cur_line = 0;
	unsigned int line_syntax = 0;
	unsigned int line_usage = 0;

	printf_debug("Initializing opcodes\n");
	if( op_section == NULL )
	{
		printf_debug(".OP section required\n");
		UV_ERR(rc);
		goto error;
	}
		
	printf_debug("Processing opcode lines...\n");
	cur_line = 0;
	for( ;;  )
	{
		//int line_pos = 0;
		//const std::string search = "\nNAME=";

		/* Its easier to parse some things before others */
		std::string value_desc;
		std::string value_usage;
		std::string value_syntax;
		std::string value_action;
		std::string value_cycles;
		std::string value_name;
		UVDDisasmInstructionShared *inst_shared = NULL;
		int primary_opcode = 0;

		
		printf_debug("\nLooking for next instruction block at index %d / %d\n", cur_line, op_section->m_lines.size());
		/* Start by extracting key/value pairs */
		for( ; cur_line < op_section->m_lines.size(); ++cur_line )
		{
			std::string::size_type equalsPos = 0;
			std::string key;
			std::string value;
			std::string line = op_section->m_lines[cur_line];

			printf_debug("Line: <%s>\n", line.c_str());
			
			/* Skip comments, empty */
			if( line.empty() || strstr(line.c_str(), "#") != NULL )
			{
				continue;
			}
			else if( line[0] == '.' )
			{
				printf_debug("Section parsing too crude to handle specified section ordering\n");
				UV_ERR(rc);
				goto error;
			}
			
			/* Setup key/value pairing */
			equalsPos = line.find("=");
			if( equalsPos == std::string::npos )
			{
				printf_debug("no key/value pair detected\n");
				UV_ERR(rc);
				goto error;
			}
			//Skip equals
			value = line.substr(equalsPos + 1);
			/* Skip the equals sign, make key comparisons easier */
			key = line.substr(0, equalsPos);
			
			printf_debug("key: <%s>, value: <%s>\n", key.c_str(), value.c_str());
	
			if( value_name.empty() && key != "NAME" )
			{
				printf_debug("NAME must go first\n");
				UV_ERR(rc);
				goto error;
			}
			
			if( !strcmp(key.c_str(), "NAME") )
			{
				printf_debug("Name found, value_name: %s\n", value_name.c_str());
				//If we alreayd have a name value, we are at the next entry
				if( !value_name.empty() )
				{
					break;
				}
				value_name = value;
			}
			else if( !strcmp(key.c_str(), "DESC") )
			{
				value_desc = value;
			}
			else if( !strcmp(key.c_str(), "USAGE") )
			{
				value_usage = value;
				line_usage = cur_line;
			}
			else if( !strcmp(key.c_str(), "SYNTAX") )
			{
				value_syntax = value;
				line_syntax = cur_line;
			}
			else if( !strcmp(key.c_str(), "ACTION") )
			{
				value_action = value;
			}
			else if( !strcmp(key.c_str(), "CYCLES") )
			{
				value_cycles = value;
			}
			else
			{
				printf_debug("Invalid key\n");
				UV_ERR(rc);
				goto error;
			}
		}

		/*
		NAME=XRL
		DESC=Bitwise Exclusive OR
		USAGE=0x6E
		SYNTAX=%A,%R6
		ACTION=nop
		CYCLES=1
		*/

		if( value_name.empty() )
		{
			printf_debug("No more config entries\n");
			break;
		}		

		inst_shared = new UVDDisasmInstructionShared();
		if( !inst_shared )
		{
			UV_ERR(rc);
			goto error;
		}
		printf_debug("Allocated new instruction, new: 0x%X\n", (unsigned int)inst_shared);	
		
		printf_debug("std::string value_name = %s\n", value_name.c_str());
		printf_debug("std::string value_desc = %s\n", value_desc.c_str());
		printf_debug("std::string value_usage = %s\n", value_usage.c_str());
		printf_debug("std::string value_syntax = %s\n", value_syntax.c_str());
		printf_debug("std::string value_action = %s\n", value_action.c_str());
		printf_debug("std::string value_cycles = %s\n", value_cycles.c_str());

		if( value_desc.empty() )
		{
			printf_debug("Description field missing\n");
			UV_ERR(rc);
			goto error;
		}

		if( value_usage.empty() )
		{
			printf_debug("Usage field missing\n");
			UV_ERR(rc);
			goto error;
		}

		if( value_action.empty() )
		{
			printf_debug("Action field missing\n");
			UV_ERR(rc);
			goto error;
		}

		inst_shared->m_config_line_syntax = op_section->m_line;
		//inst_shared->m_config_line_syntax = line_syntax;
		inst_shared->m_config_line_usage = op_section->m_line;
		//inst_shared->m_config_line_usage = line_usage;

		
		/* Trivial parsing */
		inst_shared->m_memoric = value_name;
		inst_shared->m_desc = value_desc;
		if( !value_cycles.empty() )
		{
			inst_shared->m_cpi = strtol(value_cycles.c_str(), NULL, 0);
		}
		else
		{
			inst_shared->m_cpi = 1;
		}

		printf_debug("*Parsing syntax\n");		
		if( UV_FAILED(uvd_parse_syntax(inst_shared, value_syntax)) )
		{
			printf_debug("Error parsing syntax line %d\n", inst_shared->m_config_line_syntax);
			UV_ERR(rc);
			goto error;
		}
				
		printf_debug("*Parsing usage\n");		
		if( UV_FAILED(uvd_parse_usage(inst_shared, value_usage)) )
		{
			printf_debug("Error parsing usage line %d\n", inst_shared->m_config_line_usage);
			UV_ERR(rc);
			goto error;
		}
		
		inst_shared->m_action = value_action;

		printf_debug("Storing processed\n");
		/*
		Make sure its valid 
		At a minimum need a name and a usage
		*/
		if( inst_shared->m_memoric.empty() || inst_shared->m_opcode_length == 0 )
		{
			UV_ERR(rc);
			goto error;
		}
		
		primary_opcode = inst_shared->m_opcode[0];
		printf_debug("Primary opcode: 0x%.2X\n", primary_opcode);
		/* Check for a repeated/conflicting opcode */
		printf_debug("table size: 0x%.8X\n", sizeof(m_lookupTable));
		if( m_lookupTable[primary_opcode] )
		{
			if( g_error_opcode_repeat )
			{
				printf_debug("Duplicate opcode: 0x%.2X, old: %s\n", primary_opcode, m_lookupTable[primary_opcode]->m_desc.c_str());
				UV_ERR(rc);
				goto error;
			}
		}

		
		//Compute some things to help speed up analysis and know the nature of this instruction
		uv_assert_err(inst_shared->analyzeAction());

		printf_debug("Doing actual store\n");
		m_lookupTable[primary_opcode] = inst_shared;
		printf_debug("Stored processed\n");
	}
	rc = UV_ERR_OK;

error:
	return UV_DEBUG(rc);
} 


uv_err_t UVDDisasmOpcodeLookupTable::init(UVDConfigSection *op_section)
{
	uv_err_t rc = UV_ERR_GENERAL;


	memset(m_lookupTable, 0, sizeof(m_lookupTable));
	
	if( UV_FAILED(init_opcode(op_section)) )
	{
		UV_ERR(rc);
		goto error;
	}
	
	usageStats();

	rc = UV_ERR_OK;

error:	
	return UV_DEBUG(rc);
}

uv_err_t UVDDisasmOpcodeLookupTable::deinit(void)
{
	for( unsigned i = 0; i < sizeof(m_lookupTable) / sizeof(m_lookupTable[0]); ++i )
	{
		delete m_lookupTable[i];
		m_lookupTable[i] = NULL; 
	}
	return UV_ERR_OK;
}

uv_err_t UVDDisasmOpcodeLookupTable::getElement(unsigned int index, UVDDisasmInstructionShared **element)
{
	uv_err_t rc = UV_ERR_GENERAL;

	uv_assert(element);
	uv_assert(index < 0x100);

	*element = m_lookupTable[index];
	++m_lookupTableHits[index];

	rc = UV_ERR_OK;

error:
	return UV_DEBUG(rc);
}

/* Used for opcode processing funcs */
//static uvd_func opcode_map[256];
/* on 8051, its a fairly starightforward map from numbers to opcode descriptions */
//UVDInstructionShared *m_lookupTable[256];

int g_error_opcode_repeat = 1;

/**********************
End init related
**********************/
