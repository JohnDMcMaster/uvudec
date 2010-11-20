/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvdflirt/plugin.h"
#include "uvdflirt/flirt.h"
#include "uvd/core/uvd.h"

UVDFLIRTPlugin *g_uvdFLIRTPlugin = NULL;

UVDFLIRTPlugin::UVDFLIRTPlugin()
{
	g_uvdFLIRTPlugin = this;
	m_flirt = NULL;
}

UVDFLIRTPlugin::~UVDFLIRTPlugin()
{
}

uv_err_t UVDFLIRTPlugin::init(UVDConfig *config)
{
	uv_assert_err_ret(UVDPlugin::init(config));
	uv_assert_err_ret(m_config.init());

	return UV_ERR_OK;
}

uv_err_t UVDFLIRTPlugin::getName(std::string &out)
{
	out = UVD_PLUGIN_NAME;
	return UV_ERR_OK;
}

uv_err_t UVDFLIRTPlugin::getDescription(std::string &out)
{
	out = "Fast Library Recognition Technology (FLIRT) implementation";
	return UV_ERR_OK;
}

uv_err_t UVDFLIRTPlugin::getVersion(UVDVersion &out)
{
	out.m_version = UVUDEC_VER_STRING;
	return UV_ERR_OK;
}

uv_err_t UVDFLIRTPlugin::getAuthor(std::string &out)
{
	out = "John McMaster <JohnDMcMaster@gmail.com>";
	return UV_ERR_OK;
}

uv_err_t UVDFLIRTPlugin::onUVDInit()
{
printf("FLIRT plugin init\n");
	m_flirt = new UVDFLIRT();
	uv_assert_ret(m_flirt);
	m_flirt->m_uvd = m_uvd;
	uv_assert_err_ret(m_flirt->init());

	return UV_ERR_OK;
}

uv_err_t UVDFLIRTPlugin::onUVDDeinit()
{
	delete m_flirt;
	m_flirt = NULL;

	return UV_ERR_OK;
}

uv_err_t UVD_PLUGIN_MAIN_SYMBOL(UVDConfig *config, UVDPlugin **out)
{
	*out = new UVDFLIRTPlugin();
	
	return UV_ERR_OK;
}

