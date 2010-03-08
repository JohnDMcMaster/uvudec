/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#ifndef UVD_OPTIONS_H
#define UVD_OPTIONS_H

#include "uvd_types.h"

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
