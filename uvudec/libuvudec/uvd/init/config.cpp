/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/init/arg_util.h"
#include "uvd/init/arg_property.h"
#include "uvd/init/config.h"
#include "uvd/util/debug.h"
#include "uvd/language/language.h"
#include "uvd/util/log.h"
#include "uvd/util/util.h"
#include "uvd/core/analysis.h"
#include <string>
#include <vector>
#include <string.h>

UVDConfig *g_config = NULL;

UVDConfig::UVDConfig()
{
	if( g_config )
	{
		printf_error("config alread initialized!\n");
		//UVD_PRINT_STACK();
	}
	m_argc = 0;
	m_argv = NULL;

	versionPrintPrefixThunk = NULL;
	usagePrintPostfixThunk = NULL;

	m_targetFileName = DEFAULT_DECOMPILE_FILE;
	m_sDebugFile = UVD_OPTION_FILE_STDOUT;
	//m_pDebugFile = NULL;

	m_analysisOnly = false;
	m_uselessASCIIArt = false;
	m_flowAnalysisTechnique = UVD__FLOW_ANALYSIS__LINEAR;

	m_rawFileSuffix = "_raw.bin";
	m_relocatableFileSuffix = "_rel.bin";
	m_elfFileSuffix = ".elf";

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


	m_computeFunctionMD5 = true;
	m_computeFunctionRelocatableMD5 = true;

	m_writeRawBinary = true;
	m_writeRelocatableBinary = true;
	m_writeElfFile = true;
	m_functionIndexFilename = "index.func";
}

UVDConfig::~UVDConfig()
{
	deinit();
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
	
	//Okay, now that we have base plugin search paths setup, we can initialize plugin engine
	uv_assert_err_ret(m_plugin.init(this));

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
	UVDSetDebugFlag(UVD_DEBUG_TYPE_ALL, false);
}

void UVDConfig::setVerboseAll()
{
	m_verbose = true;
	m_verbose_args = true;
	m_verbose_init = true;
	m_verbose_processing = true;
	m_verbose_analysis = true;
	m_verbose_printing = true;
	UVDSetDebugFlag(UVD_DEBUG_TYPE_ALL, true);
}

uv_err_t UVDConfig::registerDefaultArgument(UVDArgConfigHandler handler,
		const std::string &helpMessage,
		uint32_t minRequired,
		bool combine,
		bool alwaysCall)
{
	UVDArgConfig *argConfig = NULL;

	argConfig = new UVDArgConfig(handler, helpMessage, minRequired, combine, alwaysCall);
	uv_assert_ret(argConfig);
	m_configArgs.push_back(argConfig);

	return UV_ERR_OK;
}

uv_err_t UVDConfig::registerArgument(const std::string &propertyForm,
		char shortForm, std::string longForm, 
		std::string helpMessage,
		uint32_t numberExpectedValues,
		UVDArgConfigHandler handler,
		bool hasDefault,
		const std::string &plugin)
{
	return UV_DEBUG(registerArgument(propertyForm,
			shortForm, longForm, 
			helpMessage,
			"",
			numberExpectedValues,
			handler,
			hasDefault,
			plugin));
}
		
uv_err_t UVDConfig::registerArgument(const std::string &propertyForm,
		char shortForm, std::string longForm, 
		std::string helpMessage,
		std::string helpMessageExtra,
		uint32_t numberExpectedValues,
		UVDArgConfigHandler handler,
		bool hasDefault,
		const std::string &plugin)
{
	UVDArgConfig *argConfig = NULL;
		
	/*
	FIXME: migrate all argument instantiation to these funcs and make UVDArgConfig have a single constructor
	*/
	if( helpMessageExtra.empty() )
	{
		argConfig = new UVDArgConfig(propertyForm, shortForm, longForm, helpMessage, numberExpectedValues, handler, hasDefault);
	}
	else
	{
		argConfig = new UVDArgConfig(propertyForm, shortForm, longForm, helpMessage, helpMessageExtra, numberExpectedValues, handler, hasDefault);
	}
	uv_assert_ret(argConfig);
	g_config->m_configArgs.push_back(argConfig);

	if( !plugin.empty() )
	{
		g_config->m_plugin.m_pluginEngine.m_pluginArgMap[argConfig] = plugin;
	}

	return UV_ERR_OK;
}

/*
UVDPluginConfig
*/

UVDPluginConfig::UVDPluginConfig()
{
}

UVDPluginConfig::~UVDPluginConfig()
{
	UV_DEBUG(deinit());
}

uv_err_t UVDPluginConfig::init(UVDConfig *config)
{
	//FIXME: hack until I can have startup config files
	m_dirs.push_back("../lib/plugin");
	m_toLoad.push_back("uvdasm");
	m_toLoad.push_back("uvdobjbin");
	//This must be done early since command line options depend upon which plugins are loaded
	uv_assert_err_ret(m_pluginEngine.init(g_config));

	return UV_ERR_OK;
}

uv_err_t UVDPluginConfig::deinit()
{
	return UV_ERR_OK;
}

uv_err_t UVDPluginConfig::addPlugin(const std::string &pluginLibraryName)
{
	m_toLoad.push_back(pluginLibraryName);
	return UV_ERR_OK;
}

uv_err_t UVDPluginConfig::appendPluginPath(const std::string &path)
{
	m_dirs.insert(m_dirs.end(), path);
	return UV_ERR_OK;
}

uv_err_t UVDPluginConfig::prependPluginPath(const std::string &path)
{
	m_dirs.insert(m_dirs.begin(), path);
	return UV_ERR_OK;
}

//Called before debugging initialized
uv_err_t UVDInitConfigEarly()
{
	g_config = new UVDConfig();
	uv_assert_ret(g_config);
	uv_assert_err_ret(g_config->init());

	return UV_ERR_OK;
}

//Called after debugging initialized
uv_err_t UVDInitConfig()
{
	//libuvudec shared config
	uv_assert_err_ret(UVDInitArgConfig());
	//Program specific config
	//uv_assert_err_ret(initProgConfig());

	return UV_ERR_OK;
}

UVDConfig *UVDGetConfig()
{
	return g_config;
}

