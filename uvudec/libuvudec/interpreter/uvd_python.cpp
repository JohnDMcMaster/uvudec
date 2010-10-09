/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifdef USING_PYTHON

#include <string>
#include <sstream>
#include <vector>
#include "uvd.h"
#include "uvd_util.h"
#include "uvd_python.h"
#include "uvd_config.h"
#include "uvd_config_symbol.h"
#include "uvd_address.h"
#include "core/runtime.h"

UVDPythonInterpreter::UVDPythonInterpreter()
{
}

UVDPythonInterpreter::~UVDPythonInterpreter()
{
}

uv_err_t UVDPythonInterpreter::init()
{
	uv_err_t rc = UV_ERR_GENERAL;

	uv_assert_err_ret(UVDInterpreter::init());

	rc = UV_ERR_OK;
	return UV_DEBUG(rc);
}

uv_err_t UVDPythonInterpreter::preparePythonProgram(const UVDInterpreterExpression &exp, const UVDVariableMap &environment, std::string &sPythonProgram)
{
	//Make sure clear
	sPythonProgram = "";

	/*
	Collect environment
	*/
	
	//Instruction specific environment
	for( UVDVariableMap::const_iterator iter = environment.begin(); iter != environment.end(); ++iter )
	{
		std::string key = (*iter).first;
		std::string value;
		UVDVarient varient = (*iter).second;
		
		uv_assert_err_ret(varientToScriptValue(varient, value));
		sPythonProgram += key  + "=" + value + "\n";		
	}
	
	/*
	Utility functions
	*/
	
	//Call results should be printed so that they can be retrieved
	sPythonProgram +=
		"def CALL(address):\n"
			"\tuvd_print('" SCRIPT_KEY_CALL "=%d' % address)\n"
		"\n";
	
	//Jump results should be printed so that they can be retrieved
	sPythonProgram +=
		"def GOTO(address):\n"
			"\tuvd_print('" SCRIPT_KEY_JUMP "=%d' % address)\n"
		"\n";

	/*
	Address space mappings
	Just return the value as is, its a tag used for printing prefixes and such, does not affect function input...currently
	We will probably eventually make it map to the most basic areas
	*/
#if 0
	for( UVDSymbolMap::SymbolMapMap::iterator iter = g_uvd->m_architecture->m_symMap->m_map.begin(); 
			iter != g_uvd->m_architecture->m_symMap->m_map.end(); ++iter )
	{
		//std::string, UVDSymbol *
		UVDAddressSpace *memoryShared = (UVDAddressSpace *)(*iter).second;	
		std::string addressSpaceName;
		
		uv_assert_ret(memoryShared);
		//ug in theory these should be synchronized, but seems sometimes memoryShared->m_name isn't set
		//printf("first name: %s\n", (*iter).first.c_str());
		//printf("obj name: %s\n", memoryShared->m_name.c_str());
		addressSpaceName = (*iter).first;
#endif
	std::vector<std::string> addressSpaceNames;
	g_uvd->m_runtime->m_architecture->getAddresssSpaceNames(addressSpaceNames);
	for( std::vector<std::string>::iterator iter = addressSpaceNames.begin(); iter != addressSpaceNames.end(); ++iter )
	{
		std::string addressSpaceName = *iter;
		
		sPythonProgram +=
			std::string("def ") + addressSpaceName + "(address):\n"
				"\treturn address\n"
			"\n";
	}
	
	/*
	Main expression
	*/
	sPythonProgram += exp.m_sExpression + "\n";	

	//printf("going to exec\n%s\n", sPythonProgram.c_str());

	return UV_ERR_OK;
}

#endif //USING_PYTHON

