/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#ifndef UVD_PROJECT_H
#define UVD_PROJECT_H

#include "uvd.h"
#include "uvd_types.h"
#include <string>

class UVDProject
{
public:
	UVDProject();
	~UVDProject();

	uv_err_t doSave();
	uv_err_t init(int argc, char **argv);
	uv_err_t deinit();
	uv_err_t setFileName(const std::string &fileName);

public:
	UVD *m_uvd;	
	std::string m_canonicalProjectFileName;
};

#endif

