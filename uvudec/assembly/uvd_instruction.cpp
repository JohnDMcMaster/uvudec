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
#include "uvd_util.h"
#include "main.h"
#include <vector>
#include <string>
#include <stdio.h>
#include <string.h>

UVDInstructionShared::UVDInstructionShared()
{
	m_opcode_length = 0;
	memset(m_opcode, 0, sizeof(m_opcode));
	m_total_length = 0;
	m_opcode_range_offset = 0;
	m_cpi = 0;
	m_cpi_low = 0;
	m_cpi_hi = 0;
	m_inst_class = UVD_INSTRUCTION_CLASS_UNKNOWN;
	m_config_line_syntax = 0;
	m_config_line_usage = 0;
	m_isImmediateOnlyFunction = UV_ERR_GENERAL;
}

UVDInstructionShared::~UVDInstructionShared()
{
	deinit();
}

uv_err_t UVDInstructionShared::deinit()
{
	for( std::vector<UVDOperandShared *>::iterator iter = m_operands.begin(); iter != m_operands.end(); ++iter )
	{
		delete *iter;
	}
	m_operands.clear();
	return UV_ERR_OK;
}

std::string UVDInstructionShared::getHumanReadableUsage()
{
	std::string sRet;

	sRet += m_memoric;

	for( unsigned int i = 0; i < m_operands.size(); ++i )
	{	
		UVDOperandShared *op = m_operands[i];
		if( !op )
		{
			sRet += "<ERROR>";
		}
		else
		{
			sRet += " ";
			sRet += op->m_name;
		}
		
		if( i + 1 < m_operands.size() )
		{
			sRet += ",";
		}
	}
	
	return sRet;
}

uv_err_t UVDInstructionShared::analyzeAction()
{
	if( m_action.find("CALL") != std::string::npos )
	{
		m_inst_class = UVD_INSTRUCTION_CLASS_CALL;
	}
	else if( m_action.find("GOTO") != std::string::npos )
	{
		m_inst_class = UVD_INSTRUCTION_CLASS_JUMP;
	}
	else
	{
		m_inst_class = UVD_INSTRUCTION_CLASS_UNKNOWN;
	}
	
	m_isImmediateOnlyFunction = isImmediateOnlyFunctionCore();
	
	return UV_ERR_OK;
}

uv_err_t UVDInstructionShared::isImmediateOnlyFunction()
{
	/*
	FIXME:
	Make this cached or something
	There is no reason we should recompute this every time
	Ideal situation is should be computed when action is set
	
	Should be suitable to do during analyzeAction
	*/
	
	return m_isImmediateOnlyFunction;
}

