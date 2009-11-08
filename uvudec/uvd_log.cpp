#include "uvd_log.h"
#include "uvd_error.h"
#include "uvd_debug.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

char *g_log_file = NULL;
FILE *g_log_handle = NULL;


uv_err_t uv_log(uv_log_level level, const char *message, const char *file, int line, const char *func)
{
	uv_err_t rc = UV_ERR_GENERAL;
	const char *level_str = "UNKNOWN";
	
	if( !g_log_handle )
	{
		fprintf(stderr, "Warning: tried to log before logging setup\n");
		fprintf(stderr, "Message: %s\n", message);
		goto error;
	}
	
	if( !g_verbose )
	{
		rc = UV_ERR_OK;
		goto error;
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
	fprintf(g_log_handle, "%s: %s (%s:%d): %s\n", level_str, func, file, line, message);
	fflush(g_log_handle);
	rc = UV_ERR_OK;

error:
	return rc;
}

uv_err_t uv_log_set_file(const char *log_file)
{
	uv_err_t rc = UV_ERR_GENERAL;
	if( g_log_file )
	{
		if( g_log_handle )
		{
			fclose(g_log_handle);
			g_log_handle = NULL;
		}
		if( g_log_file )
		{
			free(g_log_file);
			g_log_file = NULL;
		}
	}
		
	if( !log_file )
	{
		log_file = UV_LOG_DEFAULT_FILE;
	}
	g_log_file = strdup(log_file);
	
	//Use "" as an invalid log file (logging disabled)
	if( !strcmp(g_log_file, "") )
	{
	}
	else if( !strcmp(g_log_file, "-") )
	{
		g_log_handle = stdout;
	}
	else
	{
		g_log_handle = fopen(g_log_file, "w+");
		if( !g_log_handle )
		{
			fprintf(stderr, "Cannot open log file\n");
			goto error;
		}
	}
	rc = UV_ERR_OK;
	

error:
	if( UV_FAILED(rc) )
	{
		if( g_log_handle )
		{
			fclose(g_log_handle);
			g_log_handle = NULL;
		}
		if( g_log_file )
		{
			free(g_log_file);
			g_log_file = NULL;
		}		
	}
	return rc;
}

uv_err_t uv_log_get_file(uv_const_char_ptr *log_file)
{
	uv_err_t rc = UV_ERR_GENERAL;
	if( !log_file )
	{
		goto error;
	}
	*log_file = g_log_file;
	rc = UV_ERR_OK;

error:
	return rc;
}


void uv_signal_handler(int sig)
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
	case SIGILL:
		sig_str = "SIGKILL";
		break;
	}
	
	printf("\n\nSEVERE ERROR\n");
	/*
	i before e, except after c... 
	...and you'll never be right, no matter what you say!
	*/
	printf("Received signal: %s\n", sig_str);
#ifdef NDEBUG
	printf("Not compiled with debugging support, no trace information availible\n");
#else
	printf("Last function: %s\n", get_last_func());
#endif
	/* exit() is not a "safe" function.  See man signal */
	_exit(1);
}


uv_err_t uv_log_init(const char *log_file)
{
	uv_err_t rc = UV_ERR_GENERAL;
	if( log_file )
	{
		if( UV_FAILED(uv_log_set_file(log_file)) )
		{
			goto error;
		}
	}
	else
	{
		g_log_handle = g_pDebugFile;
	}
	if( g_log_handle && g_verbose )
	{
		fprintf(g_log_handle, "\n\nLog started\n");
	}
	signal(SIGSEGV, uv_signal_handler);
	signal(SIGFPE, uv_signal_handler);
	signal(SIGKILL, uv_signal_handler);
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
	if( g_log_handle && g_verbose )
	{
		fprintf(g_log_handle, "Log terminating\n");
	}

	if( g_log_handle )
	{
		fclose(g_log_handle);
		g_log_handle = NULL;
	}
	if( g_log_file )
	{
		free(g_log_file);
		g_log_file = NULL;
	}
	rc = UV_ERR_OK;

	return rc;
}

