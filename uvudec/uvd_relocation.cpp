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
	m_size = 1;
}

UVDRelocationFixup::UVDRelocationFixup(UVDRelocatableElement *symbol, unsigned int offset, unsigned int size)
{
	m_symbol = symbol;
	m_offset = offset;
	m_size = size;
}

UVDRelocationFixup::~UVDRelocationFixup()
{
}

uv_err_t UVDRelocationFixup::applyPatch(UVDData *data)
{
	const char *dynamicValue = NULL;

	uv_assert_ret(m_symbol);
	uv_assert_err_ret(m_symbol->getDynamicValue(&dynamicValue));
	//XXX: endianness issues
	uv_assert_err_ret(data->writeData(m_offset, dynamicValue, m_size));
	
	return UV_ERR_OK;
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
	//Simply loop through all patches/relocations and apply all of them
	for( std::set<UVDRelocationFixup *>::iterator iter = m_fixups.begin(); iter != m_fixups.end(); ++iter )
	{
		UVDRelocationFixup *fixup = *iter;
		uv_assert_ret(fixup);
		uv_assert_err_ret(fixup->applyPatch(m_data));
	}

	return UV_ERR_OK;
}

void UVDRelocatableData::addFixup(UVDRelocationFixup *fixup)
{
	m_fixups.insert(fixup);
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
	m_isDynamicValueValid = true;
}

/*
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
*/
	
uv_err_t UVDRelocatableElement::getDynamicValue(char const **dynamicValue)
{
	uv_assert_ret(dynamicValue);
	uv_assert_ret(m_isDynamicValueValid);
	//FIXME: endianess issues
	*dynamicValue = (const char *)&m_dynamicValue;
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

uv_err_t UVDSelfLocatingRelocatableElement::getDynamicValue(char const **dynamicValue)
{
	unsigned int dataOffset = 0;

	uv_assert_ret(m_manager);
	//At least one of these must be specified
	uv_assert_ret(m_relocatableData || m_data);
	std::vector<UVDRelocatableData *> &relocatableDataSet = m_manager->m_data;
	
	std::vector<UVDRelocatableData *>::iterator iter = relocatableDataSet.begin();
	for( ; ; )
	{
		UVDRelocatableData *relocatableData = NULL;
		UVDData *data = NULL;

		uv_assert_ret(iter != relocatableDataSet.end());
		relocatableData = *iter;
		
		uv_assert_ret(relocatableData);
		data = relocatableData->m_data;
		uv_assert_ret(data);
		
		//See if we have a match
		if( relocatableData == m_relocatableData
				|| data == m_data )
		{
			break;
		}
		
		dataOffset += data->size();
		++iter;
	}

	uv_assert_ret(dynamicValue);
	m_dynamicValue = dataOffset + m_offset;
	//XXX: endianess issues
	*dynamicValue = (const char *)&m_dynamicValue;

	return UV_ERR_OK;
}

UVDRelocationManager::UVDRelocationManager()
{
}

UVDRelocationManager::~UVDRelocationManager()
{
}

uv_err_t UVDRelocationManager::addRelocatableElement(UVDRelocatableElement *element)
{
	uv_assert_ret(element);
	m_relocatableElements.insert(element);
	return UV_ERR_OK;
}

uv_err_t UVDRelocationManager::addRelocatableData(UVDRelocatableData *data)
{
	uv_assert_ret(data);
	m_data.push_back(data);
	return UV_ERR_OK;
}

uv_err_t UVDRelocationManager::applyPatch(UVDData **dataOut)
{
	std::vector<UVDData *> dataVector;

	for( std::vector<UVDRelocatableData *>::iterator iter = m_data.begin(); iter != m_data.end(); ++iter )
	{
		UVDRelocatableData *relocatableData = *iter;
		UVDData *data = NULL;

		uv_assert_ret(relocatableData);
		//Update the encapsulated data to reflect the relocations
		uv_assert_err_ret(relocatableData->applyRelocations());
		
		
		//And rack up the raw data so it can be assembled
		data = relocatableData->m_data;
		uv_assert_ret(data);
		dataVector.push_back(data);
	}
	uv_assert_err_ret(UVDData::concatenate(dataVector, dataOut));
	
	return UV_ERR_OK;
}

