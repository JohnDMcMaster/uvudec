/*
Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#ifdef USING_PYTHON

#include <string>
#include <sstream>
#include <vector>
#include "uv_util.h"
#include "uvd_python.h"
#include "uvd_config.h"

UVDPythonInterpreter::UVDPythonInterpreter()
{
}

#ifdef USING_PYTHON_API
std::stringstream g_stringStream;

static PyObject *uvd_easyprint(PyObject *self, PyObject* args)
{
	const char *output = NULL;

	if( !PyArg_ParseTuple(args, "s", &output) )
	{
		printf_error("PYTHON ERROR: uvd_easyprint given non-string\n");
		return NULL;
	}
	g_stringStream << output; 	

	Py_RETURN_NONE;
}

static PyMethodDef uvd_easyoutput_methods[] = {
	{"uvd_easyprint",		uvd_easyprint,	METH_VARARGS,
	 "UVD Python -> C++ output helper."},
	{NULL,		NULL}		/* sentinel */
};

void initeasy(void)
{
	PyImport_AddModule("uvd_easyprint");
	Py_InitModule("uvd_easyprint", uvd_easyoutput_methods);
}

#endif //USING_PYTHON_API

UVDPythonInterpreter::~UVDPythonInterpreter()
{
#ifdef USING_PYTHON_API
	Py_Finalize();
#endif 
}

uv_err_t UVDPythonInterpreter::init()
{
	uv_err_t rc = UV_ERR_GENERAL;

	uv_assert_err_ret(UVDInterpreter::init());

#ifdef USING_PYTHON_API
	Py_Initialize();
	if( !Py_IsInitialized() )
	{
		printf_error("Failed to initialize python!");
		return UV_DEBUG(rc);
	}

	initeasy();
#endif 
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

#elif defined(USING_PYTHON_API)

uv_err_t UVDPythonInterpreter::execPythonLines(std::string pycode, std::string &sRet)
{
	uv_err_t rc = UV_ERR_GENERAL;
	PyObject *m = NULL, *d = NULL, *v = NULL;

	pycode = "import uvd_easyprint\n" + pycode;

	m = PyImport_AddModule("__main__");
	if( m == NULL )
	{
		printf_error("Python exec setup borked\n");
		goto error;
	}
	d = PyModule_GetDict(m);

	v = PyRun_String(pycode.c_str(), Py_file_input, d, d);
	if( v == NULL )
	{
		printf_error("python run failed\n");
		PyErr_Print();
		goto error;
	}
	Py_DECREF(v);
	
	sRet = g_stringStream.str();
	g_stringStream.str("");

	rc = UV_ERR_OK;

error:
	return rc;
}

uv_err_t UVDPythonInterpreter::interpret(const UVDInterpreterExpression &exp, const UVDVariableMap &environment, std::string &sRet)
{
	uv_err_t rc = UV_ERR_GENERAL;
	std::string sPythonProgram;

	UV_ENTER();

	sPythonProgram = preparePythonProgram(exp, environment);

	rc = execPythonLines(sPythonProgram, sRet);

	return UV_DEBUG(UV_ERR_OK);
}
#else
#error Need to specify a technique to call Python code
#endif

#endif //USING_PYTHON
