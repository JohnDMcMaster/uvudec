#include "uvd/compiler/compiler.h"

/*

Unknown if will be possible to make compiler configuration or will need to have formatting code written


What differs between same language compilers?
-What causes warnings
-Embedded has many very compiler specific syntaxes
	Ex: most compilers support some form of invidual bit referencing where as compilers like GCC do not
-Standard libraries
	Should be addressed by the known functions archive
-Startup routines
	Should be addressed by the known functions archive
	May require some sort of signature though to collect entire CRT
-ABI
	Define calling conventions
	Hopefully can guess calling by referenced variables
	Global variables are sometimes used, can be tricky to figure out if global or simply changed right before the call
-Use of instructions
	May not even be aware of some instructions
-Code generation
	May require signatures to parse for loops and such


SDCC inline assembly example (from SDCC manual)

Issues shows
-need to translate between assembly variables and language (C) variables
-How to setup start and end assembly
-avoid warning is intersting to note
	Esp considering that would issue "statement has no effect" on GCC
-Will have to figure out how register saving fits into whole mess
	Compiler calling convention will have to be documented?
	What if its highly optomized and doesn't really use convention?

unsigned char __far __at(0x7f00) buf[0x100];
unsigned char head, tail;
#define USE_ASSEMBLY (1)
#if !USE_ASSEMBLY
void to_buffer( unsigned char c )
{
    if( head != (unsigned char)(tail-1) )
        buf[ head++ ] = c;
}
#else
void to_buffer( unsigned char c )
{
    c; // to avoid warning:   unreferenced function argument
    _asm
        ; save used registers here.
        ; If we were still using r2,r3 we would have to push them here.
; if( head != (unsigned char)(tail-1) )
        mov a,_tail
        dec  a
        xrl a,_head
        ; we could do an ANL a,#0x0f here to use a smaller buffer (see below)
        jz   t_b_end$
        ;
; buf[ head++ ] = c;
        mov  a,dpl         ; dpl holds lower byte of function argument
        mov  dpl,_head     ; buf is 0x100 byte aligned so head can be used directly
        mov  dph,#(_buf> >8)
        movx @dptr,a
        inc  _head
        ; we could do an ANL _head,#0x0f here to use a smaller buffer (see above)
t_b_end$:
        ; restore used registers here
    _endasm;
}
#endif
*/

UVDCompiler::UVDCompiler()
{
	m_compiler = UVD_COMPILER_UNKNOWN;
}

UVDCompiler::~UVDCompiler()
{
}

uv_err_t UVDCompiler::getCompiler(int compilerType, const std::string &version, UVDCompiler **compilerOut)
{
	UVDCompiler *compiler = NULL;

	uv_assert_ret(compilerOut);

	compiler = new UVDCompiler();
	uv_assert_ret(compiler);
	
	compiler->m_version = version;
	
	compiler->m_compiler = compilerType;
	
	*compilerOut = compiler;
	return UV_ERR_OK; 
}


uv_err_t UVDCompiler::comment(const std::string &in, std::string &out)
{
	return UV_DEBUG(UV_ERR_GENERAL);
}

uv_err_t UVDCompiler::commentAggressive(const std::string &in, std::string &out)
{
	//Default: drop all data
	if( UV_FAILED(comment(in, out)) )
	{
		out = "";
	}
	return UV_ERR_OK;
}

uv_err_t UVDCompiler::getCompilerFromFile(const std::string &file, UVDCompiler **compilerOut)
{
	uv_err_t rc = UV_ERR_GENERAL;
#if 0
	/* 
	The misc section is non-grouped directives.  
	As such, parsing it is much easier than most sections 
	*/
	unsigned int cur_line = 0;
	//char **lines = NULL;

	std::string value_name;
	std::string value_desc;
	std::string value_prefix;
	std::string value_prefix_hex;
	std::string value_postfix_hex;
	std::string value_suffix;

	
	UV_ENTER();
	printf_debug("Initializing compiler data\n");
	if( misc_section == NULL )
	{
		printf_debug(".MISC section required\n");
		UV_ERR(rc);
		goto error;
	}


	//lines = misc_section->m_lines;
	//n_lines = misc_section->m_n_lines;
		
	printf_debug("Pre-parsing compiler lines...\n");
	
	for( cur_line = 0; cur_line < misc_section->m_lines.size(); ++cur_line )
	{
		std::string key;
		std::string value;
		std::string line = misc_section->m_lines[cur_line];
		uv_err_t rc_temp = UV_ERR_GENERAL;

		printf_debug("Line: <%s>\n", line.c_str());
		rc_temp = uvdParseLine(line, key, value);
		if( UV_FAILED(rc_temp) )
		{
			UV_ERR(rc);
			goto error;
		}
		else if( rc_temp == UV_ERR_BLANK )
		{
			continue;
		}

		if( !strcmp(key.c_str(), "NAME") )
		{
			value_name = value;
		}
		else if( !strcmp(key.c_str(), "DESC") )
		{
			value_desc = value;
		}
		else if( !strcmp(key.c_str(), "LANG_SRC") )
		{
			value_lang_src = value;
		}
		else if( !strcmp(key.c_str(), "LANG_DST") )
		{
			value_lang_dst = value;
		}
		else if( !strcmp(key.c_str(), "ASM_IMM_PREFIX") )
		{
			value_prefix = value;
		}
		else if( !strcmp(key.c_str(), "ASM_IMM_PREFIX_HEX") )
		{
			value_prefix_hex = value;
		}
		else if( !strcmp(key.c_str(), "ASM_IMM_POSTFIX_HEX") )
		{
			value_postfix_hex = value;
		}
		else if( !strcmp(key.c_str(), "ASM_IMM_SUFFIX") )
		{
			value_suffix = value;
		}
		else
		{
			printf_debug("Invalid key\n");
			UV_ERR(rc);
			goto error;
		}
	}
	
	if( value_name.empty() )
	{
		printf("MCU name not present\n");
		UV_ERR(rc);
		goto error;
	}
	g_mcu_name = value_name;
	
	//MCU desc is optional
	if( value_desc.empty() && g_format_debug )
	{
		value_desc = "<NONE>";
	}
	g_mcu_desc = value_desc;
	
	if( value_prefix.empty() && g_format_debug )
	{
		value_prefix = "<NONE>";
	}
	g_asm_imm_prefix = value_prefix;
	
	if( value_prefix_hex.empty() && g_format_debug )
	{
		value_prefix_hex = "<NONE>";
	}
	g_asm_imm_prefix_hex = value_prefix_hex;

	if( value_postfix_hex.empty() && g_format_debug )
	{
		value_postfix_hex = "<NONE>";
	}
	g_asm_imm_postfix_hex = value_postfix_hex;

	if( value_suffix.empty() && g_format_debug )
	{
		value_suffix = "<NONE>";
	}
	g_asm_imm_suffix = value_suffix;

	printf_debug("Misc init OK\n");

	rc = UV_ERR_OK;
error:
#endif
	return UV_DEBUG(rc);
}
