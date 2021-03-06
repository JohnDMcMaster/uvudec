/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_OPTIONS_H
#define UVD_OPTIONS_H

#include "uvd/util/types.h"

class UVDOptions
{
public:
	UVDOptions();
	
	uv_err_t usage();

	void addNote(const std::string &note);
	void addCopyright(const std::string &copyright);
	
	//Dump the options for usage/copyright
	void print();

public:
	std::vector<std::string> m_copyrights;
	std::vector<std::string> m_notes;
};

#endif
