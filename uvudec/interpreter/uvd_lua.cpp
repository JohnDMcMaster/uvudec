/*
Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#ifdef USING_LUA

#include "uvd_lua.h"
#include "uvd_config.h"

#include <string>
#include <math.h>

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}


/*
WARNING
If this was made multitrheaded (for w/e reason), this would need to be synchronized or something
*/

std::string g_luaPrintBuffer;

//Ovveridden print function
static int luaPrint(lua_State *L);

UVDLuaInterpreter::UVDLuaInterpreter()
{
}

uv_err_t UVDLuaInterpreter::init()
{
	UV_ENTER();
	
	uv_assert_err_ret(UVDInterpreter::init());

	m_luaState = lua_open();
	if( m_luaState == NULL )
	{
		printf("Failed to initialize Lua\n");
		return 1;
	}

	// load the libs
	//FIXME: do I need this?  I don't really call any Lua functions
	//luaL_openlibs(m_luaState);

	//Overrite print so we can instead do internal processing and not print to stdout/stderr
    lua_pushcfunction(m_luaState, luaPrint);
    lua_setglobal(m_luaState, "print");

	/*
	std::string sExec = "print(\"sin res: \" .. mysin(1.57) .. \"\\n\")";
	printf("Exec: %s\n", sExec.c_str());
	if( luaL_dostring(m_luaState, sExec.c_str()) )
	{
		printf("Failed to exec string\n");
	}
	*/
	return UV_ERR_OK;
}

UVDLuaInterpreter::~UVDLuaInterpreter()
{
	lua_close(m_luaState);
}

uv_err_t UVDLuaInterpreter::interpret(const UVDInterpreterExpression &exp, const UVDVariableMap &environment, std::string &sRet)
{
	//How to start with clean environment?
	//Assume no global variables carrying over?
	
	UV_ENTER();
	
	//What we will exec in Lua
	std::string sLuaProgram;

	//Make sure we don't have any garbaged buffered
	g_luaPrintBuffer.clear();


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
		sLuaProgram += key  + "=" + value + "\n";		
	}
	
	
	/*
	Utility functions
	*/
	
	/*
	FIXME:
	compile these or make them functions that callback here
	*/
	
	//Call results should be printed so that they can be retrieved
	sLuaProgram +=
		"function CALL(address)\n"
			"\tprint('" SCRIPT_KEY_CALL "=' .. address)\n"
		"\n"
		"end\n";
	
	//Jump results should be printed so that they can be retrieved
	sLuaProgram +=
		"function GOTO(address)\n"
			"\tprint('" SCRIPT_KEY_JUMP "=' .. address)\n"
		"\n"
		"end\n";
	
	
	/*
	What we actually want to exec
	*/
	sLuaProgram += exp.m_sExpression;


	//Returns 0 on success
	g_luaPrintBuffer.clear();
	if( luaL_dostring(m_luaState, sLuaProgram.c_str()) != 0 )
	{
		//Execution failed
		printf_error("ERROR: could not execute: %s\n", sLuaProgram.c_str());
		return UV_DEBUG(UV_ERR_GENERAL);
	}

	//Grab the buffer we accumulated
	sRet = g_luaPrintBuffer;
	//We grabbed the data already
	g_luaPrintBuffer.clear();

	return UV_ERR_OK;
}

static int luaPrint(lua_State *L) 
{
	/*
	Comes in as:
	-No implied newline termination
		Seems to be added by the internal print
	-n_opts is always 1 it seems
		"abc" .. 123 comes as "abc123"
	-Must be careful on passing of newline
	*/
	
	int n_opts = lua_gettop(L);
	printf_debug("luaPrint: n_opts: %d\n", n_opts);

	for( int i = 1; i <= n_opts; ++i )
	{
		const char *val = lua_tostring(L, i);

		if( val == NULL )
		{
			printf_debug("No val?\n");
		}
		else
		{
			printf_debug("val: <%s>\n", val);
			g_luaPrintBuffer += val;
		}
	}
	
	printf_debug("Done print\n");
	
	//No values returned, successful
	return 0;
}

#endif //USING_LUA
