/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd.h"
#include "uvd_instruction.h"
#include "uvd_types.h"
#include "uvd_format.h"
#include "main.h"
#include <stdio.h>

const char *uvd_data_str(int uvd_data)
{
	switch(uvd_data)
	{
	case UV_DISASM_DATA_NONE:
		return "UV_DISASM_DATA_NONE";
	case UV_DISASM_DATA_REG:
		return "UV_DISASM_DATA_REG";
	case UV_DISASM_DATA_OPCODE:
		return "UV_DISASM_DATA_OPCODE";
	case UV_DISASM_DATA_IMMS:
		return "UV_DISASM_DATA_IMMS";
	case UV_DISASM_DATA_IMMU:
		return "UV_DISASM_DATA_IMMU";
	case UV_DISASM_DATA_FUNC:
		return "UV_DISASM_DATA_FUNC";
	default:
		return "<UNKNOWN>";
	}
}

UVDOperandShared::UVDOperandShared()
{
	m_type = 0;
	m_type_specific = NULL;
}

UVDOperandShared::~UVDOperandShared()
{
	deinit();
}

uv_err_t UVDOperandShared::deinit()
{
	switch( m_type )
	{
	case UV_DISASM_DATA_FUNC:
		delete m_func;
		m_func = NULL;
		break;
	}
	m_type = UV_DISASM_DATA_NONE;
	
	return UV_ERR_OK;
}

UVDOperand::UVDOperand()
{
	m_instruction = NULL;
	m_shared = NULL;
	m_extra = NULL;	
}

UVDOperand::~UVDOperand()
{
	deinit();
}

uv_err_t UVDOperand::deinit()
{
	if( m_shared )
	{
		switch( m_shared->m_type )
		{
		case UV_DISASM_DATA_FUNC:
			delete m_func;
			m_func = NULL;
			break;
		}
	}
	return UV_ERR_OK;
}

uv_err_t UVDOperand::parseOperand(UVDIteratorCommon *uvdIter)
{
	UVDInstruction *inst = NULL;
	UVDData *data = NULL;
	UVD *uvd = NULL;
	uv_err_t rcNextAddress = UV_ERR_GENERAL;
	int read = -1;

	uv_assert_ret(m_instruction);
	inst = m_instruction;
	uv_assert_ret(inst);
	uvd = inst->m_uvd;
	uv_assert_ret(uvd);
	data = uvd->getData();
	uv_assert_ret(data);

	uv_err_t rc = UV_ERR_GENERAL;
	
	printf_debug("Type switch\n");
	//printf_debug("Type switch type: %s (%d)\n", uvd_data_str(m_shared->m_type), m_shared->m_type); fflush(stdout);		
	/* See if it takes up any space */
	switch( m_shared->m_type )
	{
	/* Parsing signed and unsigned nums really is no different, raw binary data is identical */
	case UV_DISASM_DATA_IMMS:
	case UV_DISASM_DATA_IMMU:
	{
		switch(m_shared->m_immediate_size)
		{
		case 8:
			read = data->read(uvdIter->m_nextPosition);	
			uv_assert_ret(read >= 0);
			m_ui8 = read;
			rcNextAddress = uvdIter->nextValidExecutableAddress();
			uv_assert_err_ret(rcNextAddress);
			if( rcNextAddress == UV_ERR_DONE )
			{
				//Premature termination
				return UV_ERR_DONE;
			}
			
			break;
		case 16:
			read = data->read(uvdIter->m_nextPosition);
			uv_assert_ret(read >= 0);
			m_ui16 = read << 8;
			printf_debug("read operand imm16 @ 0x%.4X is 0x%.4X\n", uvdIter->m_nextPosition, m_ui16);
			rcNextAddress = uvdIter->nextValidExecutableAddress();
			uv_assert_err_ret(rcNextAddress);
			if( rcNextAddress == UV_ERR_DONE )
			{
				//Premature termination
				return UV_ERR_DONE;
			}
			
			read = data->read(uvdIter->m_nextPosition);
			uv_assert_ret(read >= 0);
			m_ui16 += read;	
			printf_debug("read operand imm16 @ 0x%.4X is 0x%.4X\n", uvdIter->m_nextPosition, m_ui16);
			rcNextAddress = uvdIter->nextValidExecutableAddress();
			uv_assert_err_ret(rcNextAddress);
			if( rcNextAddress == UV_ERR_DONE )
			{
				//Pre-mature termination
				return UV_ERR_DONE;
			}

			break;
		default:
			UV_ERR(rc);
			goto error;
		}
		break;
	}
	/*
	As of now, no registers require parsing.
	Depending how Intel ModR/M is implemented, this may change
	Likely ModR/M will be a config file macro that would be as if you manually typed out each opcode entry
	*/
	case UV_DISASM_DATA_REG:
	{
		printf_debug("Register: %s\n", m_shared->m_name.c_str());
		break;
	}
	case UV_DISASM_DATA_FUNC:
	{
		printf_debug("Func\n");

		m_func = new UVDFunction();
		if( !m_func )
		{
			UV_ERR(rc);
			goto error;
		}
		printf_debug("Function/modifier: %s\n", m_shared->m_name.c_str());
		uv_assert_err_ret(inst);
		if( UV_FAILED(inst->parseOperands(uvdIter, m_shared->m_func->m_args, m_func->m_args)) )
		{
			UV_ERR(rc);
			goto error;
		}
		uv_assert_ret(m_shared->m_func->m_args.size() == m_func->m_args.size());
		
		break;
	}
	//I don't tihnk this one makes sense for this, they are in another place
	//UV_DISASM_DATA_OPCODE
	default:
		printf_debug("Bad operand type: %s(%d)\n", uvd_data_str(m_shared->m_type), m_shared->m_type);
		UV_ERR(rc);
		goto error;
	}
	
	rc = UV_ERR_OK;

error:
	return UV_DEBUG(rc);
}

