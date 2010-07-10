/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#include "uvd_interpreter.h"
#include "uvd_types.h"

UVDInterpreterExpression::UVDInterpreterExpression()
{	
	m_compiled = NULL;
	m_interpreter = NULL;
}

UVDInterpreterExpression::~UVDInterpreterExpression()
{	
}

uv_err_t UVDInterpreterExpression::compile(const std::string &sExpression)
{
	uv_assert_ret(m_interpreter);
	return UV_DEBUG(m_interpreter->compile(sExpression, this));
}

UVDInterpreter::UVDInterpreter()
{
}

UVDInterpreter::~UVDInterpreter()
{
}

uv_err_t UVDInterpreter::init()
{
	return UV_ERR_OK;
}

uv_err_t UVDInterpreter::compile(const std::string &sExp, UVDInterpreterExpression *result)
{
	//Default to only doing basic setup
	//This is acceptable for expressions that can't be compiled

	uv_assert_err_ret(result);
	
	//Reset to defaults
	*result = UVDInterpreterExpression();
	//Store basic info only
	result->m_interpreter = this;
	result->m_sExpression = sExp;
	
	return UV_ERR_OK;
}

uv_err_t UVDInterpreter::getInterpreterExpression(UVDInterpreterExpression **expression_in)
{
	UVDInterpreterExpression *expression = NULL;
	
	expression = new UVDInterpreterExpression();
	uv_assert_ret(expression);
	
	expression->m_interpreter = this;
	
	uv_assert_ret(expression_in);
	*expression_in = expression;
	
	return UV_ERR_OK;
}

uv_err_t UVDInterpreter::varientToScriptValue(UVDVarient varient, std::string &value)
{
	/*
	C like types conversion
	Common to most langauges, so have as default
	*/
	uv_err_t rc = UV_ERR_GENERAL;

	switch( varient.getType() )
	{
	case UVD_VARIENT_STRING:
	{
		std::string s;
		uv_assert_err(varient.getString(s));
		value = std::string("\"") + s + "\"";
		break;
	}
	case UVD_VARIENT_UINT32:
	{
		char buff[64];
		uint32_t ui32Val;
		
		uv_assert_err(varient.getUI32(ui32Val));
		snprintf(buff, 64, "0x%.8X", ui32Val);
		value = buff;
		break;
	}
	case UVD_VARIENT_INT32:
	{
		char buff[64];
		int32_t i32Val;
		
		uv_assert_err(varient.getI32(i32Val));
		snprintf(buff, 64, "%d", i32Val);
		value = buff;
		break;
	}
	default:
		UV_DEBUG(rc);
		goto error;
	}
	

	rc = UV_ERR_OK;

error:
	return UV_DEBUG(rc);
}
