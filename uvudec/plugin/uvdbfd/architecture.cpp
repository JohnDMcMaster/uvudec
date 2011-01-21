/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvdbfd/architecture.h"
#include "uvdbfd/instruction.h"
#include "uvdbfd/object.h"
#include "uvd/assembly/instruction.h"
#include "uvd/core/iterator.h"
#include <typeinfo>

UVDBFDArchitecture::UVDBFDArchitecture()
{
}

UVDBFDArchitecture::~UVDBFDArchitecture()
{
}

uv_err_t UVDBFDArchitecture::init()
{
	return UV_ERR_OK;
}

uv_err_t UVDBFDArchitecture::deinit()
{
	return UV_ERR_OK;
}

uv_err_t UVDBFDArchitecture::getInstruction(UVDInstruction **out)
{
	uv_assert_ret(out);
	*out = new UVDBFDInstruction();
	uv_assert_ret(*out);
	return UV_ERR_OK;
}

uv_err_t UVDBFDArchitecture::getAddresssSpaceNames(std::vector<std::string> &names)
{
	return UV_ERR_OK;
}

uv_err_t UVDBFDArchitecture::parseCurrentInstruction(UVDInstructionIterator &iterCommon)
{
	//Reduce errors from stale data
	if( !iterCommon.m_instruction )
	{
		//iterCommon.m_instruction = new UVDDisasmInstruction();
		uv_assert_err_ret(getInstruction(&iterCommon.m_instruction));
		uv_assert_ret(iterCommon.m_instruction);
	}
	uv_assert_err_ret(iterCommon.m_instruction->parseCurrentInstruction(iterCommon));

	return UV_ERR_OK;
}

uv_err_t UVDBFDArchitecture::canLoad(const UVDObject *object, const UVDRuntimeHints &hints, uvd_priority_t *confidence, void *user)
{
	uv_assert_ret(object);
	uv_assert_ret(confidence);

	if( typeid(object) == typeid(UVDBFDObject *) )
	{
		*confidence = UVD_MATCH_GOOD;
	}
	else
	{
		*confidence = UVD_MATCH_NONE;
	}

	return UV_ERR_OK;
}

uv_err_t UVDBFDArchitecture::tryLoad(UVDObject *object, const UVDRuntimeHints &hints, UVDArchitecture **out, void *user)
{
	UVDBFDArchitecture *ret = NULL;
	uv_err_t rc = UV_ERR_GENERAL;
	
	if( typeid(object) != typeid(UVDBFDObject *) )
	{
		return UV_DEBUG(UV_ERR_NOTSUPPORTED);
	}
	
	ret = new UVDBFDArchitecture();
	uv_assert_ret(ret);
	uv_assert_err(ret->init());
	
	uv_assert(out);
	*out = ret;
	
	return UV_ERR_OK;

error:
	delete ret;
	return UV_DEBUG(rc);
}

