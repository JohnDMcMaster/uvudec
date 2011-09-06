/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvdbfd/architecture.h"
#include "uvdbfd/instruction.h"
#include "uvdbfd/instruction_iterator.h"
#include "uvdbfd/object.h"
#include "uvd/assembly/instruction.h"
#include "uvd/core/iterator.h"
#include "uvd/core/runtime.h"
#include "uvd/core/uvd.h"
#include <typeinfo>

class UVDBfdInstructionIteratorFactory : public UVDInstructionIteratorFactory {
public:
	UVDBfdInstructionIteratorFactory( UVDBFDObject *object = NULL );

	//virtual uv_err_t abstractInstructionIteratorBegin( UVDAbstractInstructionIterator **out );
	virtual uv_err_t abstractInstructionIteratorBeginByAddress( UVDAbstractInstructionIterator **out, UVDAddress address );
	//virtual uv_err_t abstractInstructionIteratorEnd(UVDAbstractInstructionIterator **out);
	virtual uv_err_t abstractInstructionIteratorEndByAddressSpace(UVDAbstractInstructionIterator **out, UVDAddressSpace *addressSpace);

	UVDBFDObject *object();

public:
	UVDBFDObject *m_object;
};

UVDBfdInstructionIteratorFactory::UVDBfdInstructionIteratorFactory( UVDBFDObject *object ) {
	m_object = object;
}

UVDBFDObject *UVDBfdInstructionIteratorFactory::object() {
	return dynamic_cast<UVDBFDObject*>(g_uvd->m_runtime->m_object);
}
	
//uv_err_t abstractInstructionIteratorBegin( UVDAbstractInstructionIterator **out );
uv_err_t UVDBfdInstructionIteratorFactory::abstractInstructionIteratorBeginByAddress( UVDAbstractInstructionIterator **out, UVDAddress address ) {
	UVDBfdInstructionIterator *iter = NULL;
	
	iter = new UVDBfdInstructionIterator();
	uv_assert_ret(iter);
	
	printf("Begin address space %s\n", address.m_space->m_name.c_str());
	uv_assert_err_ret(iter->initByAddress(object(), address));
	
	printf("begin name %s\n", iter->m_section->name);
	//out = NULL;
	
	uv_assert_ret(out);
	*out = iter;
	return UV_ERR_OK;
}

//uv_err_t abstractInstructionIteratorEnd(UVDAbstractInstructionIterator **out);
uv_err_t UVDBfdInstructionIteratorFactory::abstractInstructionIteratorEndByAddressSpace(UVDAbstractInstructionIterator **out, UVDAddressSpace *addressSpace) {
	UVDBfdInstructionIterator *iter = NULL;
	
	iter = new UVDBfdInstructionIterator();
	uv_assert_ret(iter);
	
	uv_assert_err_ret(iter->initEnd(object(), addressSpace));
	
	uv_assert_ret(out);
	*out = iter;
	return UV_ERR_OK;
}

/*
UVDBFDArchitecture
*/

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

/*
uv_err_t UVDBFDArchitecture::parseCurrentInstruction(UVDInstructionIterator &iterCommon)
{
	UVDInstruction *instructionRaw = NULL;
	UVDBFDInstruction *instruction = NULL;
	
	uv_assert_err_ret(iterCommon.get(&instructionRaw));
	uv_assert_ret(instructionRaw);
	instruction = dynamic_cast<UVDBFDInstruction *>(instructionRaw);
	uv_assert_ret(instruction);
	uv_assert_err_ret(instruction->parseCurrentInstruction(iterCommon));

	return UV_ERR_OK;
}
*/

uv_err_t UVDBFDArchitecture::canLoad(const UVDObject *object, const UVDRuntimeHints &hints, uvd_priority_t *confidence, void *user)
{
	uv_assert_ret(object);
	uv_assert_ret(confidence);

	if( typeid(*object) == typeid(UVDBFDObject) ) {
		*confidence = UVD_MATCH_GOOD + 1;
	} else {
		*confidence = UVD_MATCH_NONE;
	}

	return UV_ERR_OK;
}

uv_err_t UVDBFDArchitecture::tryLoad(UVDObject *object, const UVDRuntimeHints &hints, UVDArchitecture **out, void *user)
{
	UVDBFDArchitecture *ret = NULL;
	uv_err_t rc = UV_ERR_GENERAL;
	
	if( typeid(*object) != typeid(UVDBFDObject) ) {
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

uv_err_t UVDBFDArchitecture::getInstructionIteratorFactory(UVDInstructionIteratorFactory **out) {
	UVDBfdInstructionIteratorFactory *factory = NULL;
	
	//This won't work b/c currently arch is init after obj + arch
	//factory = new UVDBfdInstructionIteratorFactory(dynamic_cast<UVDBFDObject*>(m_uvd->m_runtime->m_object));
	factory = new UVDBfdInstructionIteratorFactory();
	*out = factory;
	
	return UV_ERR_OK;
}

/*
uv_err_t UVDBFDArchitecture::sectionToAddressSpace( const asection *section, UVDAddressSpace **out ) {
	return UV_ERR_GENERAL;
}

uv_err_t UVDBFDArchitecture::addressSpaceToSection( const UVDAddressSpace *addressSpace, asection **out ) {
	return UV_ERR_GENERAL;
}
*/
