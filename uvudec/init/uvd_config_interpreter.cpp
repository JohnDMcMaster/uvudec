/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#include "uvd_util.h"
#include "uvd_config.h"
#include "uvd_config_interpreter.h"
#include "uvd_language.h"


UVDConfigExpression::UVDConfigExpression()
{
	m_interpreterExpression = NULL;
	m_configExpressionInterpreter = NULL;
}

UVDConfigExpression::~UVDConfigExpression()
{
}

/*
UVDConfigExpression::UVDConfigExpression(std::string &sExpression)
{
	compile(sExpression);
}
*/

uv_err_t UVDConfigExpression::compile(const std::string &sExpression)
{
	uv_assert_ret(m_configExpressionInterpreter);
	return UV_DEBUG(m_configExpressionInterpreter->compile(sExpression, this));
}

/************
Language specific config interpreters
These do fixup on config file to enable it to be executed in real languages
************/


#ifdef USING_PYTHON

#include "interpreter/uvd_python.h"

class UVDPythonConfigExpressionInterpreter : public UVDConfigExpressionInterpreter
{
public:
	UVDPythonConfigExpressionInterpreter();
	~UVDPythonConfigExpressionInterpreter();

	uv_err_t init();
	
public:
};

UVDPythonConfigExpressionInterpreter::UVDPythonConfigExpressionInterpreter()
{
}

UVDPythonConfigExpressionInterpreter::~UVDPythonConfigExpressionInterpreter()
{
}

uv_err_t UVDPythonConfigExpressionInterpreter::init()
{
	uv_assert_err_ret(UVDConfigExpressionInterpreter::init());
	m_interpreter = new UVDPythonInterpreter();
	uv_assert_ret(m_interpreter);
	return UV_ERR_OK;
}
#endif //USING_PYTHON


#ifdef USING_LUA

#include "interpreter/uvd_lua.h"

class UVDLuaConfigExpressionInterpreter : public UVDConfigExpressionInterpreter
{
public:
	UVDLuaConfigExpressionInterpreter();
	~UVDLuaConfigExpressionInterpreter();

	uv_err_t init();
	
public:
};

UVDLuaConfigExpressionInterpreter::UVDLuaConfigExpressionInterpreter()
{
}

UVDLuaConfigExpressionInterpreter::~UVDLuaConfigExpressionInterpreter()
{
}

uv_err_t UVDLuaConfigExpressionInterpreter::init()
{
	uv_assert_err_ret(UVDConfigExpressionInterpreter::init());
	m_interpreter = new UVDLuaInterpreter();
	uv_assert_ret(m_interpreter);
	uv_assert_err_ret(m_interpreter->init());
	return UV_ERR_OK;
}
#endif //USING_LUA


/*
Generic template implementation for basic interpreter functionality
*/
template <typename T>
class UVDConfigExpressionInterpreterTemplate : public UVDConfigExpressionInterpreter
{
public:
	UVDConfigExpressionInterpreterTemplate();
	~UVDConfigExpressionInterpreterTemplate();

	uv_err_t init();
	
public:
};

template <typename T>
UVDConfigExpressionInterpreterTemplate<T>::UVDConfigExpressionInterpreterTemplate()
{
}

template <typename T>
UVDConfigExpressionInterpreterTemplate<T>::~UVDConfigExpressionInterpreterTemplate()
{
}

template <typename T>
uv_err_t UVDConfigExpressionInterpreterTemplate<T>::init()
{
	uv_assert_err_ret(UVDConfigExpressionInterpreter::init());
	m_interpreter = new T();
	uv_assert_ret(m_interpreter);
	uv_assert_err_ret(m_interpreter->init());
	return UV_ERR_OK;
}

#ifdef USING_JAVASCRIPT
#include "interpreter/uvd_javascript.h"
typedef UVDConfigExpressionInterpreterTemplate<UVDJavascriptInterpreter> UVDJavascriptConfigExpressionInterpreter;
#endif //USING_JAVASCRIPT

