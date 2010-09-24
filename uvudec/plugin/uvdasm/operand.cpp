/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd.h"
#include "uvdasm/instruction.h"
#include "uvd_types.h"
#include "uvd_format.h"
#include "uvdasm/operand.h"
#include "uvdasm/util.h"
#include "main.h"
#include "uvdasm/architecture.h"
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

/*
UVDDisasmOperandShared
*/

UVDDisasmOperandShared::UVDDisasmOperandShared()
{
	m_type = 0;
	m_type_specific = NULL;
}

UVDDisasmOperandShared::~UVDDisasmOperandShared()
{
	deinit();
}

uv_err_t UVDDisasmOperandShared::deinit()
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

uv_err_t UVDDisasmOperandShared::uvd_parsed2opshared(const UVDConfigValue *parsed_type, UVDDisasmOperandShared **op_shared_in)
{
	uv_err_t rc = UV_ERR_GENERAL;
	UVDDisasmOperandShared *op_shared = NULL;

	UV_ENTER();
	
	uv_assert(parsed_type);

	if( !op_shared_in )
	{
		UV_ERR(rc);
		goto error;
	}
	if( parsed_type->m_operand_type == UV_DISASM_DATA_IMMS
		|| parsed_type->m_operand_type == UV_DISASM_DATA_IMMU )
	{
		op_shared = new UVDDisasmOperandShared();
		if( op_shared == NULL )
		{
			UV_ERR(rc);
			goto error;
		}
		/* Cur will be free'd soon and has other issues, so duplicate */
		op_shared->m_name = parsed_type->m_name;
		
		op_shared->m_type = parsed_type->m_operand_type;
							
		op_shared->m_immediate_size = parsed_type->m_num_bits;
		//inst_shared->m_total_length += op_shared->m_immediate_size / 8;			
	}
	else if( parsed_type->m_operand_type == UV_DISASM_DATA_REG )
	{
		printf_debug("Register: %s\n", parsed_type->m_name.c_str());
				
		op_shared = new UVDDisasmOperandShared();
		if( op_shared == NULL )
		{
			UV_ERR(rc);
			goto error;
		}
		/* Cur will be free'd soon and has other issues, so duplicate */
		op_shared->m_name = parsed_type->m_name;
		op_shared->m_type = parsed_type->m_operand_type;
	}
	else if( parsed_type->m_operand_type == UV_DISASM_DATA_FUNC )
	{
		unsigned int i = 0;

		op_shared = new UVDDisasmOperandShared();
		if( op_shared == NULL )
		{
			UV_ERR(rc);
			goto error;
		}
		/* Cur will be free'd soon and has other issues, so duplicate */
		op_shared->m_name = parsed_type->m_name;

		op_shared->m_type = parsed_type->m_operand_type;
		op_shared->m_immediate_size = parsed_type->m_num_bits;
		op_shared->m_func = new UVDDisasmFunctionShared();
		if( op_shared->m_func == NULL )
		{
			UV_ERR(rc);
			goto error;
		}

		for( i = 0; i < parsed_type->m_func->m_args.size(); ++i )
		{
			UVDDisasmOperandShared *op_shared_local = NULL;
			
			/*
			op_shared_local = new UVDOperandShared();
			if( !sop_shared_local )
			{
				UV_ERR(rc);
				goto error;
			}
			*/
			if( UV_FAILED(UVDDisasmOperandShared::uvd_parsed2opshared(parsed_type->m_func->m_args[i], &op_shared_local)) )
			{
				UV_ERR(rc);
				goto error;
			}
			uv_assert(op_shared_local);
			op_shared->m_func->m_args.push_back(op_shared_local);
		}
	}
	else
	{
		UV_ERR(rc);
		goto error;
	}
	*op_shared_in = op_shared;
	rc = UV_ERR_OK;

error:
	return UV_DEBUG(rc);
}

