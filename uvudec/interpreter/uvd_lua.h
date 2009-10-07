/*
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
*/

#ifdef USING_LUA

#include "uvd_interpreter.h"

#include <string>

extern "C" {
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
