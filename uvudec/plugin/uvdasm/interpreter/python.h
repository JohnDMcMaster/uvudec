/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_PYTHON_H
#define UVD_PYTHON_H

#ifdef USING_PYTHON

#include "config.h"
#include "uvdasm/interpreter/interpreter.h"

class UVDPythonInterpreter : public UVDInterpreter
{
public:
	UVDPythonInterpreter();
	virtual ~UVDPythonInterpreter();
	virtual uv_err_t init();

	virtual uv_err_t interpret(const UVDInterpreterExpression &exp, const UVDVariableMap &environment, std::string &sRet) = 0;

	uv_err_t preparePythonProgram(const UVDInterpreterExpression &exp, const UVDVariableMap &environment, std::string &sPythonProgram);
};

#ifdef USING_PYTHON_API
class UVDPythonAPIInterpreter : public UVDPythonInterpreter
{
public:
	UVDPythonAPIInterpreter();
	virtual ~UVDPythonAPIInterpreter();	
	virtual uv_err_t init();

	virtual uv_err_t interpret(const UVDInterpreterExpression &exp, const UVDVariableMap &environment, std::string &sRet);

protected:
	uv_err_t execPythonLines(std::string pycode, std::string &sRet);
};
#endif

#ifdef USING_PYTHON_EXEC
class UVDPythonExecInterpreter : public UVDPythonInterpreter
{
public:
	UVDPythonExecInterpreter();
	virtual ~UVDPythonExecInterpreter();
	virtual uv_err_t init();

	virtual uv_err_t interpret(const UVDInterpreterExpression &exp, const UVDVariableMap &environment, std::string &sRet);
};
#endif

#endif //USING_PYTHON

#endif

