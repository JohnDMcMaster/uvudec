/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#ifdef USING_PYTHON_API

#include <sstream>
#include <string>
#include <vector>
#include "python2.6/Python.h"
#include "uvd_util.h"
#include "uvd_python.h"
#include "uvd_config.h"

UVDPythonAPIInterpreter::UVDPythonAPIInterpreter()
{
}

static std::stringstream g_stringStream;

static PyObject *uvd_print(PyObject *self, PyObject* args)
{
	const char *output = NULL;

	if( !PyArg_ParseTuple(args, "s", &output) )
	{
		printf_error("uvd_print given non-string\n");
		return NULL;
	}
	g_stringStream << output; 	

	Py_RETURN_NONE;
}

static PyMethodDef uvd_easyoutput_methods[] = {
	{"uvd_print",		uvd_print,	METH_VARARGS,
	 "UVD Python -> C++ output helper."},
	{NULL,		NULL}		/* sentinel */
};

void initeasy(void)
{
	PyImport_AddModule("uvd_print");
	Py_InitModule("uvd_print", uvd_easyoutput_methods);
}

UVDPythonAPIInterpreter::~UVDPythonAPIInterpreter()
{
	Py_Finalize();
}

uv_err_t UVDPythonAPIInterpreter::init()
{
	uv_assert_err_ret(UVDPythonInterpreter::init());

	Py_Initialize();
	if( !Py_IsInitialized() )
	{
		printf_error("Failed to initialize python!");
		return UV_DEBUG(UV_ERR_GENERAL);
	}

	initeasy();
	return UV_ERR_OK;
}

uv_err_t UVDPythonAPIInterpreter::execPythonLines(std::string pycode, std::string &sRet)
{
	PyObject *module = NULL;
	PyObject *d = NULL;
	PyObject *v = NULL;

	pycode = "from uvd_print import uvd_print\n" + pycode;

	module = PyImport_AddModule("__main__");
	if( module == NULL )
	{
		printf_error("Python exec setup borked\n");
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	d = PyModule_GetDict(module);

	printf_debug("code:\n\n%s\n\n", pycode.c_str());

	v = PyRun_String(pycode.c_str(), Py_file_input, d, d);
	if( v == NULL )
	{
		printf_error("python run failed\n");
		PyErr_Print();
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	Py_DECREF(v);
	
	sRet = g_stringStream.str();
	g_stringStream.str("");

	return UV_ERR_OK;
}

uv_err_t UVDPythonAPIInterpreter::interpret(const UVDInterpreterExpression &exp, const UVDVariableMap &environment, std::string &sRet)
{
	std::string sPythonProgram;

	uv_assert_err_ret(preparePythonProgram(exp, environment, sPythonProgram));
	uv_assert_err_ret(execPythonLines(sPythonProgram, sRet));

	return UV_ERR_OK;
}

#endif //USING_PYTHON_API

