/*
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
*/

#include "interpreter/uvd_python.h"
#include "uvd_config.h"
#include "uvd_language.h"
#include "uvd_opcode.h"
#include "uv_util.h"
#include <string>
#include <vector>
#include <string.h>

UVDConfig *g_config = NULL;

UVDConfigSection::UVDConfigSection()
{
	m_line = 0;
}

UVDConfigValue::UVDConfigValue()
{
	m_operand_type = 0;
	m_num_bits = 0;
	m_value = 0;
}

uv_err_t UVDConfigValue::parseType(const std::string &in_real, UVDConfigValue *out)
{
	uv_err_t rc = UV_ERR_GENERAL;
	std::string temp_name;
	std::string in;
	
	UV_ENTER();

	in = in_real;

	if( !out )
	{
		UV_ERR(rc);
		goto error;
	}
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
		unsigned int n_operand_parts = 0;
		char **operand_parts = NULL;
		
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
			printf("Unrecognized operand: %s, expected u or s, got %c\n", in.c_str(), in[0]);
			UV_ERR(rc);
			goto error;
		}
		
		/* [u or i]_[size in bits] */
		operand_parts = uv_split_core(in.c_str(), '_', &n_operand_parts, TRUE);
		if( !operand_parts )
		{
			UV_ERR(rc);
			goto error;
		}
		if( n_operand_parts < 2 )
		{
			UV_ERR(rc);
			goto error;
		}
		/* Skip over the sign letter */
		out->m_num_bits = atoi(operand_parts[0] + 1);
		if( out->m_num_bits % 8 != 0 || out->m_num_bits > 1000 )
		{
			printf_debug("Invalid operand size: %d\n", out->m_num_bits);
			UV_ERR(rc);
			goto error;
		}
		printf_debug("Operand data size: %d\n", out->m_num_bits);
	}
	/* Legal start are 0x for hex, 0 for octal, and 0-9 for dec, all which fall under 0-9 */
	else if( isdigit(in[0]) )
	{
		out->m_operand_type = UV_DISASM_DATA_OPCODE; 
		out->m_value = strtol(in.c_str(), NULL, 0);
		/* Assume for now opcodes are taken one byte at a time */
		out->m_num_bits = 8;
		if( out->m_value > 0xFF )
		{
			printf_debug("Opcodes must be byte increment\n");
			UV_ERR(rc);
			goto error;
		}
	}
	/* Some sort of modifier? */
	else if( strstr(in.c_str(), "(") )
	{
		UVDParsedFunction *func = NULL;
		unsigned int n_func_args = 0;
		unsigned int func_args_index = 0;
		char **func_args = NULL;
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
		if( !func )
		{
			UV_ERR(rc);
			goto error;
		}
		out->m_func = func;

		func_args = uv_split_core(sArgs.c_str(), ',', &n_func_args, TRUE);
		if( !func_args )
		{
			UV_ERR(rc);
			goto error;
		}
							
		for( func_args_index = 0; func_args_index < n_func_args; ++func_args_index )
		{
			std::string cur = func_args[func_args_index];
			UVDConfigValue *parsed_type = NULL;
			
			printf_debug("Current argument: %s\n", cur.c_str()); fflush(stdout);
			parsed_type = new UVDConfigValue();
			if( !parsed_type )
			{
				UV_ERR(rc);
				goto error;
			}
			if( UV_FAILED(parseType(cur, parsed_type)) )
			{
				UV_ERR(rc);
				goto error;
			} 
			printf_debug("parsed recursive type\n"); fflush(stdout);
			out->m_func->m_args.push_back(parsed_type);
			printf_debug("parsed recursive type, set\n"); fflush(stdout);
		}
		printf_debug("parsed recursive type done loop\n"); fflush(stdout);
	}
	else
	{
		printf_debug("Unrecognized operand: %s\n", in.c_str());
		UV_ERR(rc);
		goto error;
	}

	out->m_name = temp_name;
	
	rc = UV_ERR_OK;

error:
	return UV_DEBUG(rc);
}

UVDConfig::UVDConfig()
{
	//Don't default to an unsupported language	
#if defined(USING_JAVASCRIPT)
	//Default: javascript has the highest preformance
	m_configInterpreterLanguage = UVD_LANGUAGE_JAVASCRIPT;
#elif defined(USING_PYTHON)
	//Slow due to lack of working API, but works fine
	m_configInterpreterLangauge = UVD_LANGUAGE_PYTHON;
#elif defined(USING_LUA)
	//No bitwise operators...annoying
	m_configInterpreterLangauge = UVD_LANGUAGE_LUA;
#else
#error No valid interpreters
#endif
}

uv_err_t UVDConfig::readSections(const std::string config_file, std::vector<UVDConfigSection> sectionsIn)
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

uv_err_t UVDConfig::uvd_read_sections(const std::string &config_file, UVDConfigSection ***sections_in, unsigned int *n_sections_in)
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
			//Free section name
			free(lines[start_index]);
			lines[start_index] = NULL;
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
				cur_section->m_lines.push_back(lines[start_index]);
				free(lines[start_index]);
				lines[start_index] = NULL;
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
	return UV_DEBUG(rc);
}
