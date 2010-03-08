#include <string>
#include "uvd_format.h"
#include "uvd_config.h"
#include "uvd_types.h"

#if 0
unsigned int g_addr_min = 0;
//int g_addr_max = 0;
unsigned int g_addr_max = 0;
unsigned int g_hex_addr_print_width = 4;
int g_addr_comment = false;
int g_addr_label = false;
int g_print_used = false;
//FIXME: slow
int g_called_sources = false;
int g_called_count = true;
//FIXME: this is relaly slow, disable by default
int g_jumped_sources = false;
int g_jumped_count = true;
int g_print_string_table = false;
int g_print_header = false;

/* nothing (Intel), $ (MIPS) and % (gcc) are common */
char reg_prefix[8] = "";

//How many instances of the library are open
//static unsigned int g_open_instances = 0;

std::string g_mcu_name;
std::string g_mcu_desc;
std::string g_asm_imm_prefix;
std::string g_asm_imm_prefix_hex;
std::string g_asm_imm_postfix_hex;
std::string g_asm_imm_suffix;
int g_caps = FALSE;
int g_binary = FALSE;
struct uvd_mem_tag_t *g_memory_tags = NULL;
int g_memoric = 0;
int g_asm_instruction_info = 0;
int g_print_block_id = TRUE;
#endif

UVDFormat::UVDFormat()
{
	m_compiler = NULL;
}

uv_err_t UVDFormat::init()
{
	return UV_ERR_OK;
}

void UVDFormat::printFormatting()
{
	::printFormatting();
}

void printFormatting()
{
#if 0
	printf_debug("******\n");

	printf_debug("***\n");
	printf_debug("Flags:\n");
	printf_debug("g_addr_comment: %d\n", g_addr_comment);
	printf_debug("g_addr_label: %d\n", g_addr_label);
	printf_debug("g_caps: %d\n", g_caps);
	printf_debug("g_binary: %d\n", g_binary);
	printf_debug("g_memoric: %d\n", g_memoric);
	printf_debug("g_asm_instruction_info: %d\n", g_asm_instruction_info);

	printf_debug("***\n");
	printf_debug("Numeric:\n");
	printf_debug("g_addr_min: 0x%.8X\n", g_addr_min);
	printf_debug("g_addr_max: 0x%.8X\n", g_addr_max);

	printf_debug("***\n");
	printf_debug("Formatting strings:\n");
	printf_debug("g_mcu_name: %s\n", g_mcu_name.c_str());
	printf_debug("g_mcu_desc: %s\n", g_mcu_desc.c_str());
	printf_debug("g_asm_imm_prefix: %s\n", g_asm_imm_prefix.c_str());
	printf_debug("g_asm_imm_prefix_hex: %s\n", g_asm_imm_prefix_hex.c_str());
	printf_debug("g_asm_imm_postfix_hex: %s\n", g_asm_imm_postfix_hex.c_str());
	printf_debug("g_asm_imm_suffix: %s\n", g_asm_imm_suffix.c_str());
	printf_debug("reg_prefix: %s\n", reg_prefix);
#endif
}

std::string UVDFormat::formatAddress(uint32_t address)
{
	char formatter[32];
	char buff[32];
	
	if( !g_config )
	{
		return "";
	}
	
	snprintf(formatter, 32, "0x%%.%dX", g_config->m_hex_addr_print_width);
	snprintf(buff, 32, formatter, address);

	return std::string(buff);
}

std::string UVDFormat::formatRegister(const std::string &reg)
{
	char buff[32];
	
	if( !g_config )
	{
		return "";
	}
	
	snprintf(buff, 32, "%s%s", g_config->m_reg_prefix.c_str(), reg.c_str());

	return std::string(buff);
}
