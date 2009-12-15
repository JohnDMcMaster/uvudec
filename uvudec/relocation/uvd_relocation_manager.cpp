/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#include "uvd_relocation.h"

/*
UVDRelocationManager
*/

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
		if( UV_FAILED(relocatableData->applyRelocationsCore(useDefaultValue)) )
		{
			/*
			if( relocatableData->m_data )
			{
				printf_error("could not apply relocation %s\n", relocatableData->m_data->getSource().c_str());
			}
			*/
			return UV_DEBUG(UV_ERR_GENERAL);
		}
		
		//And rack up the raw data so it can be assembled
		uv_assert_err_ret(relocatableData->getRelocatableData(&data));
		uv_assert_ret(data);
		dataVector.push_back(data);
	}
	uv_assert_err_ret(UVDData::concatenate(dataVector, dataOut));
	
	return UV_ERR_OK;
}

