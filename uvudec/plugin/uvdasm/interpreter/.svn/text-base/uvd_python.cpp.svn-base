/*
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
*/

#include <string>
#include <vector>
#include "uv_util.h"
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
	uv_assert_err_ret(UVDInterpreter::init());
	return UV_ERR_OK;
}

#if defined(USING_PYTHON_EXEC)
uv_err_t UVDPythonInterpreter::interpret(const UVDInterpreterExpression &exp, const UVDVariableMap &environment, std::string &sRet)
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


	UV_ENTER();

	
	if( pythonFile.empty() )
	{
		uv_assert_err(getTempFile(pythonFile));
	}

	//Execut an expression
	//args.push_back("-c");
	args.push_back(pythonFile);


	/*
	Collect environment
	*/
	
	//Instruction specific environment
	for( UVDVariableMap::const_iterator iter = environment.begin(); iter != environment.end(); ++iter )
	{
		std::string key = (*iter).first;
		std::string value;
		UVDVarient varient = (*iter).second;
		
		uv_assert_err(varientToScriptValue(varient, value));
		sPythonProgram += key  + "=" + value + "\n";		
	}
	
	/*
	Utility functions
	*/
	
	//Call results should be printed so that they can be retrieved
	sPythonProgram +=
		"def CALL(address):\n"
			"\tprint '" SCRIPT_KEY_CALL "=%d' % address\n"
		"\n";
	
	//Jump results should be printed so that they can be retrieved
	sPythonProgram +=
		"def GOTO(address):\n"
			"\tprint '" SCRIPT_KEY_JUMP "=%d' % address\n"
		"\n";

	/*
	Main expression
	*/
	sPythonProgram += exp.m_sExpression + "\n";	

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
#elif defined(USING_PYTHON_API)
//I wasn't too impressed with the API
//It lacked error checking and didn't seem to work well...a bad combination
#error Python API not currently supported
#else
#error Need to specify a technique to call Python code
#endif


