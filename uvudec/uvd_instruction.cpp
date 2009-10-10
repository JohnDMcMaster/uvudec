/*
Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under the terms of the BSD license.  See LICENSE for details.
*/


#include "uvd.h"
#include "uvd_instruction.h"
#include "uvd_types.h"
#include "uvd_format.h"
#include "main.h"
#include <string.h>
#include <vector>
#include <string>

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
	return UV_ERR_OK;
}

UVDInstruction::UVDInstruction()
{
	m_shared = NULL;
	m_offset = 0;
	m_inst_size = 0;
	m_uvd = NULL;
	memset(m_inst, 0, sizeof(m_inst));
}

uv_err_t UVDInstruction::print_disasm(std::string &s)
{
	uv_err_t rc = UV_ERR_GENERAL;
	char buff[512];

	UV_ENTER();

	rc = print_disasm(buff, 512);
	if( UV_SUCCEEDED(rc) )
	{
		s = buff;
	}
	
	return UV_ERR(rc);
}

/*
*/
uv_err_t UVDInstruction::print_disasm(char *buff, unsigned int buffsz)
{
	uv_err_t rc = UV_ERR_GENERAL;
	unsigned int pos = 0;
	std::string operand_pad = "";
	
	UV_ENTER();

	uv_assert(buff);
	printf_debug("inst size: %d\n", m_inst_size);
	uv_assert(m_inst_size);

	if( m_operands.size() )
	{
		operand_pad = " ";
	}
	if( g_verbose )
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
	
	if( g_asm_instruction_info )
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
				/*
				This stage is no longer recursive
				else
				{
					unsigned int last_usage = 0;
					
					printf_debug("Pre sub buff: <%s>\n", buff);
					if( UV_FAILED(print_disasm_operand(op->m_next, buff + pos, buffsz - pos, &last_usage)) )
					{
						UV_ERR(rc);
						goto error;
					}
					printf_debug("Post sub buff: <%s>\n", buff);
					pos += last_usage;
				}
				*/
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
uv_err_t UVDInstruction::parseOperands(uint32_t &nextPosition, std::vector<UVDOperandShared *> ops_shared, std::vector<UVDOperand *> &operands)
//uv_err_t uv_disasm_next_operand(struct uv_inst_operand_shared_t *op_shared, struct uv_inst_operand_t *op, const uint8_t *binary, int *offset)
{
	//UVDInstructionShared *shared = NULL;
	uv_err_t rc = UV_ERR_GENERAL;
	/* This should match the total length field */
	//UVDOperand *op_last = NULL;
	/* One byte from opcode */
	//unsigned int operand_offset = 1;
	//unsigned int i = 0;
	
	UV_ENTER();
	
	UVDData *data = NULL;;
	data = m_uvd->m_data;
	uv_assert_ret(data);
	
	//uv_assert(instruction);
	//shared = instruction->m_shared;
	//uv_assert(shared);
	//We should be initializing this, should not have been touched yet
	uv_assert(operands.empty());
	                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           
	for( unsigned int i = 0; i < ops_shared.size(); ++i )
	{
		printf_debug("Loop op\n");
		UVDOperandShared *op_shared = ops_shared[i];
		UVDOperand *op = NULL;
		
		uv_assert(op_shared);
		
		op = new UVDOperand();
		if( !op )
		{
			UV_ERR(rc);
			goto error;
		}
		op->m_instruction = this;
		
		op->m_shared = op_shared;
		
		uv_assert_err_ret(op->parseOperand(nextPosition));
		
		uv_assert(op->m_shared);
		printf_debug("Linked %s\n", op->m_shared->m_name.c_str());
		
		operands.push_back(op);
	}
	rc = UV_ERR_OK;

error:
	return UV_DEBUG(rc);
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
