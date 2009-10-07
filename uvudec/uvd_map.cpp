#ifdef NOT_DEFINED
#include "uv_disasm_map.h"
#include "uv_disasm_types.h"
#include "uv_error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct uv_disasm_generic_map_t* uv_disasm_generic_map_alloc(void)
{
	struct uv_disasm_generic_map_t* ret = NULL;
	ret = (struct uv_disasm_generic_map_t*)malloc(sizeof(struct uv_disasm_generic_map_t));
	if( ret )
	{
		memset(ret, 0, sizeof(struct uv_disasm_generic_map_t));
	}
	return ret;
}

void uv_disasm_generic_map_free(struct uv_disasm_generic_map_t *in, 
		uv_disasm_free_t free_func_key, uv_disasm_free_t free_func_val)
{
	if( !in )
	{
		return;
	}

	if( free_func_key )
	{
		free_func_key(in->key);
	}
	if( free_func_val )
	{
		free_func_val(in->value);
	}

	if( in->left )
	{
		uv_disasm_generic_map_free(in->left, free_func_key, free_func_val);
	}
	if( in->right )
	{
		uv_disasm_generic_map_free(in->right, free_func_key, free_func_val);
	}
	
	free(in);
}

int uv_disasm_generic_map_default_cmp(void *left, void *right)
{
	return ((int)right) - ((int)left);
}

uv_err_t uv_disasm_generic_map_set(struct uv_disasm_generic_map_t **root_in, 
		uv_disasm_cmp_func_t cmp_func,
		void *key, void *value, void **old_value, 
		uv_disasm_free_t free_func_key, uv_disasm_free_t free_func_val)
{
	uv_err_t rc = UV_ERR_GENERAL;

	uv_assert(root_in);
	uv_assert(key);
	/* Let value be NULL */
	uv_assert(value);

	if( !cmp_func )
	{
		cmp_func = uv_disasm_generic_map_default_cmp;
	}
	
	/* Base case: previous key did not exist */
	if( !(*root_in) )
	{
		struct uv_disasm_generic_map_t *root_temp;

		root_temp = uv_disasm_generic_map_alloc();
		uv_assert_all(root_temp);

		root_temp->key = strdup(key);
		uv_assert_all(root_temp->key);

		root_temp->left = NULL;
		root_temp->right = NULL;
		root_temp->value = value;
		*root_in = root_temp;
		if( old_value )
		{
			*old_value = 0;
		}
	}
	/* Node exists */
	else
	{
		struct uv_disasm_generic_map_t *root = *root_in;
		int comp = 0;
		
		comp = cmp_func(key, root->key);
		
		/* Previous is less than new */
		if( comp < 0 )
		{
			uv_assert_err(uv_disasm_generic_map_set(&root->left, cmp_func, 
					key, value, old_value, 
					free_func_key, free_func_val));
		}
		/* Previous is greater than new */
		else if( comp > 0 )
		{
			uv_assert_err(uv_disasm_generic_map_set(&root->right, cmp_func, 
					key, value, old_value, 
					free_func_key, free_func_val));
		}
		/* Same key: replace */
		else
		{
			/* Report old value if allowed, use free func, or just ignore and assume its trivial */
			if( old_value )
			{
				*old_value = root->value;
			}
			else if( free_func_val )
			{
				free_func_val(root->value);
			}
			root->value = value;
		}
	}

	rc = UV_ERR_OK;

error:
	return UV_DEBUG(rc);
}

uv_err_t uv_disasm_generic_map_get(struct uv_disasm_generic_map_t *root, 
		uv_disasm_cmp_func_t func_cmp,
		void *key, void **value)
{
	uv_err_t rc = UV_ERR_GENERAL;
	int comp = 0;

	UV_ENTER();
	uv_assert_all(root);
	uv_assert(root->key);
	uv_assert(key);
	uv_assert(value);
	
	printf_debug("assert done\n"); fflush(stdout);
	
	printf_debug("Want: 0x%.8X, cur key: 0x%.8X, left: 0x%.8X, right: 0x%.8X\n", (unsigned int)key, (unsigned int)root->key, (unsigned int)root->left, (unsigned int)root->right);
	fflush(stdout);
	
	comp = func_cmp(key, root->key);
	/* Previous is less than check */
	if( comp < 0 )
	{
		uv_assert_err(uv_disasm_generic_map_get(root->left, func_cmp, key, value));
	}
	/* Previous is greater than check */
	else if( comp > 0 )
	{
		uv_assert_err(uv_disasm_generic_map_get(root->right, func_cmp, key, value));
	}
	/* Same key: match */
	else
	{
		/* Allow NULL key? */
		/* uv_assert(root->value); */
		*value = root->value;
	}

	rc = UV_ERR_OK;

error:
	return UV_DEBUG(rc);
}

#endif
