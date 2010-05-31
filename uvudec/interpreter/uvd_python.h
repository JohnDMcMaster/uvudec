/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#ifdef USING_PYTHON

#include "config.h"
#include "uvd_interpreter.h"

//This is the only supported method for now
#ifndef USING_PYTHON_EXEC
#define USING_PYTHON_EXEC
#endif

class UVDPythonInterpreter : public UVDInterpreter
{
public:
	UVDPythonInterpreter();
	virtual ~UVDPythonInterpreter();
	
	virtual uv_err_t init();

	virtual uv_err_t interpret(const UVDInterpreterExpression &exp, const UVDVariableMap &environment, std::string &sRet);

	uv_err_t preparePythonProgram(const UVDInterpreterExpression &exp, const UVDVariableMap &environment, std::string &sPythonProgram);
#if defined(USING_PYTHON_API)
	uv_err_t UVDPythonInterpreter::execPythonLines(std::string pycode, std::string &sRet);
#endif //defined(USING_PYTHON_API)
};

#endif //USING_PYTHON