/*
uv_err_t UVDDisasmOperandShared::getImmediateSize(uint32_t *immediateSizeOut)
{
	return UV_ERR_OK;
}
*/

/*
UVDDisasmOperand
*/

UVDDisasmOperand::UVDDisasmOperand()
{
	m_extra = NULL;	
}

UVDDisasmOperand::~UVDDisasmOperand()
{
	UV_DEBUG(deinit());
}

uv_err_t UVDDisasmOperand::init()
{
	return UV_ERR_OK;
}

uv_err_t UVDDisasmOperand::deinit()
{
	//printf("this UVDOperand under deinit: 0x%08X\n", (int)this);
	fflush(stdout);
	//FIXME: a test
	m_shared = NULL;
	if( m_shared )
	{
		switch( getShared()->m_type )
		{
		case UV_DISASM_DATA_FUNC:
			delete m_func;
			m_func = NULL;
			break;
		}
	}
	return UV_ERR_OK;
}

UVDDisasmOperandShared *UVDDisasmOperand::getShared()
{
	return (UVDDisasmOperandShared *)m_shared;
}

uv_err_t UVDDisasmOperand::parseOperand(UVDIteratorCommon *uvdIter)
{
	UVDDisasmInstruction *inst = NULL;
	UVDDisasmOperandShared *operandShared = (UVDDisasmOperandShared *)m_shared;
	UVDData *data = NULL;
	UVD *uvd = NULL;
	uv_err_t rcNextAddress = UV_ERR_GENERAL;
	uint8_t read = 0;

	uv_assert_ret(m_instruction);
	inst = (UVDDisasmInstruction *)m_instruction;
	uv_assert_ret(inst);
	uvd = inst->m_uvd;
	uv_assert_ret(uvd);
	data = uvd->getData();
	uv_assert_ret(data);

	uv_err_t rc = UV_ERR_GENERAL;
	
	printf_debug("Type switch\n");
	//printf_debug("Type switch type: %s (%d)\n", uvd_data_str(operandShared->m_type), operandShared->m_type); fflush(stdout);		
	/* See if it takes up any space */
	switch( operandShared->m_type )
	{
	/* Parsing signed and unsigned nums really is no different, raw binary data is identical */
	case UV_DISASM_DATA_IMMS:
	case UV_DISASM_DATA_IMMU:
	{
		switch(operandShared->m_immediate_size)
		{
		case 8:
			rcNextAddress = uvdIter->consumeCurrentExecutableAddress(&read);
			uv_assert_err_ret(rcNextAddress);
			if( rcNextAddress == UV_ERR_DONE )
			{
				//Premature termination
				return UV_ERR_DONE;
			}
			m_ui8 = read;
			
			break;
		case 16:
			rcNextAddress = uvdIter->consumeCurrentExecutableAddress(&read);
			uv_assert_err_ret(rcNextAddress);
			if( rcNextAddress == UV_ERR_DONE )
			{
				//Premature termination
				return UV_ERR_DONE;
			}
			//printf_debug("read operand imm16 @ 0x%.4X is 0x%.4X\n", uvdIter->m_nextPosition, m_ui16);
			m_ui16 = read << 8;
			
			rcNextAddress = uvdIter->consumeCurrentExecutableAddress(&read);
			uv_assert_err_ret(rcNextAddress);
			if( rcNextAddress == UV_ERR_DONE )
			{
				//Premature termination
				return UV_ERR_DONE;
			}
			m_ui16 += read;	
			
			printf_debug("read operand imm16 @ 0x%.4X is 0x%.4X\n", uvdIter->m_nextPosition, m_ui16);

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
		printf_debug("Register: %s\n", operandShared->m_name.c_str());
		break;
	}
	case UV_DISASM_DATA_FUNC:
	{
		printf_debug("Func\n");

		m_func = new UVDDisasmFunction();
		if( !m_func )
		{
			UV_ERR(rc);
			goto error;
		}
		printf_debug("Function/modifier: %s\n", operandShared->m_name.c_str());
		uv_assert_err_ret(inst);
		if( UV_FAILED(inst->parseOperands(uvdIter, operandShared->m_func->m_args, m_func->m_args)) )
		{
			UV_ERR(rc);
			goto error;
		}
		uv_assert_ret(operandShared->m_func->m_args.size() == m_func->m_args.size());
		
		break;
	}
	//I don't tihnk this one makes sense for this, they are in another place
	//UV_DISASM_DATA_OPCODE
	default:
		printf_debug("Bad operand type: %s(%d)\n", uvd_data_str(operandShared->m_type), operandShared->m_type);
		UV_ERR(rc);
		goto error;
	}
	
	rc = UV_ERR_OK;

error:
	return UV_DEBUG(rc);
}

uv_err_t UVDDisasmOperand::getVariable(std::string &name, UVDVarient &value)
{
	uv_assert_err_ret(getShared());

	switch( getShared()->m_type )
	{
	case UV_DISASM_DATA_REG:
	{
		/*
		Ignore for now...?
		Value cannot be determinted by static analysis
			Usually, and defintly not easily
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
		name = getShared()->m_name;
		switch( getShared()->m_immediate_size )
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
		name = getShared()->m_name;
		switch( getShared()->m_immediate_size )
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
		//Recurse
		//Only one of these should actually be coding (or it would be a seperate operand)
		//really, funcs should only contain one operand anyway
		//they were intended as a way of tagging vars
		//When the UVD assembler engine is put into a plugin, core engine should use tags instead of funcs
		for( std::vector<UVDOperand *>::iterator iter = m_func->m_args.begin(); iter != m_func->m_args.end(); ++iter )
		{
			UVDDisasmOperand *operand = (UVDDisasmOperand *)*iter;
			
			uv_assert_err_ret(operand->getVariable(name, value));
			if( !name.empty() )
			{
				break;
			}
		}
		break;
	default:
		break;
	}

	return UV_ERR_OK;
}

int g_fail_no_sym = 1;

uv_err_t UVDDisasmOperand::printDisassemblyOperand(std::string &out)
{
	uv_err_t rc = UV_ERR_GENERAL;
	UVDDisasmArchitecture *architecture = NULL;
 	
 	architecture = (UVDDisasmArchitecture *)g_uvd->m_architecture;
//printf("printing operand\n");

	//uv_assert(buff);
	uv_assert(getShared());
	uv_assert_ret(g_config);

	/*
	Code written as is right now may violate this, probably should do more severe checks
	snprintf will return number would have been written, so could make buffsz negative
	*/
	printf_debug("print type: %s\n", uvd_data_str(getShared()->m_type));
	
	switch( getShared()->m_type )
	{
	case UV_DISASM_DATA_REG:
	{
		out += getShared()->m_name;
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
		uvd = g_uvd;
		uv_assert_ret(uvd);
		analyzer = uvd->m_analyzer;
		uv_assert_ret(analyzer);
		
		//FIXME: rework this ugly formatting code
		//You'd never guess, but doing this sort of formatting has led to some seg faults
		//Maybe should rework
		
		//Do we have an analyzed sustitute?
		UVDVariableMap analysisResult;
		if( UV_SUCCEEDED(architecture->readCache(m_instruction->m_offset, analysisResult)) )
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
				if( UV_SUCCEEDED(architecture->readCache(print_int, analysisResult)) )
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
//printf("int formatter: %s\n", int_formatter);
		}
		else
		{
			//For signed values, print as decimal
			if( getShared()->m_type == UV_DISASM_DATA_IMMS )
			{
				snprintf(int_formatter, 8, "%%d");
				uv_assert_err_ret(getI32RepresentationAdjusted((int32_t &)print_int));
			}
			//For unsigned values, print as hex
			else
			{
				snprintf(int_formatter, 8, "0x%%.%dX", getShared()->m_immediate_size / 4);
				uv_assert_err_ret(getUI32RepresentationAdjusted(print_int));
			}
		}
		snprintf(format_string, 32, "%%s%%s%s%%s%%s", int_formatter);
		
		printf_debug("Print string: %s, pre: <%s>, post: <%s>\n", format_string, g_config->m_asm_imm_prefix.c_str(), g_config->m_asm_imm_suffix.c_str());
		
		char buff[256];
		snprintf(buff, sizeof(buff), format_string, g_config->m_asm_imm_prefix.c_str(), g_config->m_asm_imm_prefix_hex.c_str(), print_int, g_config->m_asm_imm_postfix_hex.c_str(), g_config->m_asm_imm_suffix.c_str());
		out += buff;
		break;
	}
	/*
	CHECKME: there is commented out code where we use to recurse instead of manually printing junk again
	Why aren't we recursing like we use to?  Was the prefix/postfix context the issue?
	*/
	case UV_DISASM_DATA_FUNC:
	{
		UVDSymbol *sym_value = NULL;
		std::string functionName;
		UVDDisasmArchitecture *architecture = NULL;
		
		functionName = getShared()->m_name;
		
		printf_debug("Print func\n");
		printf_debug("Print func, args: %d, func: %s\n", m_func->m_args.size(), functionName.c_str());
		
		//FIXME: global UVD ref
		uv_assert_ret(g_uvd);
		uv_assert_ret(g_uvd->m_architecture);
		architecture = (UVDDisasmArchitecture *)g_uvd->m_architecture;
		uv_assert_ret(architecture->m_symMap);
		if( UV_SUCCEEDED(architecture->m_symMap->getSym(functionName, &sym_value)) )
		{
			printf_debug("Got sym\n");
			uv_assert(sym_value);
			//Formatting information for a type of memory?
			if( sym_value->m_type == UVD_SYMBOL_MEM )
			{
				UVDAddressSpace *mem_shared = NULL;
				std::string equiv_name;
				UVDDisasmOperand *memArg = NULL;
				UVDDisasmOperandShared *memArgShared = NULL;

				printf_debug("Memory\n");
				mem_shared = sym_value->m_mem;
				printf_debug("Memory shared name: %s, prefix: <%s>, suffix: <%s>\n", mem_shared->m_name.c_str(), 
						mem_shared->m_print_prefix.c_str(), mem_shared->m_print_suffix.c_str());

				memArg = (UVDDisasmOperand *)m_func->m_args[0];
				uv_assert(memArg);
				memArgShared = (UVDDisasmOperandShared *)memArg->m_shared;
				uv_assert(memArgShared);
			
				printf_debug("mem operand type: %s\n", uvd_data_str(memArgShared->m_type));
				if( memArgShared->m_type == UV_DISASM_DATA_REG )
				{
					/* Already resolved */
					out += mem_shared->m_print_prefix;
					out += memArgShared->m_name;
					out += mem_shared->m_print_suffix;
				}
				/* 
				Addresses can't be negative, so don't check for IMMS
				A relative address might be able to be, but would that ever have a special name?
				*/
				else if( memArgShared->m_type == UV_DISASM_DATA_IMMU )
				{
					//UVDAddressSpace *mem_loc = NULL;
					//UVDSymbol *sym_temp = NULL;
					std::string equiv_name;

					/*
					mem_loc.m_space = mem_shared;
					mem_loc.m_min_addr = memArg->m_ui32;
					mem_loc.m_max_addr = mem_loc.m_min_addr + memArgShared->m_immediate_size - 1;
					*/
					/* Check for a known register mapping */
					uint32_t address = memArg->m_ui32;
					if( UV_SUCCEEDED(mem_shared->getEquivMemName(address, equiv_name)) )
					{
						printf_debug("Equiv: yes, %d -> %s\n", address, equiv_name.c_str());
						out += equiv_name;
					}
					/* Otherwise get info for that particular memory type */
					else
					{
						std::string formattedAddress;
						/* This is needed to ensure proper number of leading zeros */

						printf_debug("Equiv: no\n");

						out += mem_shared->m_print_prefix;

						//These are just for formatting right now
						uv_assert_all(m_func->m_args.size() == 1);
						uv_assert_all(memArg);
						uv_assert_all(memArgShared);
						uv_assert_all(memArgShared->m_type == UV_DISASM_DATA_IMMU);
						
						printf_debug("Immediate size: %d\n", memArgShared->m_immediate_size);					
						uv_assert(memArgShared->m_immediate_size < 128);
						
						//XXX we are acess m_ui32, is this an x86 specific thing we should be careful of?
						printf_debug("imm prefix hex: <%s>, ui32: %d, %X\n", g_config->m_asm_imm_prefix_hex.c_str(), memArg->m_ui32, memArg->m_ui32);
						//FIXME: if this is a relative address, we need to compute the correct virtual address
						uv_assert_err_ret(g_uvd->m_format->formatAddress(memArg->m_ui32, formattedAddress));
						out += g_config->m_asm_imm_prefix_hex;
						out += formattedAddress;

						out += mem_shared->m_print_suffix;
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
					
					uniaryOperand = (UVDDisasmOperand *)m_func->m_args[0];
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
				
				out += resultString;
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
			
			printf_warn("Warning: unknown symbol %s, printing args\n", getShared()->m_name.c_str());
			/* At this time, all funcs have args */
			uv_assert(m_func->m_args.size());
			out += getShared()->m_name;
			for( cur_arg_index = 0; cur_arg_index < m_func->m_args.size(); ++cur_arg_index )
			{
				UVDDisasmOperand *op_arg = NULL;

				printf_debug("Print func arg\n");
				op_arg = (UVDDisasmOperand *)m_func->m_args[cur_arg_index];
				uv_assert(op_arg);
				if( UV_FAILED(op_arg->printDisassemblyOperand(out)) )
				{
					UV_ERR(rc);
					goto error;
				}
				out += ")";
			}
		}
		break;		
	}
	default:
		printf_debug("Unknown operand type: %d\n", getShared()->m_type);
		UV_ERR(rc);
		goto error;
	}

	/* Should always print something I would think */
	uv_assert_ret(!out.empty());
	//printf_debug("Buff: <%s>\n", out.c_str());
	
	rc = UV_ERR_OK;
	
error:
	return UV_DEBUG(rc);	
}

uv_err_t UVDDisasmOperand::getUI32Representation(uint32_t &i)
{
	if( getShared()->m_type != UV_DISASM_DATA_IMMU )
	{
		return UV_ERR_GENERAL;
	}
	
	//All types should just get promoted cleanly
	i = m_ui32;
	return UV_ERR_OK;
}

uv_err_t UVDDisasmOperand::getI32RepresentationAdjusted(int32_t &i)
{
	if( getShared()->m_type != UV_DISASM_DATA_IMMS )
	{
		return UV_ERR_GENERAL;
	}
	
	uint32_t varSize = getShared()->m_immediate_size;
	
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

uv_err_t UVDDisasmOperand::getUI32RepresentationAdjusted(uint32_t &i)
{
	if( getShared()->m_type != UV_DISASM_DATA_IMMU )
	{
		return UV_ERR_GENERAL;
	}
	
	//All types should just get promoted cleanly
	i = m_ui32;
	return UV_ERR_OK;
}

uv_err_t UVDDisasmOperand::getI32Representation(int32_t &i)
{
	if( getShared()->m_type != UV_DISASM_DATA_IMMS )
	{
		return UV_ERR_GENERAL;
	}
	
	uint32_t varSize = getShared()->m_immediate_size;
	
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

