/*
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
*/

/*
Mozilla's Javascript engine
*/

#include "uvd_interpreter.h"

//This is the only supported method for now
#ifndef USING_PYTHON_EXEC
#define USING_PYTHON_EXEC
#endif

class UVDJavascriptSpidermonkeyInterpreter : public UVDInterpreter
{
public:
	UVDJavascriptSpidermonkeyInterpreter();
	virtual ~UVDJavascriptSpidermonkeyInterpreter();
	
	virtual uv_err_t init();

	virtual uv_err_t interpret(const UVDInterpreterExpression &exp, const UVDVariableMap &environment, std::string &sRet);
};