/******
base class interpreter
******/

UVDConfigExpressionInterpreter::UVDConfigExpressionInterpreter()
{
	//m_uvd = NULL;
}

/*
UVDConfigExpressionInterpreter::UVDConfigExpressionInterpreter(UVD *uvd)
{
	m_uvd = uvd;
}
*/

UVDConfigExpressionInterpreter::~UVDConfigExpressionInterpreter()
{
}

uv_err_t UVDConfigExpressionInterpreter::init()
{
	return UV_ERR_OK;
}

uv_err_t UVDConfigExpressionInterpreter::compile(const std::string &sExp, UVDConfigExpression *result)
{
	/*
	Generic compile that works for most langs
	Simply remove the register prefixes
	
	Two pass compilation here
	-Translation into native language format
	-Compiling native language to fast version
	*/
	
	std::string sExpCompiled;
	std::string::size_type pos = 0;
	
	uv_assert_ret(result);
	
	*result = UVDConfigExpression();
	result->m_configExpressionInterpreter = this;
	m_interpreter->getInterpreterExpression(&result->m_interpreterExpression);
	uv_assert_ret(result->m_interpreterExpression);

	sExpCompiled = sExp;
	//Convert registers to variables
	//Should these be marked with an _ or something?
	while( (pos = sExpCompiled.find('%')) != std::string::npos )
	{
		sExpCompiled.erase(pos, 1);
	}
	
	//Store final expression and make this into a fast version, if possible
	uv_assert_err_ret(result->m_interpreterExpression->compile(sExpCompiled));

	return UV_ERR_OK;
}

uv_err_t UVDConfigExpressionInterpreter::interpret(const std::string &sExp, std::string &sRet)
{
	UVDConfigExpression exp;

	uv_assert_err_ret(exp.compile(sExp));
	return UV_DEBUG(interpret(&exp, sRet));
}

uv_err_t UVDConfigExpressionInterpreter::interpret(const std::string &sExp, int &iRet)
{
	UVDConfigExpression exp;

	uv_assert_err_ret(exp.compile(sExp));
	return UV_DEBUG(interpret(&exp, iRet));
}

uv_err_t UVDConfigExpressionInterpreter::interpret(const UVDConfigExpression *configExpression, const UVDVariableMap &environment, std::string &sRet)
{
	uv_assert_ret(configExpression);
	uv_assert_err_ret(m_interpreter->interpret(*(configExpression->m_interpreterExpression), environment, sRet));
	return UV_ERR_OK;
}

uv_err_t UVDConfigExpressionInterpreter::interpretKeyed(const UVDConfigExpression *exp, UVDVariableMap &result)
{
	UVDVariableMap environment;
	return UV_DEBUG(interpretKeyed(exp, environment, result));
}

uv_err_t UVDConfigExpressionInterpreter::interpretKeyed(const std::string &sExpression, const UVDVariableMap &environment, UVDVariableMap &result)
{
	static UVDConfigExpression *configExpression = NULL;
	if( !configExpression )
	{
		uv_assert_err_ret(getConfigExpression(&configExpression));
	}
	uv_assert_ret(configExpression);

	//UVDConfigExpression configExpression;

	uv_assert_err_ret(configExpression->compile(sExpression));
	return UV_DEBUG(interpretKeyed(configExpression, environment, result));
}

uv_err_t UVDConfigExpressionInterpreter::interpretKeyed(const UVDConfigExpression *exp, const UVDVariableMap &environment, UVDVariableMap &result)
{
	uv_err_t rc = UV_ERR_GENERAL;
	std::string sRes;
	std::vector<std::string> lines;
	
	UV_ENTER();
	
	uv_assert_ret(exp);

	uv_assert_err(interpret(exp, environment, sRes));
	result.clear();	
	
	//Split key/value pairs
	lines = split(sRes, '\n', false);
	
	for( std::vector<std::string>::size_type i = 0; i < lines.size(); ++i )
	{
		std::string line = lines[i];
		std::string key, value;
		
		//Assume all strings for now
		uv_assert_err(uvdParseLine(line, key, value));
		result[key] = UVDVarient(value);
	}

	
	rc = UV_ERR_OK;
	
error:
	return UV_ERR(rc);
}

