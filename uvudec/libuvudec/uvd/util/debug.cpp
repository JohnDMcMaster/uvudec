/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/config/arg_util.h"
#include "uvd/config/config.h"
#include "uvd/util/debug.h"
#include "uvd/util/log.h"
#include "uvd/util/util.h"
#include "uvd/compiler/gcc.h"
#include <signal.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#ifdef __GNUC__
#include <cxxabi.h>
#include <execinfo.h>
#endif

static const char *g_last_func = NULL;
uint32_t g_debugTypeFlags = UVD_DEBUG_TYPE_NONE;

static void uvd_signal_handler(int sig)
{
	const char *sig_str = "UNKNOWN";
	switch( sig )
	{
	case SIGSEGV:
		sig_str = "SIGSEGV";
		break;
	case SIGFPE:
		sig_str = "SIGFPE";
		break;
	}
	
	printf("\n\nSEVERE ERROR\n");
	/*
	i before e, except after c... 
	...and you'll never be right, no matter what you say!
	*/
	printf("Received signal: %s\n", sig_str);
	//Yes, the stack is still intact
	UVD_PRINT_STACK();
	//exit() is not a "safe" function.  See man signal
	_exit(1);
}

static bool shouldPrintType(uint32_t type)
{
	//Any flags should be set?
	//Or is it more useful to do all?
	//usually we will have just one
	return type & g_debugTypeFlags;
}

uv_err_t UVDSetDebugFlag(uint32_t flag, uint32_t shouldSet)
{
	if( shouldSet )
	{
		g_debugTypeFlags |= flag;
	}
	else
	{
		g_debugTypeFlags &= ~flag;
	}
	return UV_ERR_OK;
}

bool UVDGetDebugFlag(uint32_t flag)
{
	return g_debugTypeFlags & flag;
}

bool UVDAnyDebugActive()
{
	return g_debugTypeFlags != 0;
}

static void printf_debug_prefix(FILE *fileHandle, const char *prefix, const char *file, uint32_t line, const char *func)
{
	if( !fileHandle )
	{
		fileHandle = stdout;
	}
	if( prefix == NULL )
	{
		prefix = "";
	}
	fprintf(fileHandle, "DEBUG %s(%s:%d): %s", file, func, line, prefix);
}

void printf_debug_core(uint32_t level, uint32_t type, const char *file, uint32_t line, const char *func, const char *format, ...)
{
	FILE *fileHandle = g_log_handle;
	va_list ap;
	uint32_t verbose = 0;
	uint32_t set_level = 0;

	if( shouldPrintType(type) )
	{
		std::string typePrefix;
		
		//Keep logging before g_config initialized
		if( !fileHandle )
		{
			fileHandle = stdout;
		}
		if( g_config )
		{
			set_level = g_config->m_debugLevel;
		}
		else
		{
			verbose = 0;
			set_level = 0;
		}
	
		//Is logging disabled or are we at too high of a level?
		if( level > set_level )
		{
			return;
		}
		
		if( g_config && g_config->m_modulePrefixes.find(type) != g_config->m_modulePrefixes.end() )
		{
			typePrefix = g_config->m_modulePrefixes[type];
			if( !typePrefix.empty() )
			{
				typePrefix += ": ";
			}
		}
		printf_debug_prefix(fileHandle, typePrefix.c_str(), file, line, func);

		va_start(ap, format);
		vfprintf(fileHandle, format, ap);
		fflush(fileHandle);
		va_end(ap);
	}
}

void uv_enter(const char *file, uint32_t line, const char *func)
{
	printf_debug_level(UVD_DEBUG_VERBOSE, "ENTER: %s:%d::%s\n", file, line, func);
	g_last_func = func;
}

const char *get_last_func()
{
	return g_last_func;
}

uv_err_t UVDDebugInit()
{
	//Initially we log to console until a "real" log is setup which may be an actual file
	//we don't know actual file because we haven't parsed args yet
	uv_assert_err_ret(uv_log_init("/dev/stdout"));

	g_debugTypeFlags = UVD_DEBUG_TYPE_NONE;
	signal(SIGSEGV, uvd_signal_handler);
	signal(SIGFPE, uvd_signal_handler);
	return UV_ERR_OK;
}

