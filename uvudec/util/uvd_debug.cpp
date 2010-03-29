/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#include "uvd_config.h"
#include "uvd_debug.h"
#include "uvd_log.h"
#include <signal.h>
#include <stdio.h>
#include <stdarg.h>

static const char *g_last_func = NULL;

void uvd_signal_handler(int sig)
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

void printf_debug_level(int level, const char *format, ...)
{
	FILE *logHandle = g_log_handle;
	va_list ap;

	//Keep logging before g_config initialized
	if( !logHandle )
	{
		logHandle = stdout;
	}
	if( g_config )
	{
		//Is logging disabledor are we at too high of a level
		if( !g_config->m_verbose || level > g_config->m_verbose_level )
		{
			return;
		}
	}
	
	va_start(ap, format);
	vfprintf(logHandle, format, ap);
	fflush(logHandle);
	va_end(ap);
}

void uv_enter(const char *file, int line, const char *func)
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
