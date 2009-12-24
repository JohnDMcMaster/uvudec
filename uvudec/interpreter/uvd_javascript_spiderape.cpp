/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#ifdef USING_SPIDERAPE

/*
Excerpts taken from:
	builtins.cpp
	scriptable_base.hpp
These are licensed under GPL/LGPL/Mozilla
If this small code excert is an issue, will rewrite and such to avoid issues
*/

#include <string>
#include <vector>
#include "uvd_util.h"
#include "uvd_python.h"
#include "uvd_config.h"
#include "uvd_javascript_spiderape.h"
#include <sf.net/ape/spiderape.hpp>
#include <sf.net/ape/scriptable_base.hpp>
// If using the 2005.10.03 release of Ape, instead include:
// #include <s11n.net/ape/spiderape.hpp>

#define JS_FAILED(x) (x != JS_TRUE)
#define JS_SUCCEEDED(x) (x == JS_TRUE)

std::stringstream g_stringStream;

void spiderMonkeyErrorReporter(JSContext *cx, const char *message, JSErrorReport *report)
{
	printf_error("SpiderMonkey (under SpikerApe): Recieved error string: %s\n", message);
}

/*
Print reimplementation to redirect output to string buffer instead of stderr/stdout
*/ 
JSBool
uvd_ape_print_impl(std::ostream & os,
	       const std::string & recsep,
	       const std::string & linesep,
	       JSContext *cx, JSObject *obj,
	       uintN argc, jsval *argv,
	       jsval *rval)
{
	//printf_debug("My print impl\n");
	uintN i;
	JSString *str = 0;
	for (i = 0; i < argc; i++)
	{
		str = JS_ValueToString(cx, argv[i]);
		if (!str)
		{
			JS_ReportError(cx, "uvd_ape_print_impl() could not convert arg to string." );
			return JS_FALSE;
		}
		os << (i ? recsep : std::string())
		   << ((JS_GetStringLength(str) > 0) ? JS_GetStringBytes(str) : "");
	}
	if( ! linesep.empty() )
	{
		os << linesep;
	}
	os.flush();
	return JS_TRUE;
}

JSBool
uvd_ape_print(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	return uvd_ape_print_impl(g_stringStream, " ", "\n", cx, obj, argc, argv, rval );
	//return uvd_ape_print_impl(std::cout, " ", "\n", cx, obj, argc, argv, rval );
}

std::string getNextBufferData()
{
	std::string sRet;
	/*
	for( ;; )
	{
		char buffer[1024];
		//Allow space for null termination
		g_stringStream.read(buffer, sizeof(buffer) - 1);
		int readCount = g_stringStream.gcount();
		printf_debug("read from buffer: %d\n", readCount);
		buffer[readCount] = 0;
		if( readCount == 0 )
		{
			break;
		}
		sRet += buffer;
	}
	*/
	sRet = g_stringStream.str();
	g_stringStream.str("");
	return sRet;
}

static JSFunctionSpec builtin_funcs[] = {
	{"print",           uvd_ape_print,              0 },
	{0, 0, 0} // don't forget this, else risk a segfault.
	};

JSFunction * add_function(ape::MonkeyWrapper &js, std::string const & name, JSFunction * f )
{
	if( ! f ) return 0;
	jsval fv = OBJECT_TO_JSVAL(f->object);
	//if( ! JS_DefineProperty(this->js_context(), this->js_object(),
	if( ! JS_DefineProperty(js.context(), js.global(),
				name.c_str(), fv, NULL, NULL, 0 ) )
	{
		//CERR << "add_function("<<name<<"): JS_DefineProperty() failed!\n";
		return 0;
	}
	//this->m_funcs[name] = f;
	return f;
}

JSFunction * add_function( ape::MonkeyWrapper &js, std::string const & name, JSNative nf )
{
	if( ! nf ) return 0;
	JSFunction * f = JS_NewFunction(js.context(),
					nf, 0, JSFUN_FLAGS_MASK, // | JSFUN_BOUND_METHOD, // | JSFUN_LAMBDA,
					js.global(),
					name.c_str() );
	return add_function( js, name, f );
}

size_t add_functions( ape::MonkeyWrapper &js, JSFunctionSpec head[] )
{
	size_t ret = 0;
	for( JSFunctionSpec * f = head;
	     f && f->name;
	     ++f )
	{
		add_function( js, f->name, f->call );
		++ret;
	}
	return ret;
}



UVDJavascriptSpiderapeInterpreter::UVDJavascriptSpiderapeInterpreter()
{
}

UVDJavascriptSpiderapeInterpreter::~UVDJavascriptSpiderapeInterpreter()
{
}

uv_err_t UVDJavascriptSpiderapeInterpreter::init()
{
	uv_assert_err_ret(UVDInterpreter::init());	

	//Stop printing errors to stdout
	JS_SetErrorReporter(m_monkeyWrapper.context(), spiderMonkeyErrorReporter);
	//Mostly ovveride print so output does not go to stdout
	add_functions(m_monkeyWrapper, builtin_funcs);

	return UV_ERR_OK;
}

uv_err_t UVDJavascriptSpiderapeInterpreter::interpret(const UVDInterpreterExpression &exp, const UVDVariableMap &environment, std::string &sRet)
{
	uv_err_t rc = UV_ERR_GENERAL;
	std::string sJavascriptProgram;
	std::string sScriptRet;

	UV_ENTER();

	//Clear buffer, if possible
	//g_stringStream.sync();

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
		sJavascriptProgram += key  + "=" + value + ";\n";		
	}
	
	/*
	Utility functions
	*/
	
	//Call results should be printed so that they can be retrieved
	sJavascriptProgram +=
		"function CALL(address)\n"
		"{\n"
			"\tprint('" SCRIPT_KEY_CALL "=' +  address + '\\n');\n"
		"}\n"
		"\n";
	
	//Jump results should be printed so that they can be retrieved
	sJavascriptProgram +=
		"function GOTO(address)\n"
		"{\n"
			"\tprint('" SCRIPT_KEY_JUMP "=' + address + '\\n');\n"
			//"print('test');\n"
		"}\n"
		"\n";

	/*
	Main expression
	*/
	sJavascriptProgram += exp.m_sExpression + ";\n";	

	printf_debug("Running: <%s>\n", sJavascriptProgram.c_str());


	/*
	Run it!
	*/
	jsval retVal;	
	if( JS_FAILED(m_monkeyWrapper.eval_source(
			// arbitrary JS code
			sJavascriptProgram.c_str(), 
			// return value
			retVal,
			// script name, for error reporting
			"uvudec_anonymous" 
			)) )
	{
		printf_error("Failed to execute script!\n");
		printf_error("%s\n", sJavascriptProgram.c_str());
		goto error;
	}
	//Would like to make this into a general error code, ignore for now
	sScriptRet = ape::jsval_to_std_string(m_monkeyWrapper.context(), retVal);
	printf_debug("script returned: %s\n", sScriptRet.c_str());

	sRet = getNextBufferData();
	printf_debug("Buffered output: <%s>\n", sRet.c_str());	
	
	rc = UV_ERR_OK;
	
error:
	return UV_DEBUG(rc);
}

#endif ///USING_SPIDERAPE
