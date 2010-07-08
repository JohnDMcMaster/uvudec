/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#ifdef USING_PYTHON

#include <string>
#include <sstream>
#include <vector>
#include "uvd_util.h"
#include "uvd_python.h"
#include "uvd_config.h"

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
	Main expression
	*/
	sPythonProgram += exp.m_sExpression + "\n";	

	return UV_ERR_OK;
}

#endif //USING_PYTHON