uv_err_t UVDOperand::getVariable(std::string &name, UVDVarient &value)
{
	uv_err_t rc = UV_ERR_GENERAL;

	uv_assert_err_ret(m_shared);

	switch( m_shared->m_type )
	{
	case UV_DISASM_DATA_REG:
	{
		/*
		Ignore for now...?
		Registers will get mapped seperately in a sort of VM
		
		std::string sName = op->m_shared->m_name;
		if( sName ==  )
		{
		}
		*/
		break;
	}
	case UV_DISASM_DATA_IMMS:
	{
		name = m_shared->m_name;
		switch( m_shared->m_immediate_size )
		{
		case 8:
			value = UVDVarient(m_i8);
			break;
		case 16:
			value = UVDVarient(m_i16);
			break;
		case 32:
			value = UVDVarient(m_i32);
			break;
		default:
			return UV_DEBUG(UV_ERR_GENERAL);
		}
		break;
	}
	case UV_DISASM_DATA_IMMU:
	{
		name = m_shared->m_name;
		switch( m_shared->m_immediate_size )
		{
		case 8:
			value = UVDVarient(m_ui8);
			break;
		case 16:
			value = UVDVarient(m_ui16);
			break;
		case 32:
			value = UVDVarient(m_ui32);
			break;
		default:
			return UV_DEBUG(UV_ERR_GENERAL);
		}
		break;
	}
	case UV_DISASM_DATA_FUNC:
		break;
	default:
		break;
	}

	rc = UV_ERR_OK;

	return UV_DEBUG(rc);
}

int g_fail_no_sym = 1;

uv_err_t UVDOperand::printDisassemblyOperand(std::string &out)
{
	char buff[256];
	uv_assert_err_ret(print_disasm_operand(buff, 256, NULL));
	out += buff;
	return UV_ERR_OK;
}

