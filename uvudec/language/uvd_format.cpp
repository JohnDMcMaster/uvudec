#include <string>
#include "uvd_format.h"
#include "uvd_config.h"
#include "uvd_types.h"

UVDFormat::UVDFormat()
{
	m_compiler = NULL;
}

UVDFormat::~UVDFormat()
{
	deinit();
}

uv_err_t UVDFormat::init()
{
	return UV_ERR_OK;
}

uv_err_t UVDFormat::deinit()
{
	delete m_compiler;
	m_compiler = NULL;
	
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

uv_err_t UVDFormat::formatAddress(uint32_t address, std::string &ret)
{
	char formatter[32];
	char buff[32];
	const char *hexPrefix = "";
	//const char *hexPrefix = "0x";
	
	if( !g_config )
	{
		ret = "";
		return UV_ERR_OK;
	}
	
	//Careful careful
	//Easy to make crash here
	snprintf(formatter, 32, "%s%%.%dX", hexPrefix, g_config->m_hex_addr_print_width);
	snprintf(buff, 32, formatter, address);
	ret = std::string(buff);

	return UV_ERR_OK;
}

uv_err_t UVDFormat::formatRegister(const std::string &reg, std::string &ret)
{
	char buff[32];

	if( !g_config )
	{
		ret = "";
		return UV_ERR_OK;
	}

	snprintf(buff, 32, "%s%s", g_config->m_reg_prefix.c_str(), reg.c_str());
	ret = std::string(buff);

	return UV_ERR_OK;
}

uv_err_t UVDFormat::setCompiler(UVDCompiler *compiler)
{
	uv_assert_ret(compiler);
	
	if( m_compiler )
	{
		delete m_compiler;
	}
	m_compiler = compiler;

	return UV_ERR_OK;
}
