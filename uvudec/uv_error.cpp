#include "uv_error.h"
#include "uv_log.h"
#include <stdio.h>

uv_err_t uv_err_ret_handler(uv_err_t rc, const char *file, int line, const char *func)
{
	const int buff_size = 1024;
	char buff[buff_size];
	if( UV_FAILED(rc) )
	{
		snprintf(buff, buff_size, "rc=%s", uv_err_str(rc));
		uv_log(UV_LOG_LVL_ERR, buff, file, line, func);
	}
	else if( UV_WARNING(rc) )
	{
		snprintf(buff, buff_size, "rc=%s", uv_err_str(rc));
		uv_log(UV_LOG_LVL_WARN, buff, file, line, func);
	}
	return rc;
}

#define uv_err_str_case(x) case x: return #x;
const char *uv_err_str(uv_err_t err)
{
	switch(err)
	{
	uv_err_str_case(UV_ERR_GENERAL);
	uv_err_str_case(UV_ERR_ACCESS);
	uv_err_str_case(UV_ERR_OUTMEM);
	uv_err_str_case(UV_ERR_NOTFOUND);
	uv_err_str_case(UV_ERR_ABORTED);
	uv_err_str_case(UV_ERR_ARGS);
	uv_err_str_case(UV_ERR_NOTSUPPORTED);
	uv_err_str_case(UV_ERR_BUFFERSIZE);
	uv_err_str_case(UV_ERR_ARBITRARYLIMIT);

	uv_err_str_case(UV_ERR_DISASM_COMBO);
	uv_err_str_case(UV_ERR_DISASM_NODAT);
	uv_err_str_case(UV_ERR_DISASM_PREFIX);

	default:
		return "UNKNOWN";
	}
}
