/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "plugin/plugin.h"
#include "uvd_config.h"
#include <dlfcn.h>

UVDPlugin::UVDPlugin()
{
	m_hLibrary = NULL;
	m_uvd = NULL;
}

UVDPlugin::~UVDPlugin()
{
	if( m_hLibrary )
	{
		dlclose(m_hLibrary);
	}
}

uv_err_t UVDPlugin::init(UVDConfig *config)
{
	return UV_ERR_OK;
}

uv_err_t UVDPlugin::deinit(UVDConfig *config)
{
	return UV_ERR_OK;
}

uv_err_t UVDPlugin::onUVDInit(UVD *uvd)
{
	m_uvd = uvd;
	return UV_ERR_OK;
}

uv_err_t UVDPlugin::onUVDDeinit(UVD *uvd)
{
	return UV_ERR_OK;
}

uv_err_t UVDPlugin::getDependencies(PluginDependencies &out)
{
	out.clear();
	return UV_ERR_OK;
}

uv_err_t UVDPlugin::getArchitecture(UVDData *data, const std::string &architecture, UVDArchitecture **out)
{
	return UV_ERR_NOTSUPPORTED;
}

uv_err_t UVDPlugin::getObject(const UVDData *data, const std::string &architecture, UVDObject **out)
{
	return UV_ERR_NOTSUPPORTED;
}

uv_err_t UVDPlugin::registerArgument(const std::string &propertyForm,
		char shortForm, std::string longForm, 
		std::string helpMessage,
		uint32_t numberExpectedValues,
		UVDArgConfigHandler handler,
		bool hasDefault)
{
	std::string pluginName;
	uv_assert_err_ret(getName(pluginName));

	uv_assert_err_ret(g_config->registerArgument(propertyForm,
			shortForm, longForm, 
			helpMessage,
			numberExpectedValues,
			handler,
			hasDefault,
			pluginName));

	return UV_ERR_OK;
}

uv_err_t UVDPlugin::registerArgument(const std::string &propertyForm,
		char shortForm, std::string longForm, 
		std::string helpMessage,
		std::string helpMessageExtra,
		uint32_t numberExpectedValues,
		UVDArgConfigHandler handler,
		bool hasDefault)
{
	std::string pluginName;
	uv_assert_err_ret(getName(pluginName));

	uv_assert_err_ret(g_config->registerArgument(propertyForm,
			shortForm, longForm, 
			helpMessage,
			helpMessageExtra,
			numberExpectedValues,
			handler,
			hasDefault,
			pluginName));
	return UV_ERR_OK;
}

