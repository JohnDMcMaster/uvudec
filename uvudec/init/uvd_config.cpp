/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#include "interpreter/uvd_python.h"
#include "uvd_arg_util.h"
#include "uvd_config.h"
#include "uvd_language.h"
#include "uvd_log.h"
#include "uvd_opcode.h"
#include "uvd_util.h"
#include <string>
#include <vector>
#include <string.h>

UVDConfig *g_config = NULL;

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
	unsigned int n_operand_parts = 0;
	char **operand_parts = NULL;
	uv_err_t rc = UV_ERR_GENERAL;
	
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

	rc = UV_ERR_OK;	
	
error:
	for( unsigned int i = 0; i < n_operand_parts; ++i )
	{
		free(operand_parts[i]);
	}
	free(operand_parts);

	return UV_ERR_OK;
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
		uv_assert_err_ret(parseTypeNumber(in, out));
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

		std::vector<std::string> funcArgs = split(sArgs, ',', true);
		for( std::vector<std::string>::iterator iter = funcArgs.begin(); iter != funcArgs.end(); ++iter )
		{
			std::string cur = *iter;
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
	m_argc = 0;
	m_argv = NULL;

	versionPrintPrefixThunk = NULL;
	usagePrintPostfixThunk = NULL;

	m_sDebugFile = UVD_OPTION_FILE_STDOUT;
	//m_pDebugFile = NULL;

	//FIXME: this doesn't scale well.  Make this more generic
	//At the very least, these defaults should be selected in the Makefile/configure proces
	//since this should be selectable by that anyway
	//Don't default to an unsupported language	
#if defined(USING_JAVASCRIPT)
	//Default: javascript has the highest preformance
	m_configInterpreterLanguage = UVD_LANGUAGE_JAVASCRIPT;
#if defined(USING_JAVASCRIPT_API)
	m_configInterpreterLanguageInterface = UVD_LANGUAGE_INTERFACE_API;
#elif defined(USING_JAVASCRIPT_EXEC)
	m_configInterpreterLanguageInterface = UVD_LANGUAGE_INTERFACE_EXEC;
#else
#error 'Bad'
#endif
#elif defined(USING_PYTHON)
	//Slow due to lack of working API, but works fine
	m_configInterpreterLanguage = UVD_LANGUAGE_PYTHON;
#if defined(USING_PYTHON_API)
	m_configInterpreterLanguageInterface = UVD_LANGUAGE_INTERFACE_API;
#elif defined(USING_PYTHON_EXEC)
	m_configInterpreterLanguageInterface = UVD_LANGUAGE_INTERFACE_EXEC;
#else
#error 'Bad'
#endif
#elif defined(USING_LUA)
	//No bitwise operators...annoying
	m_configInterpreterLangauge = UVD_LANGUAGE_LUA;
#if defined(USING_LUA_API)
	m_configInterpreterLanguageInterface = UVD_LANGUAGE_INTERFACE_API;
#elif defined(USING_LUA_EXEC)
	m_configInterpreterLanguageInterface = UVD_LANGUAGE_INTERFACE_EXEC;
#else
#error 'Bad'
#endif
#else
#error No valid interpreters
#endif

	m_analysisOnly = false;
	m_uselessASCIIArt = false;
	m_flowAnalysisTechnique = UVD__FLOW_ANALYSIS__LINEAR;

	m_printUsed = false;
	m_jumpedSources = false;
	m_jumpedCount = 0;
	m_calledSources = false;
	m_calledCount = 0;
	m_addressComment = false;
	m_addressLabel = false;

	m_verbose_level = UVD_DEBUG_NONE;
	clearVerboseAll();

	m_haltOnTruncatedInstruction = FALSE;
	m_haltOnInvalidOpcode = FALSE;

	m_hex_addr_print_width = 4;
	m_caps = false;
	m_binary = false;
	m_memoric = false;
	m_asm_instruction_info = false;
	m_print_used = false;
	m_print_string_table = false;
	m_print_block_id = false;
	m_print_header = false;
}

