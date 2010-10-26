/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

/*
XXX
This file is deprecated
It will be removed soon
DO NOT use its API
*/

#include "uvd/config/arg_util.h"
#include "uvd/config/config.h"
#include "uvd/util/debug.h"
#include "uvd/util/error.h"
#include "uvd/util/log.h"
#include "uvd/util/types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static std::string g_logFile;
FILE *g_log_handle = NULL;

uv_err_t uv_log(uv_log_level level, const char *message, const char *file, int line, const char *func)
{
	const char *level_str = "UNKNOWN";
	FILE *logHandle = g_log_handle;

	if( !logHandle )
	{
		printf_warn("tried to log before logging setup\n");
		logHandle = stdout;
	}
	
	//Only print if debugging, otherwise just clutters up screen
	if( !UVDAnyDebugActive() )
	{
		return UV_ERR_OK;
	}
	
	switch( level )
	{
	case UV_LOG_LVL_DEBUG:
		level_str = "DEBUG";
		break;
	case UV_LOG_LVL_WARN:
		level_str = "WARNING";
		break;
	case UV_LOG_LVL_BAD:
		level_str = "BAD";
		break;
	case UV_LOG_LVL_ERR:
		level_str = "ERROR";
		break;
	case UV_LOG_LVL_CRIT:
		level_str = "CRITICAL";
		break;
	}
	fprintf(logHandle, "%s: %s (%s:%d): %s\n", level_str, func, file, line, message);
	fflush(logHandle);

	return UV_ERR_OK;
}

uv_err_t UVDLogSetFile(const std::string &logFile)
{
	if( g_log_handle )
	{
		if( g_log_handle != stdout && g_log_handle != stderr )
		{
			fclose(g_log_handle);
			g_log_handle = NULL;
		}
	}
	
	g_logFile = logFile;
	
	//UV_LOG_DEFAULT_FILE
	if( UV_FAILED(parseFileOption(logFile, &g_log_handle)) )
	{
		printf_error("Could not open file: %s\n", logFile.c_str());
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	uv_assert_ret(g_log_handle);

	return UV_ERR_OK;
}

uv_err_t uv_log_get_file(uv_const_char_ptr *log_file)
{
	uv_err_t rc = UV_ERR_GENERAL;
	if( !log_file )
	{
		goto error;
	}
	*log_file = g_logFile.c_str();
	rc = UV_ERR_OK;

error:
	return rc;
}

uv_err_t uv_log_init(const char *log_file)
{
	uv_err_t rc = UV_ERR_GENERAL;
	std::string logFile;

	if( log_file )
	{
		logFile = log_file;
	}
	
	uv_assert_err(UVDLogSetFile(logFile));

	if( g_log_handle && g_config && g_config->m_verbose )
	{
		fprintf(g_log_handle, "\n\nLog started\n");
	}
	rc = UV_ERR_OK;

error:
	if( UV_FAILED(rc) )
	{
		uv_log_deinit();
	}
	return rc;
}

uv_err_t uv_log_deinit(void)
{
	uv_err_t rc = UV_ERR_GENERAL;
	if( g_log_handle )
	{
		if( g_config && g_config->m_verbose )
		{
			fprintf(g_log_handle, "Log terminating\n");
		}
		
		if( g_log_handle != stdout && g_log_handle != stderr )
		{
			fclose(g_log_handle);
		}
		g_log_handle = NULL;
	}
	rc = UV_ERR_OK;

	return rc;
}
