/*
Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under the terms of the BSD license.  See LICENSE for details.
*/

#ifndef UV_DISASM_MAP_H
#define UV_DISASM_MAP_H

#include "uv_disasm_types.h"

#ifdef __cplusplus
extern "C"
{
#endif /* ifdef __cplusplus */

/* Store symbols in a string keyed binary search tree */
struct uv_disasm_generic_map_t
{
	void *key;	
	void *value;
	struct uv_disasm_generic_map_t *left;
	struct uv_disasm_generic_map_t *right;
};

struct uv_disasm_generic_map_t* uv_disasm_generic_map_alloc(void);
void uv_disasm_generic_map_free(struct uv_disasm_generic_map_t *in, 
		uv_disasm_free_t free_func_key, uv_disasm_free_t free_func_val);

uv_err_t uv_disasm_generic_map_set(struct uv_disasm_generic_map_t **root_in,
		uv_disasm_cmp_func_t cmp,
		void *key, void *value, void **old_value,
		uv_disasm_free_t free_func_key, uv_disasm_free_t free_func_val);
uv_err_t uv_disasm_generic_map_get(struct uv_disasm_generic_map_t *root,
		uv_disasm_cmp_func_t cmp,
		void *key, void **value);

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* UV_DISASM_MAP_H */
