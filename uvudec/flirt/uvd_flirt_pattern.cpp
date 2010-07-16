/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd_config.h"
#include "uvd_crc.h"
#include "uvd_flirt_pattern.h"
#include "uvd_library.h"
#include "uvd_util.h"

UVDFLIRTPatternGenerator::UVDFLIRTPatternGenerator()
{
}

UVDFLIRTPatternGenerator::~UVDFLIRTPatternGenerator()
{
	deinit();
}

uv_err_t UVDFLIRTPatternGenerator::deinit()
{
	return UV_ERR_OK;
}

uv_err_t UVDFLIRTPatternGenerator::saveToFile(const std::string &inputFile, const std::string &file)
{
	std::string output;
	
	uv_assert_err_ret(saveToString(inputFile, output, true));
	uv_assert_err_ret(writeFile(file, output));
	
	return UV_ERR_OK;
}

uv_err_t UVDFLIRTPatternGenerator::saveToString(const std::string &inputFile, std::string &output, uint32_t terminateFile)
{
	uv_assert_err_ret(saveToStringCore(inputFile, output));
	
	if( terminateFile )
	{
		output += "---" + g_config->m_flirt.m_patternFileNewline;
	}

	return UV_ERR_OK;
}
