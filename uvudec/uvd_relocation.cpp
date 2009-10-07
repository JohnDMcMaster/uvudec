/*
Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under the terms of the BSD license.  See LICENSE for details.
*/

#include "uvd_relocation.h"


UVDRelocationFixup::UVDRelocationFixup()
{
	m_symbol = NULL;
	m_offset = 0;
}

UVDRelocationFixup::UVDRelocationFixup(UVDRelocatableElement *symbol, unsigned int offset)
{
	m_symbol = symbol;
	m_offset = offset;
}

UVDRelocationFixup::~UVDRelocationFixup()
{
}

uv_err_t UVDRelocationFixup::applyPatch(UVDData *data)
{
	return UV_ERR_GENERAL;
}

	
UVDRelocatableData::UVDRelocatableData()
{
	m_data = NULL;
}

UVDRelocatableData::UVDRelocatableData(UVDData *data)
{
	m_data = data;
}

UVDRelocatableData::~UVDRelocatableData()
{
}

uv_err_t UVDRelocatableData::applyRelocations()
{
	return UV_ERR_GENERAL;
}

void UVDRelocatableData::addFixup(UVDRelocationFixup *)
{
}

UVDRelocatableElement::UVDRelocatableElement()
{
	m_dynamicValue = 0;
	m_isDynamicValueValid = false;
}

UVDRelocatableElement::~UVDRelocatableElement()
{
}

void UVDRelocatableElement::setDynamicValue(int dynamicValue)
{
	m_dynamicValue = dynamicValue;
}

int UVDRelocatableElement::getDynamicValue(void)
{
	return m_dynamicValue;
}

uv_err_t UVDRelocatableElement::getDynamicValue(int *dynamicValue)
{
	if( !m_isDynamicValueValid )
	{
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	uv_assert_ret(dynamicValue);
	*dynamicValue = m_dynamicValue;
	return UV_ERR_OK;
}
	

UVDSelfLocatingRelocatableElement::UVDSelfLocatingRelocatableElement()
{
	m_manager = NULL;
	m_relocatableData = NULL;
	m_data = NULL;
	m_offset = 0;
}

UVDSelfLocatingRelocatableElement::UVDSelfLocatingRelocatableElement(UVDRelocationManager *manager, UVDRelocatableData *relocatableData, unsigned int offset)
{
	m_manager = manager;
	m_relocatableData = relocatableData;
	m_data = NULL;
	m_offset = offset;
}

UVDSelfLocatingRelocatableElement::UVDSelfLocatingRelocatableElement(UVDRelocationManager *manager, UVDData *data, unsigned int offset)
{
	m_manager = manager;
	m_relocatableData = NULL;
	m_data = data;
	m_offset = offset;
}

uv_err_t UVDSelfLocatingRelocatableElement::getDynamicValue(int *dynamicValue)
{
	return UV_ERR_GENERAL;
}

UVDRelocationManager::UVDRelocationManager()
{
}

UVDRelocationManager::~UVDRelocationManager()
{
}

void UVDRelocationManager::addRelocatableElement(UVDRelocatableElement *element)
{
}

void UVDRelocationManager::addRelocatableData(UVDRelocatableData *data)
{
}

uv_err_t UVDRelocationManager::applyPatch(UVDData *data)
{
	return UV_ERR_GENERAL;
}

uv_err_t UVDRelocationManager::applyPatch(UVDData **data)
{
	return UV_ERR_GENERAL;
}

