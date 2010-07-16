/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd_config.h"
#include "uvd_debug.h"
#include "uvd_log.h"
#include <signal.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#ifdef __GNUC__
#include <cxxabi.h>
#include <execinfo.h>
#endif

static const char *g_last_func = NULL;

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
	//exit() is not a "safe" function.  See man signal
	_exit(1);
}

void printf_debug_level(uint32_t level, const char *format, ...)
{
	FILE *logHandle = g_log_handle;
	va_list ap;
	uint32_t verbose = 0;
	uint32_t set_level = 0;

	//Keep logging before g_config initialized
	if( !logHandle )
	{
		logHandle = stdout;
	}
	if( g_config )
	{
		verbose = g_config->m_verbose;
		set_level = g_config->m_verbose_level;
	}
	else
	{
		verbose = 0;
		set_level = 0;
	}
	
	//Is logging disabled or are we at too high of a level?
	if( !verbose || level > set_level )
	{
		return;
	}
	
	va_start(ap, format);
	vfprintf(logHandle, format, ap);
	fflush(logHandle);
	va_end(ap);
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
	signal(SIGSEGV, uvd_signal_handler);
	signal(SIGFPE, uvd_signal_handler);
	return UV_ERR_OK;
}

uv_err_t UVDDebugDeinit()
{
	return UV_ERR_OK;
}

#ifdef __GNUC__
void uvd_print_trace(const char *file, int line, const char *function)
{
	/*
	Based on code from http://tombarta.wordpress.com/2008/08/01/c-stack-traces-with-gcc/
	*/
    const size_t max_depth = 100;
    size_t stack_depth;
    void *stack_addrs[max_depth];
    char **stack_strings;

    stack_depth = backtrace(stack_addrs, max_depth);
    stack_strings = backtrace_symbols(stack_addrs, stack_depth);

    printf("Call stack from %s:%d:\n", file, line);

    for (size_t i = 1; i < stack_depth; i++) {
        printf("    %s\n", stack_strings[i]);
    }
    free(stack_strings); // malloc()ed by backtrace_symbols
    fflush(stdout);
 #if 0
    const size_t max_depth = 100;
    size_t stack_depth;
    void *stack_addrs[max_depth];
    char **stack_strings;

    stack_depth = backtrace(stack_addrs, max_depth);
    stack_strings = backtrace_symbols(stack_addrs, stack_depth);

    printf("Call stack from %s @ %s:%d:\n", function, file, line);

	for (size_t i = 1; i < stack_depth; i++) {
		size_t sz = 200; // just a guess, template names will go much wider
		char *function = (char *)malloc(sz);
		char *begin = 0, *end = 0;
		// find the parentheses and address offset surrounding the mangled name
		for (char *j = stack_strings[i]; *j; ++j) {
		    if (*j == '(') {
		        begin = j;
		    }
		    else if (*j == '+') {
		        end = j;
		    }
		}
		if (begin && end) {
		    *begin++ = ' ';
		    *end = ' ';
		    // found our mangled name, now in [begin, end)

		    int status;
		    char *ret = abi::__cxa_demangle(begin, function, &sz, &status);
		    if (ret) {
		        // return value may be a realloc() of the input
		        function = ret;
		    }
		    else {
		        // demangling failed, just pretend it's a C function with no args
		        strncpy(function, begin, sz);
		        strncat(function, "()", sz);
		        function[sz-1] = ' ';
		    }
		    printf("    %s:%s\n", stack_strings[i], function);
		}
		else
		{
		    // didn't find the mangled name, just print the whole line
		    printf("    %s\n", stack_strings[i]);
		}
		free(function);
	}
    free(stack_strings); // malloc()ed by backtrace_symbols
    fflush(stdout);
#endif
}
#else
void uvd_print_trace(const char *file, int line, const char *function)
{
	printf_warn("stack trace unsupported\n");
}
#endif

