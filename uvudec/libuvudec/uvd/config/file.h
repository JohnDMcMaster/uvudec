/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_CONFIG_FILE_H
#define UVD_CONFIG_FILE_H

#include "uvd/util/types.h"

class UVDConfig;
class UVDConfigFile
{
public:
	UVDConfigFile();
	~UVDConfigFile();

	static uv_err_t preprocess(const std::string in, std::string out);
	uv_err_t init(const std::string &filename);

public:
	std::string m_content;
};

class UVDConfigFileLoader
{
public:
	UVDConfigFileLoader();
	~UVDConfigFileLoader();
	uv_err_t init(UVDConfig *config);

	uv_err_t earlyArgParse(UVDConfig *);
	uv_err_t loadFile(const std::string &file);

public:
	std::set<UVDConfigFile *> m_configFiles;
};

#endif

