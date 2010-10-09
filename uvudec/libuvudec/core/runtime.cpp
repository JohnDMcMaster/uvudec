/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "core/runtime.h"

UVDRuntime::UVDRuntime()
{
	m_object = NULL;
	m_architecture = NULL;
}

UVDRuntime::~UVDRuntime()
{
}

uv_err_t UVDRuntime::init(UVDObject *object, UVDArchitecture *architecture)
{
	//Haven't decided yet if its an error not to specify these
	//For now, assume it is unless we get a good reason not to
	uv_assert_ret(object);
	uv_assert_ret(architecture);
	m_architecture = architecture;
	m_object = object;

	uv_assert_err_ret(rebuildAddressSpaces());

	return UV_ERR_OK;
}

uv_err_t UVDRuntime::rebuildAddressSpaces()
{
	/*
	For now, just copy in all object file defined spaces since they are the ones with actual code
	In the future, we'll probably want to make a number of improvements
	
	We should map ROM images into their appropriete memory spaces
	It is not enough to just eliminate the architecture defined memory space
		It may not fill up the entire space
		Will lose intrinsic capabilites
	
	One is to do some sort of assumption based on memory protection
	Techincally assuming there is an OS doesn't mean we have MMU protection
		Ex: Linux running on MicroBlaze compiled w/o MMU
	if( m_OS )
		rely on object information only
	else
		Use architecture file as baseline
		Object file may indicate hints where we actually load code
		We indicate the actual validity of a mapped address based on if data is actually associated with it
	*/
	uv_assert_err_ret(clearAddressSpaces());
	if( m_architecture )
	{
		for( std::vector<UVDAddressSpace *>::iterator iter = m_architecture->m_addressSpaces.m_addressSpaces.begin();
				iter != m_architecture->m_addressSpaces.m_addressSpaces.end(); ++iter )
		{
			//TODO: we will eventually need to take these into account
		}
	}
	
	if( m_object )
	{
		//Binary object supported for now doesn't do this
		//ELF, PE, s-record, etc will
		for( std::vector<UVDSection *>::iterator iter = m_object->m_sections.begin();
				iter != m_object->m_sections.end(); ++iter )
		{
			UVDSection *section = *iter;
			UVDAddressSpace *space = NULL;

			uv_assert_ret(section);
			//Don't shift addresses, keep exactly the same for now
			//uv_assert_err_ret(section->m_addressSpace->remap(&space));
			uv_assert_err_ret(section->toAddressSpace(&space));

			m_addressSpaces.m_addressSpaces.push_back(space);
		}
	}
	
	return UV_ERR_OK;
}

uv_err_t UVDRuntime::clearAddressSpaces()
{
	for( std::vector<UVDAddressSpace *>::iterator iter = m_addressSpaces.m_addressSpaces.begin();
			iter != m_addressSpaces.m_addressSpaces.end(); ++iter )
	{
		delete *iter;
	}
	m_addressSpaces.m_addressSpaces.clear();
	
	return UV_ERR_OK;
}

uv_err_t UVDRuntime::getPrimaryExecutableAddressSpace(UVDAddressSpace **out)
{
	uv_assert_ret(out);
	for( std::vector<UVDAddressSpace *>::iterator iter = m_addressSpaces.m_addressSpaces.begin();
			iter != m_addressSpaces.m_addressSpaces.end(); ++iter )
	{
		UVDAddressSpace *addressSpace = *iter;
		
		uv_assert_ret(addressSpace);
		//Make sure, we'd probably get too many false positives on unknown spaces
		if( addressSpace->m_X == UVD_TRI_TRUE )
		{
			*out = addressSpace;
			return UV_ERR_OK;
		}
	}

	//Nothing found
	*out = NULL;
	return UV_ERR_NOTFOUND;
}