uv_err_t UVDInstructionShared::getImmediateOnlyFunctionAttributes(/*std::string &func,
		std::string &identifier,*/ uint32_t *identifierSizeBitsOut, uint32_t *immediateOffsetOut)
{
	uint32_t identifierSizeBits = 0;
	uint32_t immediateOffset = 0;
	std::string func;
	std::string identifier;
			

	//printf("m_action: %s\n", m_action.c_str());
	uv_assert_err_ret(isImmediateOnlyFunction());
	
	//Split it up a bit	
	//If this fails it doesn't meet the criteria
	uv_assert_err_ret(parseFunc(m_action, func, identifier));

	//identifiers type is encoded as a prefix
	//<var> := <type>_<name>
	//<type> := <sign char><size in bits>
	//u8_0
	if( identifier.find("8") != std::string::npos )
	{
		identifierSizeBits = 8;
	}
	else if( identifier.find("16") != std::string::npos )
	{
		identifierSizeBits = 16;
	}
	else if( identifier.find("32") != std::string::npos )
	{
		identifierSizeBits = 32;
	}
	else
	{
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	
	//If we have a single immediate, its offset should be the size of the opcode
	immediateOffset = m_opcode_length;
	
	uv_assert_ret(identifierSizeBitsOut);
	*identifierSizeBitsOut = identifierSizeBits;
	
	uv_assert_ret(immediateOffsetOut);
	*immediateOffsetOut = immediateOffset;

	return UV_ERR_OK;
}

uv_err_t UVDInstructionShared::isImmediateOnlyFunctionCore()
{
	std::string name;
	std::string content;
	
	//Split it up a bit	
	//If this fails it doesn't meet the criteria
	if( UV_FAILED(parseFunc(m_action, name, content)) )
	{
		return UV_ERR_GENERAL;
	}

	//Simple, only immediate
	//ACTION=CALL(u16_0)
	if( UV_SUCCEEDED(isConfigIdentifier(content)) )
	{
		return UV_ERR_OK;
	}
	//Complex, some ugly expression
	//ACTION=CALL(%PC&0x1F00+u8_0+0x6000)
	else
	{
		return UV_ERR_GENERAL;
	}
}

/*
UVDInstruction
*/

UVDInstruction::UVDInstruction()
{
	m_shared = NULL;
	m_offset = 0;
	m_inst_size = 0;
	m_uvd = NULL;
	memset(m_inst, 0, sizeof(m_inst));
}

UVDInstruction::~UVDInstruction()
{
	deinit();
}

uv_err_t UVDInstruction::deinit()
{
	//m_shared is not owned by this as there can be n to 1, as is obviously not m_uvd
	for( std::vector<UVDOperand *>::iterator iter = m_operands.begin(); iter != m_operands.end(); ++iter )
	{
		delete *iter;
	}
	m_operands.clear();

	return UV_ERR_OK;
}

uv_err_t UVDInstruction::print_disasm(std::string &s)
{
	uv_err_t rc = UV_ERR_GENERAL;
	char buff[512];


	rc = print_disasm(buff, 512);
	if( UV_SUCCEEDED(rc) )
	{
		s = buff;
	}
	
	return UV_ERR(rc);
}

uv_err_t UVDInstruction::print_disasm(char *buff, unsigned int buffsz)
{
	uv_err_t rc = UV_ERR_GENERAL;
	unsigned int pos = 0;
	std::string operand_pad = "";
	UVDConfig *config = g_config;
	
	uv_assert_ret(config);
	

	uv_assert(buff);
	printf_debug("inst size: %d\n", m_inst_size);
	uv_assert(m_inst_size);

	if( m_operands.size() )
	{
		operand_pad = " ";
	}
	if( config->m_verbose )
	{
		printf_debug("offset: 0x%.4X\n", m_offset);
	}
	/*
	if( g_verbose || g_binary )
	{
		unsigned int i = 0;
		
		for( i = 0; i < m_inst_size; ++i )
		{
			printf("\tinst[%d]: 0x%.2X\n", i, ((unsigned int)m_inst[i]) & 0xFF);
		}
	}
	*/

	printf_debug("beginning print\n");
	
	if( config->m_asm_instruction_info )
	{
		pos = snprintf(buff, buffsz, "%s (0x%.2X/%s)%s", m_shared->m_memoric.c_str(), ((unsigned int)m_inst[0]) & 0xFF, m_shared->m_desc.c_str(), operand_pad.c_str());
	}
	else
	{
		pos = snprintf(buff, buffsz, "%s%s", m_shared->m_memoric.c_str(), operand_pad.c_str());
	}
	if( !m_operands.empty() )
	{
		printf_debug("Has operand\n");
		printf_debug("Pre operand buff: <%s>\n", buff);
		for( unsigned int i = 0; i < m_operands.size(); ++i )
		{
			UVDOperand *operand = m_operands[i];
			unsigned int used = 0;

			if( UV_FAILED(operand->print_disasm_operand(buff + pos, buffsz - pos, &used)) )
			{
				printf_debug("Buff so far: %s\n", buff);
				UV_ERR(rc);
				goto error;
			}
			pos += used;
			//Out of buffer?
			if( pos >= buffsz )
			{
				printf_debug("WARNING: buffer out of room\n");
				break;
			}

			if( i + 1 < m_operands.size() )
			{
				/* note that strncpy would return dest pointer, not num copied */
				pos += snprintf(buff + pos, buffsz - pos, "%s", PRINT_OPERAND_SEPERATOR);
				if( buffsz <= pos )
				{
					printf_debug("WARNING: buffer out of room\n");
					UV_ERR(rc);
					break;
				}
			}

		}
	}
	else
	{
		printf_debug("No operand\n");
	}
	rc = UV_ERR_OK;

error:
	return UV_DEBUG(rc);
}

//Given an identified instruction operand, parse the next operand out of the remaining binary (data)
uv_err_t UVDInstruction::parseOperands(UVDIteratorCommon *uvdIter,
		std::vector<UVDOperandShared *> ops_shared, std::vector<UVDOperand *> &operands)
{
	UVDData *data = NULL;;
	uv_assert_ret(m_uvd);
	data = m_uvd->getData();
	uv_assert_ret(data);
	
	//uv_assert(instruction);
	//shared = instruction->m_shared;
	//uv_assert(shared);
	//We should be initializing this, should not have been touched yet
	uv_assert_ret(operands.empty());
	                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           
	for( unsigned int i = 0; i < ops_shared.size(); ++i )
	{
		printf_debug("Loop op\n");
		UVDOperandShared *op_shared = ops_shared[i];
		UVDOperand *op = NULL;
		uv_err_t rcParse = UV_ERR_GENERAL;
		
		uv_assert_ret(op_shared);
		
		op = new UVDOperand();
		uv_assert_ret(op);
		op->m_instruction = this;
		
		op->m_shared = op_shared;
		
		rcParse = op->parseOperand(uvdIter);
		uv_assert_err_ret(rcParse);
		//Truncated analysis?
		if( rcParse == UV_ERR_DONE )
		{
			return UV_ERR_DONE;
		}
		
		uv_assert_ret(op->m_shared);
		printf_debug("Linked %s\n", op->m_shared->m_name.c_str());
		
		operands.push_back(op);
	}
	
	return UV_ERR_OK;
}

uv_err_t UVDInstruction::collectVariables(UVDVariableMap &environment)
{
	uv_err_t rc = UV_ERR_GENERAL;

	environment.clear();
	
	for( std::vector<UVDOperand *>::size_type i = 0; i < m_operands.size(); ++i )
	{
		UVDOperand *operand = m_operands[i];
		std::string sKey;
		UVDVarient vValue;
		
		uv_assert(operand);
	
		uv_assert_err(operand->getVariable(sKey, vValue));
		if( !sKey.empty() )
		{
			environment[sKey] = vValue;
		}
	}
	
	rc = UV_ERR_OK;	

error:
	return UV_DEBUG(rc);
}
