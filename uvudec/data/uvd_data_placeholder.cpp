#include "uvd_data.h"

UVDDataPlaceholder::UVDDataPlaceholder()
{
}

UVDDataPlaceholder::~UVDDataPlaceholder()
{
}

std::string UVDDataPlaceholder::getSource()
{
	return "placeholder";
}

uint32_t UVDDataPlaceholder::size() const
{
	return 0;
}

uv_err_t UVDDataPlaceholder::size(uint32_t *sizeOut) const
{
	uv_assert_ret(sizeOut);
	*sizeOut = 0;
	return UV_ERR_OK;
}

int UVDDataPlaceholder::read(unsigned int offset, char *buffer, unsigned int bufferSize) const	
{
	return 0;
}

int UVDDataPlaceholder::read(unsigned int offset) const
{
	return 0;
}

int UVDDataPlaceholder::read(unsigned int offset, std::string &s, unsigned int readSize) const
{
	return 0;
}

uv_err_t UVDDataPlaceholder::writeData(unsigned int offset, const char *buffer, unsigned int bufferSize)
{
	if( bufferSize == 0 )
	{
		return UV_ERR_OK;
	}
	return UV_DEBUG(UV_ERR_GENERAL);
}
