/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvdasm/interpreter/instruction.h"
#include "uvdasm/interpreter/interpreter.h"

/*
UVDInterpretedInstruction
*/

UVDInterpretedInstruction::UVDInterpretedInstruction()
{
}

UVDInterpretedInstruction::~UVDInterpretedInstruction()
{
}

uv_err_t UVDInterpretedInstruction::parse(const UVDVariableMap &attributes)
{
	uv_assert_err_ret(parseAttributeOptional(attributes, SCRIPT_KEY_SYMBOL, &m_symbol, ""));
	return UV_ERR_OK;
}

uv_err_t UVDInterpretedInstruction::parseAttributeCore(const UVDVariableMap &attributes, const std::string &key, uint32_t *val)
{
	uv_assert_ret(val);

	iter = attributes.find(key);
	if( iter == attributes.end() )
	{
		return UV_ERR_GENERAL;
	}
	*val = (uint32_t)strtol((*iter).c_str(), NULL, 0);
	return UV_ERR_OK;
}

uv_err_t UVDInterpretedInstruction::parseAttributeCore(const UVDVariableMap &attributes, const std::string &key, std::string *val)
{
	uv_assert_ret(val);

	iter = attributes.find(key);
	if( iter == attributes.end() )
	{
		return UV_ERR_GENERAL;
	}
	*val = *iter;
	return UV_ERR_OK;
}

uv_err_t UVDInterpretedInstruction::parseAttributeRequired(const UVDVariableMap &attributes, const std::string &attribute, uint32_t *val)
{
	return UV_DEBUG(parseAttributeOptional(attributes, attribute, val));
}

uv_err_t UVDInterpretedInstruction::parseAttributeRequired(const UVDVariableMap &attributes, const std::string &attribute, std::string *val)
{
	return UV_DEBUG(parseAttributeOptional(attributes, attribute, val));
}

uv_err_t parseAttributeOptional(const UVDVariableMap &attributes, const std::string &attribute, uint32_t *val, uint32_t valDefault)
{
	uv_assert_ret(val);

	if( UV_FAILED(parseAttributeCore(attributes, attribute, val)) )
	{
		*val = valDefault;
	}
	return UV_ERR_OK;
}

uv_err_t parseAttributeOptional(const UVDVariableMap &attributes, const std::string &attribute, std::string *val, const std::string &valDefault)
{
	uv_assert_ret(val);

	if( UV_FAILED(parseAttributeCore(attributes, attribute, val)) )
	{
		*val = valDefault;
	}
	return UV_ERR_OK;
}

/*
UVDInterpretedCall
*/

UVDInterpretedCall::UVDInterpretedCall()
{
	m_callTarget = 0;
}

UVDInterpretedCall::~UVDInterpretedCall()
{
}

uv_err_t UVDInterpretedCall::parse(const UVDVariableMap &attributes)
{
	uv_assert_err_ret(parseAttributeRequired(attributes, SCRIPT_KEY_CALL, &m_callTarget));

	return UV_ERR_OK;
}

/*
UVDInterpretedBranch
*/

UVDInterpretedBranch::UVDInterpretedBranch()
{
	m_branchTarget = 0;
}

UVDInterpretedBranch::~UVDInterpretedBranch()
{
}

uv_err_t UVDInterpretedBranch::parse(const UVDVariableMap &attributes)
{
	uv_assert_err_ret(parseAttributeRequired(attributes, SCRIPT_KEY_JUMP, &m_branchTarget));

	return UV_ERR_OK;
}
