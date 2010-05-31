/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#pragma once

#include "uvd_types.h"
#include "interpreter/uvd_interpreter.h"

#include <string>

/*
A configuration specific expression
Do not confuse with UVDInterpreterExpression which is internal to a specific Language
Translates config file syntax into that required by the actual interpreter used
*/
class UVDConfigExpressionInterpreter;
class UVDConfigExpression
{
public:
	UVDConfigExpression();
	virtual ~UVDConfigExpression();
	uv_err_t deinit();
	//UVDConfigExpression(std::string &sExp);
	/*
	Translate a config expression into a form suitible for internal use
	*/
	virtual uv_err_t compile(const std::string &sExpression);

public:
	//Original config directive
	std::string m_sExpression;
	//The real Language representation
	UVDInterpreterExpression *m_interpreterExpression;
	//So we can get the core Language we are compiling for and other settings
	UVDConfigExpressionInterpreter *m_configExpressionInterpreter;
};

class UVDConfigExpressionInterpreter
{
public:
	UVDConfigExpressionInterpreter();
	//UVDConfigExpressionInterpreter(UVD *uvd);
	virtual ~UVDConfigExpressionInterpreter();

	virtual uv_err_t init();
	virtual uv_err_t deinit();

	//Compile an expression.  Can make repeated use faster.
	virtual uv_err_t compile(const std::string &sExp, UVDConfigExpression *result);

	uv_err_t interpret(const std::string &sExp, std::string &result);
	uv_err_t interpret(const std::string &sExp, int &result);
	uv_err_t interpret(const UVDConfigExpression *exp, std::string &result);
	uv_err_t interpret(const UVDConfigExpression *exp, int &result);
	//Core interpret function
	virtual uv_err_t interpret(const UVDConfigExpression *exp, const UVDVariableMap &environment, std::string &sRet);

	/*
	Reccomended form
	Returned values should be key/value pairs
	*/
	uv_err_t interpretKeyed(const UVDConfigExpression *exp, UVDVariableMap &result);
	uv_err_t interpretKeyed(const std::string &exp, const UVDVariableMap &environment, UVDVariableMap &result);
	uv_err_t interpretKeyed(const UVDConfigExpression *exp, const UVDVariableMap &environment, UVDVariableMap &result);

	void updateCache(uint32_t address, const UVDVariableMap &result);
	uv_err_t readCache(uint32_t address, UVDVariableMap &result);
	
	//Get the "best" config interpreter availible
	static uv_err_t getConfigExpressionInterpreter(UVDConfigExpressionInterpreter **interpreter);
	//Get a blank config expression usable with the interpreter fetched from previous
	virtual uv_err_t getConfigExpression(UVDConfigExpression **expression);

public:
	//UVD *m_uvd;
	//Scripting interpreter used to process config directives
	UVDInterpreter *m_interpreter;

	/*
	See also analyzer's cache for other data
	
	Cache of scripted data analysis
	-Speeds up analysis (removed duplicate processing)
	-Allows retrieval of some information during print
	-Allows multi-pass analysis
	*/
	std::map<uint32_t, UVDVariableMap> m_interpretCache;
};