UVDConfig::~UVDConfig()
{
	deinit();
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

uv_err_t UVDConfig::parseMain(int argc, char *const *argv)
{
	return UV_DEBUG(parseMain(argc, argv, NULL));
}

uv_err_t UVDConfig::parseMain(int argc, char *const *argv, char *const *envp)
{
	uv_err_t processRc = UV_ERR_GENERAL;

	m_argc = argc;
	m_argv = argv;
	
	m_args = charPtrArrayToVector(m_argv, m_argc);

	//Parse the data
	processRc = UVDArgConfig::process(m_configArgs, m_args);
	uv_assert_err_ret(processRc);
	if( processRc == UV_ERR_DONE )
	{
		return processRc;
	}

	//And then initialize as needed
	uv_assert_err_ret(processParseMain());

	return UV_ERR_OK;
}

uv_err_t UVDConfig::parseUserConfig()
{
	//TODO: add support using libconfig
	return UV_ERR_OK;
}

//disasm->m_noncodingAddresses = g_noncodingAddresses;
//std::vector<UVDMemoryLocation> g_noncodingAddresses;

uv_err_t UVDConfig::processParseMain()
{
	//Make sure we are logging to correct target now that we have parsed args
	uv_assert_err_ret(uv_log_init(m_sDebugFile.c_str()));
	
	printf_debug("m_verbose_args: %d, m_verbose_init: %d, m_verbose_processing: %d, m_verbose_analysis: %d, m_verbose_printing: %d\n",
			m_verbose_args, m_verbose_init, m_verbose_processing, m_verbose_analysis, m_verbose_printing);

	return UV_ERR_OK;
}

uv_err_t UVDConfig::init()
{
	//By default assume all addresses are potential analysis areas
	m_addressRangeValidity.m_default = UVD_ADDRESS_ANALYSIS_INCLUDE;
	
	uv_assert_err_ret(m_symbols.init());
	
	//Load user defined defaults
	uv_assert_err_ret(parseUserConfig());
	
	return UV_ERR_OK;
}

uv_err_t UVDConfig::deinit()
{
	for( std::vector<UVDArgConfig *>::iterator iter = m_configArgs.begin(); iter != m_configArgs.end(); ++iter )
	{
		delete *iter;
	}
	m_configArgs.clear();
	
	return UV_ERR_OK;
}

uv_err_t UVDConfig::addAddressInclusion(uint32_t low, uint32_t high)
{
	//If the first check we do is an inclusion, assume by default to exclude
	if( m_addressRangeValidity.empty() )
	{
		m_addressRangeValidity.m_default = UVD_ADDRESS_ANALYSIS_EXCLUDE;
	}
	m_addressRangeValidity.add(low, high, UVD_ADDRESS_ANALYSIS_INCLUDE);
	return UV_ERR_OK;
}

uv_err_t UVDConfig::addAddressExclusion(uint32_t low, uint32_t high)
{
	//If the first check we do is an exclusion, assume by default to include
	if( m_addressRangeValidity.empty() )
	{
		m_addressRangeValidity.m_default = UVD_ADDRESS_ANALYSIS_INCLUDE;
	}
	m_addressRangeValidity.add(low, high, UVD_ADDRESS_ANALYSIS_EXCLUDE);
	return UV_ERR_OK;
}

uv_err_t UVDConfig::getValidAddressRanges(std::vector<UVDRangePair> &ranges)
{
	uv_err_t rc = UV_ERR_GENERAL;
	UVDRangePair cur;

	ranges.clear();
	
	//Seed it
	rc = nextValidAddress(0, &cur.m_min);
	uv_assert_err_ret(rc);
	//No valid ranges?
	if( rc == UV_ERR_DONE )
	{
		return UV_ERR_OK;
	}
	
	//Otherwise we are seeded and ready to churn out ranges
	for( ;; )
	{
		//Find the end range
		rc = nextInvalidAddress(cur.m_min, &cur.m_max);
		uv_assert_err_ret(rc);
		//Fill max range if no more and we are on a valid range till end
		if( rc == UV_ERR_DONE )
		{
			cur.m_max = UVD_ADDR_MAX;
		}
		else
		{
			//The address before the next invalid is the highest valid address
			--cur.m_max;
		}
		//Add the range
		ranges.push_back(cur);

		//Done if we just processed the last valid range
		if( rc == UV_ERR_DONE )
		{
			break;
		}
		
		//Shift
		//We are garaunteed at least one more address since we are not at end (indicated by UV_ERR_DONE)
		rc = nextValidAddress(cur.m_max + 1, &cur.m_min);
		uv_assert_err_ret(rc);
		//No more valid addresses?
		if( rc == UV_ERR_DONE )
		{
			break;
		}
	}
	
	return UV_ERR_OK;
}

uv_err_t UVDConfig::getAddressMin(uint32_t *addr)
{
	uv_err_t rc = UV_ERR_GENERAL;
	
	//Get the lowest possible valid address
	rc = nextValidAddress(0, addr);
	uv_assert_err_ret(rc);
	uv_assert_ret(rc != UV_ERR_DONE);
	
	return UV_ERR_OK;
}

uv_err_t UVDConfig::getAddressMax(uint32_t *addr)
{
	uv_err_t rc = UV_ERR_GENERAL;
	
	//Get the highest possible valid address
	rc = lastValidAddress(UINT_MAX, addr);
	uv_assert_err_ret(rc);
	uv_assert_ret(rc != UV_ERR_DONE);
	
	return UV_ERR_OK;
}

uv_err_t UVDConfig::nextValidAddress(uint32_t start, uint32_t *ret)
{
	return UV_DEBUG(nextAddressState(start, ret, UVD_ADDRESS_ANALYSIS_INCLUDE));
}

uv_err_t UVDConfig::nextInvalidAddress(uint32_t start, uint32_t *ret)
{
	return UV_DEBUG(nextAddressState(start, ret, UVD_ADDRESS_ANALYSIS_EXCLUDE));
}

uv_err_t UVDConfig::lastValidAddress(uint32_t start, uint32_t *ret)
{
	return UV_DEBUG(lastAddressState(start, ret, UVD_ADDRESS_ANALYSIS_INCLUDE));
}

uv_err_t UVDConfig::lastInvalidAddress(uint32_t start, uint32_t *ret)
{
	return UV_DEBUG(lastAddressState(start, ret, UVD_ADDRESS_ANALYSIS_EXCLUDE));
}

uv_err_t UVDConfig::nextAddressState(uint32_t start, uint32_t *ret, uint32_t targetState)
{
	//Given is a valid canidate
	uint32_t next = start;
	//printf("nextAddressState() start at 0x%.8X\n", start);
	
	//Each time we invalidate the address, re-iterate over the list to see if its stable
	for( ;; )
	{
		uint32_t state = UVD_ADDRESS_ANALYSIS_UNKNOWN;
		UVDUint32RangePriorityList::iterator iter;
		uint32_t nextStart = next;

		//See if we get a better match
		for( iter = m_addressRangeValidity.begin(); iter != m_addressRangeValidity.end(); ++iter )
		{
			if( (*iter).matches(next) )
			{
				state = (*iter).m_matchState;
				break;
			}

			if( (*iter).m_t.m_min > start && (nextStart == start || (*iter).m_t.m_min < nextStart) )
			{
				nextStart = (*iter).m_t.m_min;
			}
		}
		
		//Nothing more can be matched
		if( state == UVD_ADDRESS_ANALYSIS_UNKNOWN )
		{
			//If the default matches our state, we are done
			if( m_addressRangeValidity.m_default == targetState )
			{
				break;
			}
			//Otherwise, is there a valid range we could jump to?
			else if( nextStart != next )
			{
				next = nextStart;
			}
			//If no more range canidates, there are no possible valid locations left
			else
			{
				return UV_ERR_DONE;
			}
		}
		//Are we at a valid address?  We are done
		else if( state == targetState )
		{
			break;
		}
		//A non-matching state, keep going if possible
		//Exclusion?  Advance to next possible canidate then
		//else if( state == UVD_ADDRESS_ANALYSIS_EXCLUDE )
		else
		{
			//Advance to one beyond the end of the exclusion range
			//...But only if its a valid address
			if( (*iter).m_t.m_max == UINT_MAX )
			{
				//There are no valid addresses
				return UV_ERR_DONE;
			}
			//Otherwise advance to the next canidate
			next = (*iter).m_t.m_max + 1;
		}
	}
	
	//Save and ret
	uv_assert_ret(ret);
	*ret = next;
	
	return UV_ERR_OK;
}

#define printf_debug_address_state(...)

uv_err_t UVDConfig::lastAddressState(uint32_t start, uint32_t *ret, uint32_t targetState)
{
	//Given is a valid canidate
	uint32_t next = start;
	
	//Each time we invalidate the address, re-iterate over the list to see if its stable
	//Since the ACL order is arbirary, we cannot lineraize this
	for( ;; )
	{
		uint32_t state = UVD_ADDRESS_ANALYSIS_UNKNOWN;
		UVDUint32RangePriorityList::iterator iter;
		//If we need to keep decreasing space, the next availible space where it could change
		uint32_t nextStart = next;

		printf_debug_address_state("address state loop\n");

		//Avoid special cases with empty check
		if( !m_addressRangeValidity.empty() )
		{
			printf_debug_address_state("checking ACL\n");
			iter = m_addressRangeValidity.end();
			//Backtrack to a valid position
			--iter;
			//See if we get a better match
			for( ; ; --iter )
			{
				if( (*iter).matches(next) )
				{
					state = (*iter).m_matchState;
					break;
				}
				printf_debug_address_state("no iter match \n");
				
				//Prepare the next block range to check if we don't match a range
				//Valid canidate and best canidate
				if( (*iter).m_t.m_max < start && (nextStart == start || (*iter).m_t.m_max > nextStart) )
				{
					nextStart = (*iter).m_t.m_max;
				}
				
				if( iter == m_addressRangeValidity.begin() )
				{
					break;
				}
			}
		}
		
		//Nothing could be matched
		if( state == UVD_ADDRESS_ANALYSIS_UNKNOWN )
		{
			printf_debug_address_state("no match\n");
			//If the default matches our state, we are done
			if( m_addressRangeValidity.m_default == targetState )
			{
				break;
			}
			//Otherwise, is there a valid range we could jump to?
			else if( nextStart != next )
			{
				next = nextStart;
				printf_debug_address_state("Continuing at 0x%.8X\n", next);
			}
			//If no more range canidates, there are no possible valid locations left
			else
			{
				return UV_ERR_DONE;
			}
		}
		//Are we at a valid address?  We are done
		else if( state == targetState )
		{
			printf_debug_address_state("state reached\n");
			break;
		}
		//A non-matching state, keep going if possible
		//Exclusion?  Advance to next possible canidate then
		//else if( state == UVD_ADDRESS_ANALYSIS_EXCLUDE )
		else
		{
			printf_debug_address_state("non matching state\n");
			//Advance to one beyond the end of the exclusion range
			//...But only if its a valid address
			if( (*iter).m_t.m_min == 0 )
			{
				//There are no valid addresses
				return UV_ERR_DONE;
			}
			//Otherwise advance to the next canidate
			next = (*iter).m_t.m_min - 1;
		}
	}
	
	//Save and ret
	uv_assert_ret(ret);
	*ret = next;
	
	return UV_ERR_OK;
}

bool UVDConfig::anyVerboseActive()
{
	return m_verbose_args || m_verbose || m_verbose_init || m_verbose_processing || m_verbose_analysis || m_verbose_printing;
}

void UVDConfig::clearVerboseAll()
{
	m_verbose = false;
	m_verbose_args = false;
	m_verbose_init = false;
	m_verbose_processing = false;
	m_verbose_analysis = false;
	m_verbose_printing = false;
}

void UVDConfig::setVerboseAll()
{
	m_verbose = true;
	m_verbose_args = true;
	m_verbose_init = true;
	m_verbose_processing = true;
	m_verbose_analysis = true;
	m_verbose_printing = true;
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
