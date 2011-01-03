/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

/*
TODO FIXME XXX WARNING XXX FIXME TODO
We do some sketchy stuff with constness as a hack to get some reasonable default canLoad behaviors
I'll probably go to programmer hell for it, but it provides some reasonable default behavior
Its based on the possibly assumption that initialization routines won't modify the object data
Since the same init routines are used for both, it doesn't quite work out nicely...

In any case, its highly reccomend you implement one if you implement the other
*/

#include "uvd/config.h"
#include "uvd/architecture/architecture.h"
#include "uvd/config/config.h"
#include "uvd/core/uvd.h"
#include "uvd/object/object.h"
#include "uvd/plugin/plugin.h"
#include <dlfcn.h>
#include <typeinfo>

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

uv_err_t UVDPlugin::getDataDir(std::string &out)
{
	UVDConfig *config = NULL;
	
	config = g_config;
#ifdef UVD_ENABLE_DEVELOPMENT
	//Data is grouped into plugin dirs for development
	out = UVDSprintf("%s/plugin/%s/data", config->m_installDir.c_str(), getName().c_str());
#else
	//...but should be moved into data dir for installation
	out = UVDSprintf("%s/data/plugin/%s", config->m_installDir.c_str(), getName().c_str());
#endif
	return UV_ERR_OK;
}

uv_err_t UVDPlugin::onUVDInit()
{
	return UV_ERR_OK;
}

uv_err_t UVDPlugin::onUVDDeinit()
{
	return UV_ERR_OK;
}

std::string UVDPlugin::getName()
{
	std::string ret;
	
	if( UV_FAILED(getName(ret)) )
	{
		return "";
	}
	return ret;
}

std::string UVDPlugin::getDescription()
{
	std::string ret;
	
	if( UV_FAILED(getDescription(ret)) )
	{
		return "";
	}
	return ret;
}

std::string UVDPlugin::getAuthor()
{
	std::string ret;
	
	if( UV_FAILED(getAuthor(ret)) )
	{
		return "";
	}
	return ret;
}

UVDVersion UVDPlugin::getVersion()
{
	UVDVersion ret;
	
	if( UV_FAILED(getVersion(ret)) )
	{
		return UVDVersion();
	}
	return ret;
}

uv_err_t UVDPlugin::getDependencies(PluginDependencies &out)
{
	out.clear();
	return UV_ERR_OK;
}

uv_err_t UVDPlugin::outputHeader(std::vector<std::string> &lines)
{
	return UV_ERR_OK;
}

uv_err_t UVDPlugin::canLoadObject(const UVDData *data, const UVDRuntimeHints &hints, uvd_priority_t *confidence)
{
	/*
	Crude default handler that literally tries a full load and then aborts it
	*/
	UVDObject *object = NULL;
	
	uv_assert_ret(confidence);
	
	UVD_POKE(data);
	if( UV_SUCCEEDED(loadObject((UVDData *)data, hints, &object)) )
	{
		printf_plugin_debug("%s: canGetObject acceptable match by getObject\n", getName().c_str());
		UVD_POKE(data);
		//Just in case...
		object->m_data = NULL;
		delete object;
		UVD_POKE(data);
		*confidence = UVD_MATCH_ACCEPTABLE;
		return UV_ERR_OK;
	}
	else
	{
		UVD_POKE(data);
		return UV_ERR_NOTSUPPORTED;
	}
}

uv_err_t UVDPlugin::loadObject(UVDData *data, const UVDRuntimeHints &hints, UVDObject **out)
{
	return UV_ERR_NOTSUPPORTED;
}

uv_err_t UVDPlugin::canGetArchitecture(const UVDObject *object, const UVDRuntimeHints &hints, uvd_priority_t *confidence)
{
	/*
	Crude default handler that literally tries a full load and then aborts it
	*/
	UVDArchitecture *architecture = NULL;
	
	uv_assert_ret(confidence);
	
	if( UV_SUCCEEDED(getArchitecture((UVDObject *)object, hints, &architecture)) )
	{
		printf_plugin_debug("%s: canGetArchitecture acceptable match by getArchitecture\n", getName().c_str());
		delete architecture;
		*confidence = UVD_MATCH_ACCEPTABLE;
		return UV_ERR_OK;
	}
	else
	{
		return UV_ERR_NOTSUPPORTED;
	}
}

uv_err_t UVDPlugin::getArchitecture(UVDObject *object, const UVDRuntimeHints &hints, UVDArchitecture **out)
{
	return UV_ERR_NOTSUPPORTED;
}

uv_err_t UVDPlugin::getSupportedArchitectures(std::set<std::string> &architectures)
{
	return UV_ERR_NOTSUPPORTED;
}

uv_err_t UVDPlugin::getStringsAnalyzer(UVDStringsAnalyzer **out)
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

