/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvdasm/util.h"
#include "uvd/assembly/instruction.h"
#include "uvd/util/util.h"

UVDConfigValue::UVDConfigValue()
{
	m_operand_type = 0;
	m_num_bits = 0;
	m_value = 0;
	m_func = NULL;
	m_bitmask = 0;
	m_opcodeRangeOffset = 0;
}

UVDConfigValue::~UVDConfigValue()
{
	deinit();
}

uv_err_t UVDConfigValue::deinit()
{
	switch( m_operand_type )
	{
	case UV_DISASM_DATA_FUNC:
		delete m_func;
		m_func = NULL;
	}
	return UV_ERR_OK;
}

uv_err_t UVDConfigValue::parseTypeNumber(const std::string &in, UVDConfigValue *out)
{
	std::vector<std::string> operandParts;
	
	if( in[0] == 's' )
	{
		out->m_operand_type = UV_DISASM_DATA_IMMS;
	}
	else if( in[0] == 'u' )
	{
		out->m_operand_type = UV_DISASM_DATA_IMMU;
	}
	else
	{
		printf_error("Unrecognized operand: %s, expected u or s, got %c\n", in.c_str(), in[0]);
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	
	//[u or i]_[size in bits]
	operandParts = UVDSplit(in, '_', TRUE);
	uv_assert_ret(operandParts.size() >= 2);
	//Skip over the sign letter
	out->m_num_bits = atoi(operandParts[0].c_str() + 1);
	if( out->m_num_bits % 8 != 0 || out->m_num_bits > 1000 )
	{
		printf_debug("Invalid operand size: %d\n", out->m_num_bits);
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	printf_debug("Operand data size: %d\n", out->m_num_bits);

	return UV_ERR_OK;
}

uv_err_t UVDConfigValue::parseType(const std::string &in_real, UVDConfigValue *out)
{
	std::string temp_name;
	std::string in;
	
	in = in_real;

	uv_assert_ret(out);
	printf_debug("Parsing type of: %s\n", in.c_str());
	temp_name = in;

	if( in[0] == '%' )
	{
		/* Skip the % for registers */
		temp_name = temp_name.erase(0, 1);
		out->m_operand_type = UV_DISASM_DATA_REG;
	}
	else if( in[0] == 'u' || in[0] == 's' )
	{
		uv_assert_err_ret(parseTypeNumber(in, out));
	}
	/* Legal start are 0x for hex, 0 for octal, and 0-9 for dec, all which fall under 0-9 */
	else if( isdigit(in[0]) )
	{
		/*
		Something like 
		0xDE
		032
		232
		
		Ex for 0xD0-0xDF:
		0xD0/0xF0
		*/
		std::string byteString;
		
		out->m_operand_type = UV_DISASM_DATA_OPCODE; 

		if( in.find('/') != std::string::npos )
		{
			std::vector<std::string> parts = UVDSplit(in, '/');
			if( parts.size() != 2 )
			{
				printf_error("required form 0x12/0x34, got extra / on %s\n", in.c_str());
				return UV_DEBUG(UV_ERR_GENERAL);
			}
			byteString = parts[0];
			out->m_bitmask = strtol(parts[1].c_str(), NULL, 0);
		}
		//XXX: also support 0x10:0x1F notation
		else if( in.find('+') != std::string::npos )
		{
			std::vector<std::string> parts = UVDSplit(in, '+');
			if( parts.size() != 2 )
			{
				printf_error("required form 0x12+0x03, got extra + on %s\n", in.c_str());
				return UV_DEBUG(UV_ERR_GENERAL);
			}
			byteString = parts[0];
			out->m_opcodeRangeOffset = strtol(parts[1].c_str(), NULL, 0);
		}
		else
		{
			byteString = in;
		}
		out->m_value = strtol(byteString.c_str(), NULL, 0);
		
		/* Assume for now opcodes are taken one byte at a time */
		out->m_num_bits = 8;
		if( out->m_value > 0xFF )
		{
			printf_debug("Opcodes must be byte increment\n");
			return UV_DEBUG(UV_ERR_GENERAL);
		}
	}
	/* Some sort of modifier? */
	else if( in.find("(") != std::string::npos )
	{
		UVDParsedFunction *func = NULL;
		std::string sArgs;
		std::string sFunc;
	
		sFunc = parseSubstring(in, "", "", "(");
		sArgs = parseSubstring(in, "", "(", "");
		if( sArgs[sArgs.size() - 1] != ')' )
		{
			return UV_DEBUG(UV_ERR_GENERAL);
		}
		sArgs.erase(sArgs.size() - 1);

		printf_debug("function name: %s, args: %s\n", sFunc.c_str(), sArgs.c_str());
		//Functions are keyed to their name
		temp_name = sFunc;
		
		out->m_operand_type = UV_DISASM_DATA_FUNC;
		func = new UVDParsedFunction();
		uv_assert_ret(func);
		out->m_func = func;

		std::vector<std::string> funcArgs = split(sArgs, ',', true);
		for( std::vector<std::string>::iterator iter = funcArgs.begin(); iter != funcArgs.end(); ++iter )
		{
			std::string cur = *iter;
			UVDConfigValue *parsed_type = NULL;
			
			printf_debug("Current argument: %s\n", cur.c_str());
			parsed_type = new UVDConfigValue();
			uv_assert_ret(parsed_type);
			uv_assert_err_ret(parseType(cur, parsed_type));
			printf_debug("parsed recursive type\n");
			out->m_func->m_args.push_back(parsed_type);
			printf_debug("parsed recursive type, set\n");
		}
		printf_debug("parsed recursive type done loop\n");
	}
	else
	{
		printf_error("Unrecognized operand: %s\n", in.c_str());
		return UV_DEBUG(UV_ERR_GENERAL);
	}

	out->m_name = temp_name;
	
	return UV_ERR_OK;
}

/*
UVDParsedFunction
*/

UVDParsedFunction::UVDParsedFunction()
{
}

UVDParsedFunction::~UVDParsedFunction()
{
	deinit();
}

uv_err_t UVDParsedFunction::deinit()
{
	for( std::vector<UVDConfigValue *>::iterator iter = m_args.begin(); iter != m_args.end(); ++iter )
	{
		delete *iter;
	}
	m_args.clear();
	
	return UV_ERR_OK;
}

