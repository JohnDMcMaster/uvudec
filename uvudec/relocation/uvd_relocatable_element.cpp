/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#include "uvd_relocation.h"

/*
UVDRelocatableElement
*/

UVDRelocatableElement::UVDRelocatableElement()
{
	m_dynamicValue = 0;
	m_isDynamicValueValid = false;
}

UVDRelocatableElement::UVDRelocatableElement(uint32_t dynamicValue)
{
	m_dynamicValue = dynamicValue;
	m_isDynamicValueValid = true;
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
	
uv_err_t UVDRelocatableElement::getDynamicValue(int8_t *dynamicValue)
{
	int32_t temp = 0;
	uv_assert_err_ret(getDynamicValue(&temp));
	uv_assert_ret(dynamicValue);
	*dynamicValue = (int8_t)temp;
	return UV_ERR_OK;
}

uv_err_t UVDRelocatableElement::getDynamicValue(uint8_t *dynamicValue)
{
	uint32_t temp = 0;
	uv_assert_err_ret(getDynamicValue(&temp));
	uv_assert_ret(dynamicValue);
	*dynamicValue = (uint8_t)temp;
	return UV_ERR_OK;
}

uv_err_t UVDRelocatableElement::getDynamicValue(int16_t *dynamicValue)
{
	int32_t temp = 0;
	uv_assert_err_ret(getDynamicValue(&temp));
	uv_assert_ret(dynamicValue);
	*dynamicValue = (int16_t)temp;
	return UV_ERR_OK;
}

uv_err_t UVDRelocatableElement::getDynamicValue(uint16_t *dynamicValue)
{
	uint32_t temp = 0;
	uv_assert_err_ret(getDynamicValue(&temp));
	uv_assert_ret(dynamicValue);
	*dynamicValue = (uint16_t)temp;
	return UV_ERR_OK;
}

uv_err_t UVDRelocatableElement::getDynamicValue(int32_t *dynamicValue)
{
	uint32_t temp = 0;
	uv_assert_err_ret(getDynamicValue(&temp));
	uv_assert_ret(dynamicValue);
	*dynamicValue = (uint32_t)temp;
	return UV_ERR_OK;
}

uv_err_t UVDRelocatableElement::getDynamicValue(uint32_t *dynamicValue)
{
	uv_assert_err_ret(updateDynamicValue());
	uv_assert_ret(m_isDynamicValueValid);

	uv_assert_ret(dynamicValue);
	*dynamicValue = m_dynamicValue;
	
	return UV_ERR_OK;
}

uv_err_t UVDRelocatableElement::getDynamicValue(char const **dynamicValue)
{
	uv_assert_err_ret(updateDynamicValue());
	uv_assert_ret(m_isDynamicValueValid);

	uv_assert_ret(dynamicValue);
	*dynamicValue = (const char *)&m_dynamicValue;
	
	return UV_ERR_OK;
}

uv_err_t UVDRelocatableElement::updateDynamicValue()
{
	//No update possible (we cannot validate m_isDynamicValidValid), but no error either
	return UV_ERR_OK;
}

uv_err_t UVDRelocatableElement::getName(std::string &s)
{
	s = m_sName;
	return UV_ERR_OK;
}

uv_err_t UVDRelocatableElement::setName(const std::string &sName)
{
	m_sName = sName;
	return UV_ERR_OK;
}

/*
uv_err_t applyDynamicValue(char const **dynamicValue, int *value);
{
	uv_assert_ret(dynamicValue);
	//FIXME: endianess issues
	*dynamicValue = (const char *)&m_dynamicValue;
	
	return UV_ERR_OK;
}
*/

UVDSelfLocatingRelocatableElement::UVDSelfLocatingRelocatableElement()
{
	m_manager = NULL;
	m_relocatableData = NULL;
	m_data = NULL;
	m_offset = 0;
}

UVDSelfLocatingRelocatableElement::UVDSelfLocatingRelocatableElement(UVDRelocationManager *manager, UVDRelocatableData *relocatableData, uint32_t offset)
{
	m_manager = manager;
	m_relocatableData = relocatableData;
	m_data = NULL;
	m_offset = offset;
}

UVDSelfLocatingRelocatableElement::UVDSelfLocatingRelocatableElement(UVDRelocationManager *manager, UVDData *data, uint32_t offset)
{
	m_manager = manager;
	m_relocatableData = NULL;
	m_data = data;
	m_offset = offset;
}

uv_err_t UVDSelfLocatingRelocatableElement::getDynamicValue(char const **dynamicValue)
{
	uint32_t dataOffset = 0;

	uv_assert_ret(m_manager);
	//At least one of these must be specified
	uv_assert_ret(m_relocatableData || m_data);
	std::vector<UVDRelocatableData *> &relocatableDataSet = m_manager->m_data;
	
	std::vector<UVDRelocatableData *>::iterator iter = relocatableDataSet.begin();
	for( ; ; )
	{
		UVDRelocatableData *relocatableData = NULL;
		UVDData *data = NULL;

		if( iter == relocatableDataSet.end() )
		{
			printf_error("Could not find needed relocatable value out of %d entries\n", relocatableDataSet.size());
			return UV_DEBUG(UV_ERR_GENERAL);
		}
		relocatableData = *iter;
		
		uv_assert_ret(relocatableData);
		uv_assert_err_ret(relocatableData->getRelocatableData(&data));
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

	setDynamicValue(dataOffset + m_offset);
	//XXX: endianess issues
	uv_assert_ret(dynamicValue);
	*dynamicValue = (const char *)&m_dynamicValue;

	return UV_ERR_OK;
}

/*
UVDSelfSizingRelocatableElement
*/

UVDSelfSizingRelocatableElement::UVDSelfSizingRelocatableElement()
{
	m_relocatableData = NULL;
	m_data = NULL;
	m_padding = 0;
}

UVDSelfSizingRelocatableElement::UVDSelfSizingRelocatableElement(UVDRelocatableData *relocatableData, uint32_t padding)
{
	m_relocatableData = relocatableData;
	m_data = NULL;
	m_padding = padding;
}

UVDSelfSizingRelocatableElement::UVDSelfSizingRelocatableElement(UVDData *data, uint32_t padding)
{
	m_relocatableData = NULL;
	m_data = data;
	m_padding = padding;
}

uv_err_t UVDSelfSizingRelocatableElement::getDynamicValue(char const **dynamicValue)
{
	UVDData *data = NULL;
	uint32_t dataSize = 0;

	//At least one of these must be specified
	if( m_relocatableData )
	{
		uv_assert_err_ret(m_relocatableData->getRelocatableData(&data));
	}
	else if( m_data )
	{
		data = m_data;
	}
	else
	{
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	
	uv_assert_ret(data);
	uv_assert_err_ret(data->size(&dataSize));
	
	setDynamicValue(dataSize + m_padding);
	//XXX: endianess issues
	uv_assert_ret(dynamicValue);
	*dynamicValue = (const char *)&m_dynamicValue;

	return UV_ERR_OK;
}

