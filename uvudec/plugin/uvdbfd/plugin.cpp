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

UVDBFDPlugin::UVDBFDPlugin()
{
}

UVDBFDPlugin::~UVDBFDPlugin()
{
}

uv_err_t UVDBFDPlugin::onUVDInit(UVD *uvd)
{
printf("bfd on uvd init\n");
	/*
	UVDFLIRTPatternGeneratorBFD *generatorBFD = NULL;
	
	uv_assert_err_ret(UVDFLIRTPatternGeneratorBFD::getPatternGenerator(&generatorBFD));
	uvd->m_flirt->m_patFactory.m_patternGenerators.push_back(generatorBFD);
	*/
	std::string pluginName;
	uv_assert_err_ret(getName(pluginName));
	uv_assert_err_ret(uvd->m_flirt->m_patFactory.registerObject(pluginName,
			UVDFLIRTPatternGeneratorBFD::canLoad, UVDFLIRTPatternGeneratorBFD::tryLoad));
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

uv_err_t UVDBFDPlugin::getArchitecture(UVDObject *object, const UVDRuntimeHints &hints, UVDArchitecture **out)
{
	UVDBFDArchitecture *ret = NULL;
	
	ret = new UVDBFDArchitecture();
	uv_assert_ret(ret);
	uv_assert_err_ret(ret->init());
	
	uv_assert_ret(out);
	*out = ret;
	
	return UV_ERR_OK;
}

uv_err_t UVDBFDPlugin::getObject(UVDData *data, const UVDRuntimeHints &hints, UVDObject **out)
{
	UVDBFDObject *ret = NULL;
	
	ret = new UVDBFDObject();
	uv_assert_ret(ret);
	uv_assert_err_ret(ret->init(data));
	
	uv_assert_ret(out);
	*out = ret;
	
	return UV_ERR_OK;
}