uv_err_t UVDOperand::print_disasm_operand(char *buff, unsigned int buffsz, unsigned int *buff_used_in)
{
	uv_err_t rc = UV_ERR_GENERAL;
	unsigned int pos = 0;
	
	UV_ENTER();

	uv_assert(buff);
	uv_assert(m_shared);
	uv_assert_ret(g_config);

	/*
	//To look for operand corruption
	if( true )
	{
		if( buff_used_in )
		{
			*buff_used_in = 0;
		}
		return UV_ERR_OK;
	}
	*/

	/*
	Code written as is right now may violate this, probably should do more severe checks
	snprintf will return number would have been written, so could make buffsz negative
	*/
	printf_debug("print type: %s\n", uvd_data_str(m_shared->m_type));
	pos = 0;
	uv_assert(pos < buffsz);
	
	switch( m_shared->m_type )
	{
	case UV_DISASM_DATA_REG:
	{
		pos += snprintf(buff, buffsz, "%s", m_shared->m_name.c_str());
		break;
	}
	case UV_DISASM_DATA_IMMS:
	case UV_DISASM_DATA_IMMU:
	{
		char format_string[32];
		char int_formatter[8];
		//char localBuff[256];
		
		std::string immediatePrint;
		uint32_t print_int = 0;
		UVD *uvd = NULL;
		UVDAnalyzer *analyzer = NULL;
		
		uv_assert_ret(m_instruction);
		uvd = m_instruction->m_uvd;
		uv_assert_ret(uvd);
		analyzer = uvd->m_analyzer;
		uv_assert_ret(analyzer);
		
		//FIXME: rework this ugly formatting code
		//You'd never guess, but doing this sort of formatting has led to some seg faults
		//Maybe should rework
		
		//Do we have an analyzed sustitute?
		UVDVariableMap analysisResult;
		if( UV_SUCCEEDED(analyzer->readCache(m_instruction->m_offset, analysisResult)) )
		{
			std::string sAddr;
			int printWidth = 8;
			bool isAddress = false;
			
			if( analysisResult.find(SCRIPT_KEY_JUMP) != analysisResult.end() )
			{
				analysisResult[SCRIPT_KEY_JUMP].getString(sAddr);			
				printWidth = 4;
				isAddress = true;
			}
			else if( analysisResult.find(SCRIPT_KEY_CALL) != analysisResult.end() )
			{
				analysisResult[SCRIPT_KEY_CALL].getString(sAddr);			
				printWidth = 4;
				isAddress = true;
			}
			else
			{
				return UV_DEBUG(UV_ERR_GENERAL);
			}
			
			printf_debug("analysis cache address hit, value: %s\n", sAddr.c_str());
			print_int = (unsigned int)strtol(sAddr.c_str(), NULL, 0);
						
			if( isAddress )
			{
				UVDVariableMap addressAnalysisResult;
				if( UV_SUCCEEDED(analyzer->readCache(print_int, analysisResult)) )
				{
					//Symbol representation of some address?
					if( analysisResult.find(SCRIPT_KEY_SYMBOL) != analysisResult.end() )
					{
						std::string sSymbol;
						analysisResult[SCRIPT_KEY_SYMBOL].getString(sSymbol);								
					}
				}
			}

			snprintf(int_formatter, 8, "0x%%.%dX", printWidth);
		}
		else
		{
			//For signed values, print as decimal
			if( m_shared->m_type == UV_DISASM_DATA_IMMS )
			{
				snprintf(int_formatter, 8, "%%d");
				uv_assert_err_ret(getI32RepresentationAdjusted((int32_t &)print_int));
			}
			//For unsigned values, print as hex
			else
			{
				snprintf(int_formatter, 8, "0x%%.%dX", m_shared->m_immediate_size / 4);
				uv_assert_err_ret(getUI32RepresentationAdjusted(print_int));
			}
		}
		snprintf(format_string, 32, "%%s%%s%s%%s%%s", int_formatter);
		
		printf_debug("Print string: %s, pre: <%s>, post: <%s>\n", format_string, g_config->m_asm_imm_prefix.c_str(), g_config->m_asm_imm_suffix.c_str());
		
		pos += snprintf(buff, buffsz, format_string, g_config->m_asm_imm_prefix.c_str(), g_config->m_asm_imm_prefix_hex.c_str(), print_int, g_config->m_asm_imm_postfix_hex.c_str(), g_config->m_asm_imm_suffix.c_str());
		break;
	}
	case UV_DISASM_DATA_FUNC:
	{
		UVDSymbol *sym_value = NULL;
		std::string functionName;
		
		functionName = m_shared->m_name;
		
		printf_debug("Print func\n");
		printf_debug("Print func, args: %d, func: %s\n", m_func->m_args.size(), functionName.c_str());
		
		//FIXME: global UVD ref
		uv_assert_ret(g_uvd);
		uv_assert_ret(g_uvd->m_symMap);
		if( UV_SUCCEEDED(g_uvd->m_symMap->getSym(functionName, &sym_value)) )
		{
			printf_debug("Got sym\n");
			uv_assert(sym_value);
			//Formatting information for a type of memory?
			if( sym_value->m_type == UVD_SYMBOL_MEM )
			{
				UVDMemoryShared *mem_shared = NULL;
				std::string equiv_name;
				UVDOperand *mem_arg = NULL;

				uv_assert(pos < buffsz);
				printf_debug("Memory\n");
				mem_shared = sym_value->m_mem;
				printf_debug("Memory shared name: %s, prefix: <%s>, suffix: <%s>\n", mem_shared->m_name.c_str(), 
						mem_shared->m_print_prefix.c_str(), mem_shared->m_print_suffix.c_str());

				mem_arg = m_func->m_args[0];
				uv_assert(mem_arg);
				uv_assert(mem_arg->m_shared);
			
				printf_debug("mem operand type: %s\n", uvd_data_str(mem_arg->m_shared->m_type));
				if( mem_arg->m_shared->m_type == UV_DISASM_DATA_REG )
				{
					/* Already resolved */
					uv_assert(pos < buffsz);
					pos += snprintf(buff, buffsz, "%s%s%s", mem_shared->m_print_prefix.c_str(), mem_arg->m_shared->m_name.c_str(), mem_shared->m_print_suffix.c_str());
					uv_assert(pos < buffsz);
				}
				/* 
				Addresses can't be negative, so don't check for IMMS
				A relative address might be able to be, but would that ever have a special name?
				*/
				else if( mem_arg->m_shared->m_type == UV_DISASM_DATA_IMMU )
				{
					//UVDMemoryShared *mem_loc = NULL;
					//UVDSymbol *sym_temp = NULL;
					std::string equiv_name;

					/*
					mem_loc.m_space = mem_shared;
					mem_loc.m_min_addr = mem_arg->m_ui32;
					mem_loc.m_max_addr = mem_loc.m_min_addr + mem_arg->m_shared->m_immediate_size - 1;
					*/
					/* Check for a known register mapping */
					uint32_t address = mem_arg->m_ui32;
					if( UV_SUCCEEDED(mem_shared->getEquivMemName(address, equiv_name)) )
					{
						printf_debug("Equiv: yes, %d -> %s\n", address, equiv_name.c_str());
						uv_assert(pos < buffsz);
						pos += snprintf(buff, buffsz, "%s", equiv_name.c_str());
						uv_assert(pos < buffsz);
					}
					/* Otherwise get info for that particular memory type */
					else
					{
						/* This is needed to ensure proper number of leading zeros */
						char format_string[32];

						printf_debug("Equiv: no\n");

						uv_assert(pos < buffsz);
						pos += snprintf(buff + pos, buffsz - pos, "%s", mem_shared->m_print_prefix.c_str());
						uv_assert(pos < buffsz);

						//These are just for formatting right now
						uv_assert_all(m_func->m_args.size() == 1);
						uv_assert_all(mem_arg);
						uv_assert_all(mem_arg->m_shared);
						uv_assert_all(mem_arg->m_shared->m_type == UV_DISASM_DATA_IMMU);
						
						printf_debug("Immediate size: %d\n", mem_arg->m_shared->m_immediate_size);					
						uv_assert(mem_arg->m_shared->m_immediate_size < 128);
						snprintf(format_string, 32, "%%s%%.%dX", mem_arg->m_shared->m_immediate_size / 4);
						printf_debug("format string: %s\n", format_string);
						
						uv_assert(pos < buffsz);
						printf_debug("imm prefix hex: <%s>, ui32: %d, %X\n", g_config->m_asm_imm_prefix_hex.c_str(), mem_arg->m_ui32, mem_arg->m_ui32);
						pos += snprintf(buff + pos, buffsz - pos, format_string, g_config->m_asm_imm_prefix_hex.c_str(), mem_arg->m_ui32);
						uv_assert(pos < buffsz);

						/*
						if( UV_FAILED(print_disasm_operand(m_func->m_args[0], buff + pos, buffsz - pos, &last_usage)) )
						{
							UV_ERR(rc);
							goto error;
						}
						pos += last_usage;
						*/

						printf_debug("print middle, mem_shared: %d, buff size: %d, buff pos: %d\n", (unsigned int)mem_shared, buffsz, pos);
						uv_assert(pos < buffsz);
						printf_debug("Buff: <%s>\n", buff);
						pos += snprintf(buff + pos, buffsz - pos, "%s", mem_shared->m_print_suffix.c_str());
						uv_assert(pos < buffsz);
						printf_debug("print done\n");
					}
				}
				else
				{
					UV_ERR(rc);
					goto error;
				}
			}
			else if( sym_value->m_type == UVD_SYMBOL_OPERATOR )
			{
				UVDOperator *pOperator = NULL;
				std::string resultString;
			
				uv_assert_err_ret(sym_value->getValue(&pOperator));
				uv_assert_ret(pOperator);
				
				printf_debug("Operator: %s, args: %d\n", pOperator->toString().c_str(), m_func->m_args.size());
			
				if( m_func->m_args.size() == 1 )
				{
					//How to tell if prefix/postfix?
					//Will likely have to do a switch or such
					UVDOperand *uniaryOperand = NULL;
					std::string uniaryString;
					
					uniaryOperand = m_func->m_args[0];
					uv_assert_ret(uniaryOperand);
					
					uv_assert_err_ret(uniaryOperand->printDisassemblyOperand(uniaryString));
					
					resultString = pOperator->toString() + " " + uniaryString;
				}
				//Assume binary operator
				else if( m_func->m_args.size() == 2 )
				{
					UVDOperand *lOperand = NULL;
					UVDOperand *rOperand = NULL;
					std::string lString;
					std::string rString;
					
					lOperand = m_func->m_args[0];
					uv_assert_ret(lOperand);
					rOperand = m_func->m_args[1];
					uv_assert_ret(rOperand);

					uv_assert_err_ret(lOperand->printDisassemblyOperand(lString));
					uv_assert_err_ret(rOperand->printDisassemblyOperand(rString));
					
					resultString = lString + " " + pOperator->toString() + " " + rString;
				}
				else
				{
					UV_ERR(rc);
					goto error;
				}
				
				pos += snprintf(buff + pos, buffsz - pos, "%s", resultString.c_str());
				uv_assert(pos < buffsz);
			}
			else
			{
				UV_ERR(rc);
				goto error;
			}
		}
		else
		{
			unsigned int cur_arg_index = 0;

			if( g_fail_no_sym )
			{
				UV_DEBUG(rc);
				goto error;
			}
			
			printf_warn("Warning: unknown symbol %s, printing args\n", m_shared->m_name.c_str());
			/* At this time, all funcs have args */
			uv_assert(m_func->m_args.size());
			uv_assert(pos < buffsz);
			pos += snprintf(buff + pos, buffsz - pos, "%s(", m_shared->m_name.c_str());
			uv_assert(pos < buffsz);
			for( cur_arg_index = 0; cur_arg_index < m_func->m_args.size(); ++cur_arg_index )
			{
				UVDOperand *op_arg = NULL;
				unsigned int last_usage = 0;

				printf_debug("Print func arg\n");
				if( buffsz <= pos )
				{
					printf_debug("WARNING: buffer out of room\n");
					UV_ERR(rc);
					break;
				}

				op_arg = m_func->m_args[cur_arg_index];
				uv_assert(op_arg);
				if( UV_FAILED(op_arg->print_disasm_operand(buff + pos, buffsz - pos, &last_usage)) )
				{
					UV_ERR(rc);
					goto error;
				}
				pos += last_usage;
				pos += snprintf(buff + pos, buffsz - pos, "%s", ")");
			}
		}
		break;		
	}
	default:
		printf_debug("Unknown operand type: %d\n", m_shared->m_type);
		UV_ERR(rc);
		goto error;
	}

	/* Should always print something I would think */
	printf_debug("Pos: %d\n", pos);
	uv_assert(pos > 0);
	uv_assert(buff[0] != 0);
	printf_debug("Buff: <%s>\n", buff);

	if( buff_used_in )
	{
		*buff_used_in = pos;
	}
	
	printf_debug("Post operand buff: <%s>\n", buff);

	rc = UV_ERR_OK;
	
error:
	return UV_DEBUG(rc);	
}