uv_err_t UVDConfigExpressionInterpreter::interpret(const UVDConfigExpression *exp, std::string &sRet)
{
	UVDVariableMap environment;
	return UV_DEBUG(interpret(exp, environment, sRet));
}

uv_err_t UVDConfigExpressionInterpreter::getConfigExpressionInterpreter(UVDConfigExpressionInterpreter **interpreter_in)
{
	UVDConfigExpressionInterpreter *interpreter = NULL;
	
	//Select the embedded interpreter we are going to use
	int selectedInterpreter = 0;
	
	uv_assert_ret(g_config);
	selectedInterpreter = g_config->m_configInterpreterLanguage;
	switch( selectedInterpreter )
	{
#ifdef USING_PYTHON
	case UVD_LANGUAGE_PYTHON:
		printf_debug_level(UVD_DEBUG_SUMMARY, "Chose python interpreter\n");
		interpreter = new UVDPythonConfigExpressionInterpreter();
		break;
#endif //USING_PYTHON
#ifdef USING_LUA
	case UVD_LANGUAGE_LUA:
		printf_debug_level(UVD_DEBUG_SUMMARY, "Chose Lua interpreter\n");
		interpreter = new UVDLuaConfigExpressionInterpreter();
		break;
#endif //USING_LUA
#ifdef USING_JAVASCRIPT
	case UVD_LANGUAGE_JAVASCRIPT:
		printf_debug_level(UVD_DEBUG_SUMMARY, "Chose JavaScript interpreter\n");
		interpreter = new UVDJavascriptConfigExpressionInterpreter();
		break;
#endif //USING_JAVASCRIPT
	default:
		printf("Invalid config interpreter: 0x%.8X\n", selectedInterpreter);
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	
	uv_assert_ret(interpreter);
	uv_assert_err_ret(interpreter->init());
	uv_assert_ret(interpreter->m_interpreter);
	
	uv_assert_ret(interpreter_in);
	*interpreter_in = interpreter;
	
	return UV_ERR_OK;
}

uv_err_t UVDConfigExpressionInterpreter::getConfigExpression(UVDConfigExpression **expression_in)
{
	/*
	This use to return a language specific config expression
	The language specific code (compile) was moved to the interpreter to ease development
	*/
	
	UVDConfigExpression *expression = NULL;
	
	expression = new UVDConfigExpression();
	uv_assert_ret(expression);
	
	expression->m_configExpressionInterpreter = this;
	
	//Setup the real language compilation
	uv_assert_err_ret(m_interpreter->getInterpreterExpression(&expression->m_interpreterExpression));
	uv_assert_ret(expression->m_interpreterExpression);
	
	//All okay, assign to output
	uv_assert_ret(expression_in);
	*expression_in = expression;
	
	return UV_ERR_OK;
}

uv_err_t UVDConfigExpressionInterpreter::interpret(const UVDConfigExpression *exp, int &result)
{
	uv_err_t rc = UV_ERR_GENERAL;
	std::string sTemp;
	
	rc = interpret(exp, sTemp);	
	if( UV_FAILED(rc) )
	{
		return UV_ERR(rc);
	}

	result = strtol(sTemp.c_str(), NULL, 0);
	return UV_ERR_OK;
}

void UVDConfigExpressionInterpreter::updateCache(uint32_t address, const UVDVariableMap &result)
{
	printf("Caching interpretation of address %d\n", address);
	m_interpretCache[address] = result;
}

uv_err_t UVDConfigExpressionInterpreter::readCache(uint32_t address, UVDVariableMap &result)
{
	if( m_interpretCache.find(address) == m_interpretCache.end() )
	{
		return UV_ERR_GENERAL;
	}
	result = m_interpretCache[address];
	return UV_ERR_OK;
}

