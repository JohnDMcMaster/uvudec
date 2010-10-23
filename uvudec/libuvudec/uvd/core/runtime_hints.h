/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_CORE_RUNTIME_HINTS_H
#define UVD_CORE_RUNTIME_HINTS_H

#include <string>

/*
Hints for loading a specific object file
This is not set in the main config because we can load many objects for a single project
	although it may be set in there as a default hint
*/
class UVDRuntimeHints
{
public:
	UVDRuntimeHints();
	~UVDRuntimeHints();
	
public:
	std::string m_object;
	std::string m_architecture;
};

#endif

