#include "uvd_elf.h"
#include <string>

UVDElfSymbol::UVDElfSymbol()
{
	m_data = NULL;
}

UVDElfSymbol::~UVDElfSymbol()
{
}

void UVDElfSymbol::setSymbolName(const std::string &sName)
{
	m_sName = sName;
}

std::string UVDElfSymbol::getSymbolName()
{
	return m_sName;
}

uv_err_t UVDElfSymbol::getData(UVDData **data)
{
	uv_assert_ret(data);
	*data = m_data;
	return UV_ERR_OK;
}

void UVDElfSymbol::setData(UVDData *data)
{
	m_data = data;
}

