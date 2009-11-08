#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include "uvd_types.h"
#include "uvd_debug.h"
#include "uvd_error.h"

int g_verbose = FALSE;
const char *g_last_func = NULL;
int g_verbose_init = FALSE;
int g_verbose_processing = FALSE;
int g_verbose_analysis = FALSE;
int g_verbose_printing = FALSE;
int g_verbose_level = UVD_DEBUG_NONE;
FILE *g_pDebugFile = stdout;
FILE *g_pOutputFile = stdout;

void uv_enter(const char *file, int line, const char *func)
{
	printf_debug_level(UVD_DEBUG_VERBOSE, "ENTER: %s:%d::%s\n", file, line, func);
	g_last_func = func;
}

const char *get_last_func()
{
	return g_last_func;
}

void printf_debug_level(int level, const char *format, ...)
{
	if( g_verbose && level <= g_verbose_level )
	{
		va_list ap;
		
		va_start(ap, format);
		vfprintf(g_pDebugFile, format, ap);
		fflush(stdout);
		va_end(ap);
	}
}
