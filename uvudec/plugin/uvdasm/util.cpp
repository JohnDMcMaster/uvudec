/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvdasm/util.h"
#include "uvd/assembly/instruction.h"
#include "uvd/util/util.h"

UVDConfigSection::UVDConfigSection()
{
	m_line = 0;
}

UVDConfigSection::~UVDConfigSection()
{
}

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
		printf_debug("Unrecognized operand: %s\n", in.c_str());
		return UV_DEBUG(UV_ERR_GENERAL);
	}

	out->m_name = temp_name;
	
	return UV_ERR_OK;
}

uv_err_t UVDAsmUtil::readSections(const std::string config_file, std::vector<UVDConfigSection> sectionsIn)
{
	UVDConfigSection **sections = NULL;
	uv_assert_err_ret(uvd_read_sections(config_file, &sections, NULL));
	uv_assert_ret(sections)
	while( *sections )
	{
		sectionsIn.push_back(**sections);
		free(*sections);
		*sections = NULL;
		++sections;
	}

	return UV_ERR_OK;
}

uv_err_t UVDAsmUtil::uvd_read_sections(const std::string &config_file, UVDConfigSection ***sections_in, unsigned int *n_sections_in)
{
	uv_err_t rc = UV_ERR_GENERAL;
	char *config_file_data = NULL;
	char **lines = NULL;
	unsigned int n_lines = 0;
	unsigned int i = 0;
	UVDConfigSection **sections = NULL;
	unsigned int n_sections = 0;
	unsigned int start_index = 0;
	unsigned int section_index = 0;
	
	printf_debug("Reading file...\n");
	if( UV_FAILED(read_filea(config_file.c_str(), &config_file_data)) )
	{
		goto error;
		UV_ERR(rc);
	}
	
	/* Find out how many sections we got */
	lines = uv_split_lines(config_file_data, &n_lines);
	if( !lines )
	{
		UV_ERR(rc);
		goto error;
	}
	/*
	if( lines[0][0] != '.' )
	{
		printf_debug("File must start with a section\n");
		UV_ERR(rc);
		goto error;
	}
	*/
	
	/* Count number of section */
	for( i = 0; i < n_lines; ++i )
	{
		if( lines[i][0] == '.' )
		{
			++n_sections;
		}
	}
	sections = (UVDConfigSection **)malloc(sizeof(UVDConfigSection *) * n_sections);
	if( !sections )
	{
		UV_ERR(rc);
		goto error;
	}
	
	for( i = 0; i < n_lines; ++i )
	{
		/* Trigger on falling edges of sections */
		if( lines[i][0] == '.' || i == n_lines - 1 )
		{
			UVDConfigSection *cur_section = NULL;
			unsigned int nLines = 0;

			/* Initialize where the section starts */
			if( start_index == 0 )
			{
				start_index = i;
				continue;
			}

			cur_section = new UVDConfigSection();
			if( !cur_section )
			{
				UV_ERR(rc);
				goto error;
			}
			cur_section->m_line = start_index;
			
			/* Skip the . */
			cur_section->m_name = lines[start_index] + 1;
			printf_debug("Reading section: %s\n", cur_section->m_name.c_str());
			printf_debug("Start: %d, end: %d\n", start_index, i);
			++start_index;
			/* i is one greater than the range we want */
			/*
			cur_section->m_n_lines = i - start_index;
			cur_section->m_lines = (std::string *)malloc(sizeof(std::string ) * cur_section->m_n_lines);
			if( !cur_section->m_lines )
			{
				UV_ERR(rc);
				goto error;
			}
			*/
			/* Copy lines */
			nLines = (unsigned int)(i - start_index);
			printf_debug("Copying lines: %d\n", nLines);
			for( unsigned int j = 0; j < nLines; ++j )
			//while( start_index < i )
			{
				std::string s = lines[start_index];
				cur_section->m_lines.push_back(s);
				++start_index;
			}
			start_index = i;
			sections[section_index] = cur_section;
			++section_index;
			printf_debug("Section read\n");
		}
	}
	
	*sections_in = sections;
	*n_sections_in = n_sections;
	rc = UV_ERR_OK;

error:
	free(config_file_data);

	for( unsigned int i = 0; i < n_lines; ++i )
	{
		free(lines[i]);
	}
	free(lines);

	return UV_DEBUG(rc);
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

