/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the GPL V3 or later, see COPYING for details
*/

#include "uvd/init/arg_property.h"
#include "uvd/init/config.h"
#include "uvd/project/file_extensions.h"
#include "uvd/init/init.h"
#include "uvd/util/util.h"
#include "GUI/project.h"
#include "GUI/format.h"
#include <jansson.h>

class UVDProjectWriter
{
public:
	UVDProjectWriter();
	~UVDProjectWriter();
	uv_err_t init(UVDProject *project);

	uv_err_t doSave(const std::string &fileName);

public:
	UVDProject *m_project;
};

/*
UVDProject
*/

UVDProject::UVDProject()
{
	m_uvd = NULL;
	m_symbolInFocus = NULL;
}

UVDProject::~UVDProject()
{
}

uv_err_t UVDProject::doSave()
{
	UVDProjectWriter writer;

	uv_assert_err_ret(writer.init(this));
	uv_assert_ret(!m_canonicalProjectFileName.empty());
	uv_assert_err_ret(writer.doSave(m_canonicalProjectFileName));

	return UV_ERR_OK;
}

uv_err_t UVDProject::setFileName(const std::string &fileName)
{
	std::string temp;

	uv_assert_err_ret(getCannonicalFileName(fileName, temp));
	uv_assert_err_ret(weaklyEnsurePathEndsWithExtension(temp, UVD_EXTENSION_PROJECT, m_canonicalProjectFileName));

	return UV_ERR_OK;
}

uv_err_t UVDProject::init(int argc, char **argv)
{
	UVDConfig *config = NULL;
	uv_err_t parseMainRc = UV_ERR_GENERAL;
	
	//Early library initialization.  Logging and arg parsing structures
	config = g_config;
	uv_assert_ret(config);
	
	//Grab our command line options
	parseMainRc = config->parseMain(argc, argv);
	uv_assert_err_ret(parseMainRc);
	//No weird actions should be specified
	uv_assert_ret(parseMainRc == UV_ERR_OK);
	
	return UV_ERR_OK;
}

uv_err_t UVDProject::deinit()
{
	uv_assert_err_ret(UVDDeinit());

	return UV_ERR_OK;
}

UVDGUIFormat *UVDProject::getFormat()
{
	//TODO: add an assert here using C++ RTTI
	return (UVDGUIFormat *)m_uvd->m_format;
}

/*
UVDProjectWriter
*/

UVDProjectWriter::UVDProjectWriter()
{
}

UVDProjectWriter::~UVDProjectWriter()
{
}

uv_err_t UVDProjectWriter::init(UVDProject *project)
{
	m_project = project;
	return UV_ERR_OK;
}

uv_err_t UVDProjectWriter::doSave(const std::string &canonicalProjectFile)
{
	/*
	{
		"binaryFileName": "candela.bin",
		"binaryArchitectureFileName": "8051/8051.op"
	}


	{	
		[
			{
				"id": "<the commit ID>",
				"message": "<the commit message>",
				<more fields, not important to this tutorial>
			},
			{
				"id": "<the commit ID>",
				"message": "<the commit message>",
				<more fields, not important to this tutorial>
			},
			<more commits...>
		]
	}
	*/
	
	/*
	TODO: convert this into selected values from the property structure?
	*/
	
	std::string out;
	
	uv_assert_ret(g_config);
	
	out += "{";
	out += "\"" UVD_PROP_TARGET_FILE "\": \"" + g_config->m_targetFileName + "\",";
	out += "\"" UVD_PROP_ARCH_FILE "\": \"" + g_config->m_architectureFileName + "\"";
	out += "}";
	
printf("save target: %s\n", canonicalProjectFile.c_str());
printf("saving: %s\n", out.c_str());
	
	uv_assert_err_ret(writeFile(canonicalProjectFile, out));

	return UV_ERR_OK;
}

