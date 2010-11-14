/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/core/uvd.h"
#include "uvd/flirt/flirt.h"
#include "uvdbfd/architecture.h"
#include "uvdbfd/object.h"
#include "uvdbfd/plugin.h"
#include "uvdbfd/flirt/flirt.h"

UVDBFDPlugin *g_bfdPlugin = NULL;

UVDBFDPlugin::UVDBFDPlugin()
{
	g_bfdPlugin = this;
}

UVDBFDPlugin::~UVDBFDPlugin()
{
}

uv_err_t UVDBFDPlugin::init(UVDConfig *config)
{
	uv_assert_err_ret(UVDPlugin::init(config));
	uv_assert_err_ret(m_config.init(config));
	return UV_ERR_OK;
}

uv_err_t UVDBFDPlugin::onUVDInit()
{
	printf_flirt_debug("bfd on uvd init\n");
	/*
	UVDFLIRTPatternGeneratorBFD *generatorBFD = NULL;
	
	uv_assert_err_ret(UVDFLIRTPatternGeneratorBFD::getPatternGenerator(&generatorBFD));
	uvd->m_flirt->m_patFactory.m_patternGenerators.push_back(generatorBFD);
	*/
	std::string pluginName;
	uv_assert_err_ret(getName(pluginName));
	uv_assert_err_ret(m_uvd->m_flirt->m_patFactory.registerObject(pluginName,
			UVDFLIRTPatternGeneratorBFD::canLoad, UVDFLIRTPatternGeneratorBFD::tryLoad,
			this));
	return UV_ERR_OK;
}

uv_err_t UVDBFDPlugin::getName(std::string &out)
{
	out = UVD_PLUGIN_NAME;
	return UV_ERR_OK;
}

uv_err_t UVDBFDPlugin::getDescription(std::string &out)
{
	out = "GNU binutils (libbfd and friends) capabilities";
	return UV_ERR_OK;
}

uv_err_t UVDBFDPlugin::getVersion(UVDVersion &out)
{
	out.m_version = UVUDEC_VER_STRING;
	return UV_ERR_OK;
}

uv_err_t UVDBFDPlugin::getAuthor(std::string &out)
{
	out = "John McMaster <JohnDMcMaster@gmail.com>";
	return UV_ERR_OK;
}

uv_err_t UVDBFDPlugin::canGetObject(const UVDData *data, const UVDRuntimeHints &hints, uvd_priority_t *confidence)
{
	return UV_DEBUG(UVDBFDObject::canLoad(data, hints, confidence, this));
}

uv_err_t UVDBFDPlugin::getObject(UVDData *data, const UVDRuntimeHints &hints, UVDObject **out)
{
	return UV_DEBUG(UVDBFDObject::tryLoad(data, hints, out, this));
}

uv_err_t UVDBFDPlugin::canGetArchitecture(const UVDObject *object, const UVDRuntimeHints &hints, uvd_priority_t *confidence)
{
	return UV_DEBUG(UVDBFDArchitecture::canLoad(object, hints, confidence, this));
}

uv_err_t UVDBFDPlugin::getArchitecture(UVDObject *object, const UVDRuntimeHints &hints, UVDArchitecture **out)
{
	return UV_DEBUG(UVDBFDArchitecture::tryLoad(object, hints, out, this));
}