uv_err_t UVDOperand::getUI32Representation(uint32_t &i)
{
	if( m_shared->m_type != UV_DISASM_DATA_IMMU )
	{
		return UV_ERR_GENERAL;
	}
	
	//All types should just get promoted cleanly
	i = m_ui32;
	return UV_ERR_OK;
}

uv_err_t UVDOperand::getI32RepresentationAdjusted(int32_t &i)
{
	if( m_shared->m_type != UV_DISASM_DATA_IMMS )
	{
		return UV_ERR_GENERAL;
	}
	
	uint32_t varSize = m_shared->m_immediate_size;
	
	//Binary storage means issues arrise.  Ex: 0xFF is -1 as i8_t, but translates to 255 when represented as i32_t
	if( varSize <= 8 )
	{
		i = m_i8;
	}
	else if( varSize <= 16 )
	{
		i = m_i16;
	}
	else if( varSize <= 32 )
	{
		i = m_i32;
	}
	else
	{
		//More of a severe error
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	
	return UV_ERR_OK;
}

uv_err_t UVDOperand::getUI32RepresentationAdjusted(uint32_t &i)
{
	if( m_shared->m_type != UV_DISASM_DATA_IMMU )
	{
		return UV_ERR_GENERAL;
	}
	
	//All types should just get promoted cleanly
	i = m_ui32;
	return UV_ERR_OK;
}

uv_err_t UVDOperand::getI32Representation(int32_t &i)
{
	if( m_shared->m_type != UV_DISASM_DATA_IMMS )
	{
		return UV_ERR_GENERAL;
	}
	
	uint32_t varSize = m_shared->m_immediate_size;
	
	//Binary storage means issues arrise.  Ex: 0xFF is -1 as i8_t, but translates to 255 when represented as i32_t
	if( varSize <= 8 )
	{
		i = m_i8;
	}
	else if( varSize <= 16 )
	{
		i = m_i16;
	}
	else if( varSize <= 32 )
	{
		i = m_i32;
	}
	else
	{
		//More of a severe error
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	
	return UV_ERR_OK;
}

UVDFunctionShared::UVDFunctionShared()
{
}

UVDFunctionShared::~UVDFunctionShared()
{
	deinit();
}

uv_err_t UVDFunctionShared::deinit()
{
	for( std::vector<UVDOperandShared *>::iterator iter = m_args.begin(); iter != m_args.end(); ++iter )
	{
		delete *iter;
	}
	m_args.clear();

	return UV_ERR_OK;
}

UVDFunction::UVDFunction()
{
}

UVDFunction::~UVDFunction()
{
	deinit();
}

uv_err_t UVDFunction::deinit()
{
	for( std::vector<UVDOperand *>::iterator iter = m_args.begin(); iter != m_args.end(); ++iter )
	{
		delete *iter;
	}
	m_args.clear();

	return UV_ERR_OK;
}
