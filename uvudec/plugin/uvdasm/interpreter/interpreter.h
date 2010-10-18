/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_INTERPRETER_H
#define UVD_INTERPRETER_H

#include <string>
#include "uvd/util/types.h"
#include "uvdasm/interpreter/instruction.h"

//Resultant address from a call routine
#define SCRIPT_KEY_CALL				"CALL_ADDRESS"
//Resultant address from a jump
#define SCRIPT_KEY_JUMP				"JUMP_ADDRESS"
//Resultant data from arithmetic
//This requires non-static analysis
//#define SCRIPT_KEY_ARITMETIC		"ARIMETIC"
//An alternative representation of an address
#define SCRIPT_KEY_SYMBOL			"ADDRESS_SYMBOL"

/*
A compiled expression to be passed into an interpreter
*/
class UVDInterpreter;
class UVDInterpreterExpression
{
public:
	UVDInterpreterExpression();
	virtual ~UVDInterpreterExpression();
	
	//This now gets compiled from the class, but add here for convience/abstraction reasons
	//This was done so as to not bother creating specialized base classes unless really needed
	virtual uv_err_t compile(const std::string &sExp);
	
public:	

	std::string m_sExpression;

	/*
	Set if a langauge specific compiled version exists
	Otherwise, this should be NULL and m_sExpression should be interpreted 
	*/
	void *m_compiled;
	UVDInterpreter *m_interpreter;
};

/*
Base class for interpretation engines
They should subclass from this to implement compiling and interpreting code

Ex:

UVDPythonInterpreter *interpreter = NULL;
UVDVariableMap environment;
UVDInterpreterExpression *exp;
std::string sRet;

UVDPythonInterpreter::getPythonInterpreter(&interpreter);
interpreter->getExpression(&exp);

exp->compile("print "Hello, world!");
interpreter->interpret(exp, environment, sRet);
printf("Script result: %s\n", sRet.c_str());
*/
class UVDInterpreter
{
public:
	UVDInterpreter();
	virtual ~UVDInterpreter();
	
	virtual uv_err_t init();
	
	//Default is to simply return it as a non-compilable expression (string only)
	virtual uv_err_t compile(const std::string &sExp, UVDInterpreterExpression *result);

	virtual uv_err_t interpret(const UVDInterpreterExpression &exp, const UVDVariableMap &environment, std::string &sRet) = 0;

	//Get a matching expression to use with this interpreter
	virtual uv_err_t getInterpreterExpression(UVDInterpreterExpression **expression);

	/*
	Convert a varient value to the scripting langauge native type string
	*/
	virtual uv_err_t varientToScriptValue(UVDVarient varient, std::string &value);

	//
	uv_err_t extractCallOutput(UVDVariableMap &variableMap, UVDInterpretedCall *out);
	//for unconditional and conditional branches/jumps
	uv_err_t extractBranchOutput(UVDVariableMap &variableMap, UVDInterpretedBranch *out);
};

#endif

