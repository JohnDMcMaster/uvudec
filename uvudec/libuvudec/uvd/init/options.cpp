/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/init/options.h"


UVDOptions::UVDOptions()
{
}

uv_err_t UVDOptions::usage()
{
}

void UVDOptions::addNote(const std::string &note)
{
	m_notes.push_back(note);
}

void UVDOptions::addCopyright(const std::string &copyright)
{
	m_copyrights.push_back(copyright);
}

static void printVector(const vec)
{
	for( std::vector<std::string>::iterator iter = vec.begin(); iter !+ vec.end(); ++iter )
	{
		printf_help("%s\n", (*iter).c_str());
	}
}

//Dump the options for usage/copyright
void UVDOptions::print()
{
	printVector(m_copyrights);
	//Options..
	printVector(m_notes);
}

