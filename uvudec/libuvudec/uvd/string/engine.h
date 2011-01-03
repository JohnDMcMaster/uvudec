/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_STRING_ENGINE_H
#define UVD_STRING_ENGINE_H

/*
String analysis
	like the UNIX program "strings" that extracts text from binaries, but with varying algorithms
Probably will be extended to do other executabe analysis at some point
For now though, just concentrate on extracting strings
Depending on the analyzer, collect from string tables, raw binary parse algorithms, etc
Strings should not necessarily have to be C style strings, raw API return values are memory ranges to avoid this bias

Original string finded code added to a database in the analyzer
This code is currently inactive
Probably will be reactivated
Make assumptions for now that binary will not be modified
*/

#include <set>
#include "uvd/string/analyzer.h"
#include "uvd/string/string.h"

class UVD;
class UVDPlugin;
class UVDStringEngine
{
public:
	UVDStringEngine();
	uv_err_t init(UVD *uvd);
	
	//Called when a new plugin is loaded/activated
	uv_err_t pluginActivatedCallback(UVDPlugin *plugin);
	//Have all string analyzers do their magic
	//uv_err_t analyze();
	//Clear analyzer DB and re-populate it
	//FIXME: maybe need to register callback to probe new plugins as loaded?
	uv_err_t findAnalyzers();
	uv_err_t tryPlugin(UVDPlugin *plugin);
	//This could take up a lot of memory
	//should try to get some iterable version
	//Input vector will be cleared
	//FIXME: make this into a set
	uv_err_t getAllStrings(std::vector<UVDString> &out);

public:
	//There are no priority rules right now
	//Mostly I'm currently just trying to create an updgrade path to make the system more flexible later
	std::set<UVDStringsAnalyzer *> m_analyzers;
	UVD *m_uvd;
};

#endif

