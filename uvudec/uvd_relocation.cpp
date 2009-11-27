/*
Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
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
	return UV_DEBUG(applyPatchCore(data, false));
}

uv_err_t UVDRelocationFixup::applyPatchCore(UVDData *data, bool useDefaultValue)
{
	const char *dynamicValue = NULL;
	int defaultValue = RELOCATION_DEFAULT_VALUE;

	//XXX: endianness, size issues
	if( useDefaultValue )
	{
		dynamicValue = (const char *)&defaultValue;
	}
	else
	{
		uv_assert_ret(m_symbol);
		uv_assert_err_ret(m_symbol->getDynamicValue(&dynamicValue));
	}
	
	uv_assert_err_ret(data->writeData(m_offset, dynamicValue, m_size));
	
	return UV_ERR_OK;
}

	
UVDRelocatableData::UVDRelocatableData()
{
	m_data = NULL;
	m_defaultRelocatableData = NULL;
}

UVDRelocatableData::UVDRelocatableData(UVDData *data)
{
	m_data = data;
}

class UVDRelocatableDataPlaceholder : public UVDRelocatableData
{
public:
	UVDRelocatableDataPlaceholder();
	~UVDRelocatableDataPlaceholder();
	
public:
	//Statically allocated data structure
	//Empty data structure opreations to make other operations work nicely
	UVDDataPlaceholder m_dataPlaceholder;
};

UVDRelocatableDataPlaceholder::UVDRelocatableDataPlaceholder()
{
	m_data = &m_dataPlaceholder;
	m_defaultRelocatableData = &m_dataPlaceholder;
}

UVDRelocatableDataPlaceholder::~UVDRelocatableDataPlaceholder()
{
}

uv_err_t getUVDRelocatableDataPlaceholder(UVDRelocatableData **dataOut)
{
	UVDRelocatableData *data = NULL;
	
	data = new UVDRelocatableDataPlaceholder();
	uv_assert_ret(data);

	uv_assert_ret(dataOut);
	*dataOut = data;

	return UV_ERR_OK;
}

UVDRelocatableData::~UVDRelocatableData()
{
}

uv_err_t UVDRelocatableData::applyRelocations()
{
	return UV_DEBUG(applyRelocationsCore(false));
}

uv_err_t UVDRelocatableData::applyRelocationsCore(bool useDefaultValue)
{
	//Simply loop through all patches/relocations and apply all of them
	for( std::set<UVDRelocationFixup *>::iterator iter = m_fixups.begin(); iter != m_fixups.end(); ++iter )
	{
		UVDRelocationFixup *fixup = *iter;
		uv_assert_ret(fixup);
		uv_assert_err_ret(fixup->applyPatchCore(m_data, useDefaultValue));
	}

	return UV_ERR_OK;
}

void UVDRelocatableData::addFixup(UVDRelocationFixup *fixup)
{
	m_fixups.insert(fixup);
}

uv_err_t UVDRelocatableData::getRelocatableData(UVDData **data)
{
	uv_assert_ret(data);
	//uv_assert_ret(m_data);
	*data = m_data;
	return UV_ERR_OK;
}

uv_err_t UVDRelocatableData::getDefaultRelocatableData(UVDData **data)
{
	uv_assert_ret(data);
	if( m_defaultRelocatableData )
	{
		*data = m_defaultRelocatableData;
		return UV_ERR_OK;
	}
	
	//Copy our data as a base
	uv_assert_err_ret(UVDDataMemory::getUVDDataMemoryByCopy(m_data, &m_defaultRelocatableData));
	uv_assert_ret(m_defaultRelocatableData);
	
	//And fixup (zero) all the relocation places
	for( std::set<UVDRelocationFixup *>::iterator iter = m_fixups.begin(); iter != m_fixups.end(); ++iter )
	{
		UVDRelocationFixup *fixup = *iter;
		uv_assert_ret(fixup);
		uv_assert_err_ret(fixup->applyPatchCore(m_defaultRelocatableData, true));
	}
	
	*data = m_defaultRelocatableData;
	return UV_ERR_OK;
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

std::string UVDRelocatableElement::getName()
{
	return m_sName;
}

void UVDRelocatableElement::setName(const std::string &sName)
{
	m_sName = sName;
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
	setDynamicValue(dataOffset + m_offset);
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

/*
uv_err_t UVDRelocationManager::addRelocatableElement(UVDRelocatableElement *element)
{
	uv_assert_ret(element);
	m_relocatableElements.insert(element);
	return UV_ERR_OK;
}
*/

uv_err_t UVDRelocationManager::addRelocatableData(UVDRelocatableData *data)
{
	uv_assert_ret(data);
	m_data.push_back(data);
	return UV_ERR_OK;
}

uv_err_t UVDRelocationManager::applyPatch(UVDData **dataOut)
{
	return UV_DEBUG(applyPatchCore(dataOut, false));
}

uv_err_t UVDRelocationManager::applyPatchCore(UVDData **dataOut, bool useDefaultValue)
{
	std::vector<UVDData *> dataVector;

	for( std::vector<UVDRelocatableData *>::iterator iter = m_data.begin(); iter != m_data.end(); ++iter )
	{
		UVDRelocatableData *relocatableData = *iter;
		UVDData *data = NULL;

		uv_assert_ret(relocatableData);
		//Update the encapsulated data to reflect the relocations
		uv_assert_err_ret(relocatableData->applyRelocationsCore(useDefaultValue));
		
		//And rack up the raw data so it can be assembled
		data = relocatableData->m_data;
		uv_assert_ret(data);
		dataVector.push_back(data);
	}
	uv_assert_err_ret(UVDData::concatenate(dataVector, dataOut));
	
	return UV_ERR_OK;
}

UVDSimpleRelocationFixup::UVDSimpleRelocationFixup()
{
	m_relocationFixup = NULL;
	m_data = NULL;
}

uv_err_t UVDSimpleRelocationFixup::getUVDSimpleRelocationFixup(
		UVDSimpleRelocationFixup **simpleFixupOut, UVDRelocatableElement *relocatableElement,
		char *data, int size)
{
	UVDSimpleRelocationFixup *simpleFixup = NULL;
	UVDRelocationFixup *relocationFixup = NULL;
	//Apply at location
	int offset = 0;
	
	uv_assert_ret(relocatableElement);
	
	simpleFixup = new UVDSimpleRelocationFixup();
	uv_assert_ret(simpleFixup);

	relocationFixup = new UVDRelocationFixup(relocatableElement, offset, size);
	uv_assert_ret(relocationFixup);
	simpleFixup->m_relocationFixup = relocationFixup;
	
	//We need to transfer the buffer (as opposed to copy contents) since it needs to be applied to the real data
	uv_assert_err_ret(UVDDataMemory::getUVDDataMemoryByTransfer((UVDDataMemory **)&simpleFixup->m_data,
			//We are just applying patches, don't try to free the memory
			data, size, false));
	uv_assert_ret(simpleFixup->m_data);
	
	uv_assert_ret(simpleFixupOut);
	*simpleFixupOut = simpleFixup;
	return UV_ERR_OK;
}

UVDSimpleRelocationFixup::~UVDSimpleRelocationFixup()
{
}

uv_err_t UVDSimpleRelocationFixup::applyPatch()
{
	uv_assert_ret(m_relocationFixup);
	uv_assert_err_ret(m_relocationFixup->applyPatch(m_data));
	return UV_ERR_OK;
}
