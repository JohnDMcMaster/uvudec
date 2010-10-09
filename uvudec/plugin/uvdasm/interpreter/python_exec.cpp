/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifdef USING_PYTHON_EXEC

#include <string>
#include <vector>
#include "uvd_util.h"
#include "uvdasm/interpreter/python.h"
#include "uvd_config.h"

UVDPythonExecInterpreter::UVDPythonExecInterpreter()
{
}

UVDPythonExecInterpreter::~UVDPythonExecInterpreter()
{
}

uv_err_t UVDPythonExecInterpreter::init()
{
	uv_assert_err_ret(UVDPythonInterpreter::init());
	return UV_ERR_OK;
}

uv_err_t UVDPythonExecInterpreter::interpret(const UVDInterpreterExpression &exp, const UVDVariableMap &environment, std::string &sRet)
{
	/*
	Do python hack for now
	[mcmaster@localhost buffer]$ echo 'myvar=4; other=myvar+20; print other' |python -
	24
	[mcmaster@localhost uv_udec]$ python -c 'print "test";'
	test
	*/
	static std::string pythonFile;
	
	uv_err_t rc = UV_ERR_GENERAL;
	std::vector<std::string> args;
	std::string sPythonProgram;
	int iRet = 0;
	std::string sErr;

	if( pythonFile.empty() )
	{
		uv_assert_err(getTempFile(pythonFile));
	}

	//Execut an expression
	//args.push_back("-c");
	args.push_back(pythonFile);

	uv_assert_err_ret(preparePythonProgram(exp, environment, sPythonProgram));


	if( UV_FAILED(writeFile(pythonFile, sPythonProgram)) )
	{
		UV_DEBUG(rc);
		goto error;
	}	
	
	//args.push_back(sPythonProgram);
	
	/*
	Run it!
	*/
	uv_assert_err(executeToText("python",
			args,
			iRet,
			&sRet,
			&sErr));

	if( iRet )
	{
		UV_DEBUG(rc);
		goto error;
	}

	printf_debug("Raw python result: %s\n", sRet.c_str());
	if( !sErr.empty() )
	{
		printf_debug("Python program:\n%s\n\n", sPythonProgram.c_str());
		printf_debug("Python error: %s\n", sErr.c_str());
	}
		
	rc = UV_ERR_OK;
	
error:
	UV_DEBUG(deleteFile(pythonFile));
	return UV_DEBUG(rc);
}

#endif //USING_PYTHON_EXEC