uv_err_t UVDDebugDeinit()
{
	uv_assert_err_ret(uv_log_deinit());
	return UV_ERR_OK;
}

#ifdef __GNUC__
void uvd_print_trace(const char *file, int line, const char *function)
{
	/*
	Based on code from http://tombarta.wordpress.com/2008/08/01/c-stack-traces-with-gcc/

	Eventually I'd like to write my own debugger and will be able to read this info directly so I can get line nums and such
	Until then, hackish parsing

	Example lines
		libuvudec.so(_ZN28UVDFLIRTSignatureRawSequence14const_iteratordeEv+0x89) [0xaa9531]
		./uvpat2sig.dynamic() [0x8049de5]
		/lib/libc.so.6(__libc_start_main+0xe6) [0x5eccc6]

	*/
	const size_t max_depth = 100;
	size_t stack_depth;
	void *stack_addrs[max_depth];
	char **stack_strings;

	stack_depth = backtrace(stack_addrs, max_depth);
	stack_strings = backtrace_symbols(stack_addrs, stack_depth);

	printf("Call stack from %s:%d:\n", file, line);

	for (size_t i = 1; i < stack_depth; i++)
	{
		std::string curStack = stack_strings[i];
		std::vector<std::string> lineParts = split(curStack, ' ');
		
		if( lineParts.size() == 2 )
		{
			std::string funcPart = lineParts[0];
			std::string module;
			std::string moduleOffset;
			
			if( UV_SUCCEEDED(parseFunc(funcPart, module, moduleOffset)) )
			{
				std::vector<std::string> moduleOffsetParts = split(moduleOffset, '+');
				
				if( moduleOffsetParts.size() == 2 )
				{
					UVDCompilerGCC gccCompiler;
					std::string functionNameRaw = moduleOffsetParts[0];
					std::string functionNameDemangled;
					std::string functionOffsetRaw = moduleOffsetParts[1];
				
					if( UV_SUCCEEDED(gccCompiler.demangleByABI(functionNameRaw, functionNameDemangled)) )
					{
						//Mimic the rest of the stack
						printf("    %s(%s+%s) [%08x]\n", module.c_str(), functionNameDemangled.c_str(), functionOffsetRaw.c_str(), (int)stack_addrs[i]);					
						continue;
					}
				}
			}
		}
		printf("    %s\n", curStack.c_str());
	}
	free(stack_strings); // malloc()ed by backtrace_symbols
	fflush(stdout);
}
#else
void uvd_print_trace(const char *file, int line, const char *function)
{
	printf_warn("stack trace unsupported\n");
}
#endif

void UVDPrintfError(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	UVDPrintfErrorV(format, ap);
	va_end(ap);

}

#define CURSE_RED		"\x1b[0;31m"
#define CURSE_END		"\x1b[0;39m"

void UVDPrintfErrorV(const char *format, va_list ap)
{
	std::string curseStartStr;
	std::string curseEndStr;

	if( g_config ) {
		if( g_config->m_suppressErrors )
		{
			return;
		}
		if( g_config->m_curse && isatty(fileno(stdout)) )
		{
			curseStartStr = CURSE_RED;
			curseEndStr = CURSE_END;
		}
	}
	
	fflush(stdout);
	fprintf(stdout, "%sERROR%s: ", curseStartStr.c_str(), curseEndStr.c_str() );
	vfprintf(stdout, format, ap);
	fflush(stdout);
}

void UVDPrintfWarning(const char *format, ...)
{
	va_list ap;

	fflush(stdout);
	va_start(ap, format);
	UVDPrintfWarningV(format, ap);
	va_end(ap);

}

void UVDPrintfWarningV(const char *format, va_list ap)
{
	if( g_config && g_config->m_suppressErrors )
	{
		return;
	}
	
	fprintf(stdout, "%s", "WARNING: ");
	vfprintf(stdout, format, ap);
	fflush(stdout);
}

uv_err_t UVDSetDebugFile(const std::string &file)
{
	return UV_DEBUG(uv_log_init(file.c_str()));
}

void UVDPoke(uint8_t *in, size_t size)
{
	for( size_t i = 0; i < size; ++i )
	{
		if( in[i] )
		{
			std::string s;
		}
	}
}

