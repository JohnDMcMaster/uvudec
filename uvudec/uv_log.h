/*
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under the terms of the BSD license.  See LICENSE for details.
*/

/*
Define UV_LOG_DISABLED to disable logging on preprocessor level
*/
#ifndef UV_LOG_H
#define UV_LOG_H

#include "uv_error.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif /* ifdef __cplusplus */

typedef uint32_t uv_log_level;



//Debug message, does not indicate anything is wrong
#define UV_LOG_LVL_DEBUG					0
//Warning message, indicates something should be done with care
//Ex: deprecated API
#define UV_LOG_LVL_WARN						1
//Something that is very likely, but not necessarily going to cause an error
//Ex: out of memory.  Operation can be retried with smaller resource requests
#define UV_LOG_LVL_BAD						2
//A fatal error has occured, but probably before cuasing unrecoverable damage
//Comes from common errors to check for
#define UV_LOG_LVL_ERR						3
//The entire library has been corrupted
//Program is likely to be unstable, ie likely to crash
#define UV_LOG_LVL_CRIT						4

#define UV_LOG_DEFAULT_FILE				"uv_log.txt"
#define UV_LOG_LVL(level, message) uv_log(level, message, __FILE__, __LINE__)
#define UV_LOG(message) UV_LOG_LVL(UV_LOG_LVL_DEBUG, message)
#define UV_LOG_ERR(message) UV_LOG_LVL(UV_LOG_LVL_ERR, message)


#ifdef UV_LOG_DISABLED

#define uv_log(level, message, file, line, func)		UV_ERR_OK
#define uv_log_set_file(log_file)						UV_ERR_OK
#define uv_log_get_file(log_file)						UV_ERR_OK
#define uv_log_init(log_file)							UV_ERR_OK
#define uv_log_deinit()									UV_ERR_OK

#else //ifdef UV_LOG_DISABLED

uv_err_t uv_log(uv_log_level level, const char *message, const char *file, int line, const char *func);
uv_err_t uv_log_set_file(const char *log_file);
uv_err_t uv_log_get_file(uv_const_char_ptr *log_file);

//If NULL, will assume default log file
//If empty, will not log.  This in future should be more policy controlled
uv_err_t uv_log_init(const char *log_file);
uv_err_t uv_log_deinit(void);

#endif //ifdef UV_LOG_DISABLED

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif //ifndef UV_LOG_H

