/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_LUA_H
#define UVD_LUA_H

#include "config.h"

#ifdef USING_LUA

#include "uvd_interpreter.h"

#include <string>

extern "C" 
{
#include "lua.h"
}

class UVDLuaInterpreter : public UVDInterpreter
{
public:
	UVDLuaInterpreter();
	virtual ~UVDLuaInterpreter();
	virtual uv_err_t init();
	
	virtual uv_err_t interpret(const UVDInterpreterExpression &exp, const UVDVariableMap &environment, std::string &sRet);

protected:
	lua_State *m_luaState;
};

#endif //USING_LUA

#endif
