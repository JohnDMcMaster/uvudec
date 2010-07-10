/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#include "uvd_relocation.h"
	
/*
UVDRelocatableDataPlaceholder
*/

class UVDRelocatableDataPlaceholder : public UVDRelocatableData
{
public:
	UVDRelocatableDataPlaceholder();
	~UVDRelocatableDataPlaceholder();
	virtual uv_err_t deinit();

	uv_err_t setData(UVDData *data);
	uv_err_t transferData(UVDData *data, uint32_t freeDataAtDestruction);
	
public:
	//Statically allocated data structure
	//Empty data structure opreations to make other operations work nicely
	UVDDataPlaceholder m_dataPlaceholder;
};

UVDRelocatableDataPlaceholder::UVDRelocatableDataPlaceholder()
{
	//This is statically allocated, should not be freed
	//FIXME: this was probably was was responsible for UVDRelocatableData desturctor causing crashes
	m_freeDataAtDestruction = FALSE;
	m_data = &m_dataPlaceholder;
	m_defaultRelocatableData = &m_dataPlaceholder;
}

UVDRelocatableDataPlaceholder::~UVDRelocatableDataPlaceholder()
{
}

uv_err_t UVDRelocatableDataPlaceholder::deinit()
{
	//These were not dynamically allocated, so don't try to free them
	m_data = NULL;
	m_defaultRelocatableData = NULL;
	
	uv_assert_err_ret(UVDRelocatableData::deinit());
	return UV_ERR_OK;
}

uv_err_t UVDRelocatableDataPlaceholder::setData(UVDData *data)
{
	//A placeholder has no data
	return UV_DEBUG(UV_ERR_NOTSUPPORTED);
}

uv_err_t UVDRelocatableDataPlaceholder::transferData(UVDData *data, uint32_t freeDataAtDestruction)
{
	return UV_DEBUG(UV_ERR_NOTSUPPORTED);
}

/*
UVDRelocatableData
*/

UVDRelocatableData::UVDRelocatableData()
{
	m_data = NULL;
	m_freeDataAtDestruction = TRUE;
	m_defaultRelocatableData = NULL;
}

/*
UVDRelocatableData::UVDRelocatableData(UVDData *data)
{
	m_data = data;
	m_defaultRelocatableData = NULL;
}
*/

UVDRelocatableData::~UVDRelocatableData()
{
	deinit();
}

uv_err_t UVDRelocatableData::deinit()
{
//FIXME: figure out double free issues
return UV_ERR_OK;
	for( std::set<UVDRelocationFixup *>::iterator iter = m_fixups.begin(); iter != m_fixups.end(); ++iter )
	{
		delete *iter;
	}
	m_fixups.clear();

	if( m_freeDataAtDestruction )
	{
		delete m_data;
	}
	m_data = NULL;
	
	delete m_defaultRelocatableData;
	m_defaultRelocatableData = NULL;

	return UV_ERR_OK;
}

uv_err_t UVDRelocatableData::getUVDRelocatableDataPlaceholder(UVDRelocatableData **dataOut)
{
	UVDRelocatableData *data = NULL;
	
	data = new UVDRelocatableDataPlaceholder();
	uv_assert_ret(data);

	uv_assert_ret(dataOut);
	*dataOut = data;

	return UV_ERR_OK;
}

uv_err_t UVDRelocatableData::isEmpty(uint32_t *isEmpty)
{
	uv_assert_ret(isEmpty);
	if( m_data )
	{
		*isEmpty = false;
	}
	else
	{
		*isEmpty = true;
	}
	return UV_ERR_OK;
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

uv_err_t UVDRelocatableData::addFixup(UVDRelocationFixup *fixup)
{
	m_fixups.insert(fixup);
	return UV_ERR_OK;
}

uv_err_t UVDRelocatableData::getRelocatableData(UVDData **data)
{
	uv_assert_ret(data);
	uv_assert_err_ret(updateData());
	//Why commented out?
	//uv_assert_ret(m_data);
	*data = m_data;
	return UV_ERR_OK;
}

uv_err_t UVDRelocatableData::getDefaultRelocatableData(UVDData **data)
{
	uv_assert_ret(data);

	if( !m_defaultRelocatableData && m_data )
	{
		uv_assert_err_ret(updateDefaultRelocatableData());
		uv_assert_ret(m_defaultRelocatableData);
	}
	*data = m_defaultRelocatableData;
	return UV_ERR_OK;
}

uv_err_t UVDRelocatableData::updateData()
{
	return UV_ERR_OK;
}

uv_err_t UVDRelocatableData::updateDefaultRelocatableData()
{
	UVDData *data = NULL;
	
	//Copy our data as a base
	uv_assert_err_ret(getRelocatableData(&data));
	uv_assert_ret(data);
	uv_assert_err_ret(UVDDataMemory::getUVDDataMemoryByCopy(data, &m_defaultRelocatableData));
	uv_assert_ret(m_defaultRelocatableData);
	
	//And fixup (zero) all the relocation places
	for( std::set<UVDRelocationFixup *>::iterator iter = m_fixups.begin(); iter != m_fixups.end(); ++iter )
	{
		UVDRelocationFixup *fixup = *iter;
		uv_assert_ret(fixup);
		uv_assert_err_ret(fixup->applyPatchCore(m_defaultRelocatableData, true));
	}
	return UV_ERR_OK;
}

uv_err_t UVDRelocatableData::setData(UVDData *data)
{
	/*
	Since we should own this data object, we need to delete old if present and make a deep copy
	*/
	if( m_freeDataAtDestruction )
	{
		delete m_data;
	}
	m_data = NULL;
	//This is invalidated and will have to be regen
	delete m_defaultRelocatableData;
	m_defaultRelocatableData = NULL;
	
	if( data )
	{
		uv_assert_err_ret(data->deepCopy(&m_data));
	}

	return UV_ERR_OK;
}

uv_err_t UVDRelocatableData::transferData(UVDData *data, uint32_t freeDataAtDestruction)
{
	if( m_freeDataAtDestruction )
	{
		delete m_data;
	}
	m_data = data;
	m_freeDataAtDestruction = freeDataAtDestruction;

	return UV_ERR_OK;
}

void UVDRelocatableData::hexdump()
{
	UVDData *data = NULL;
	
	if( UV_FAILED(getRelocatableData(&data)) )
	{
		printf_debug("Could not get data\n");
		return;
	}
	data->hexdump();
}

void UVDRelocatableData::hexdumpDefault()
{
	UVDData *data = NULL;
	
	if( UV_FAILED(getDefaultRelocatableData(&data)) )
	{
		printf_debug("Could not get data\n");
		return;
	}
	data->hexdump();
}

uv_err_t UVDRelocatableData::size(uint32_t *sizeOut)
{
	UVDData *data = NULL;
	
	uv_assert_err_ret(getRelocatableData(&data));
	uv_assert_ret(sizeOut);
	if( data )
	{
		uv_assert_err_ret(data->size(sizeOut));
	}
	else
	{
		*sizeOut = 0;
	}

	return UV_ERR_OK;
}

bool UVDRelocatableData::requiresDataSync()
{
	return true;
}

/*
UVDMultiRelocatableData
*/

UVDMultiRelocatableData::UVDMultiRelocatableData()
{
}

UVDMultiRelocatableData::~UVDMultiRelocatableData()
{
}

uv_err_t UVDMultiRelocatableData::setData(UVDData *data)
{
	//Not needed/supported for now
	printf_error("setData() not supported for UVDMultiRelocatableData\n");
	return UV_DEBUG(UV_ERR_GENERAL);
}

uv_err_t UVDMultiRelocatableData::updateData()
{
	std::vector<UVDData *> datas;

	//printf("\n\nMulti update on 0x%.8X\n", (unsigned int)this);
	for( std::vector<UVDRelocatableData *>::iterator iter = m_relocatableDatas.begin();
			iter != m_relocatableDatas.end(); ++iter )
	{
		UVDRelocatableData *relocatableData = *iter;
		UVDData *data = NULL;
		

		uv_assert_ret(relocatableData);
		uv_assert_err_ret(relocatableData->getRelocatableData(&data));
		datas.push_back(data);
		//printf("\tMulti update on relocatable data 0x%.8X, data: 0x%.8X\n", (unsigned int)relocatableData, (unsigned int)data);
	}
	
	//FIXME: we should probably delete the old m_data, realloc, copy or something it
	uv_assert_err_ret(UVDData::concatenate(datas, &m_data));
	//printf("multi update\n");	
	return UV_ERR_OK;
}

uv_err_t UVDMultiRelocatableData::updateDefaultRelocatableData()
{
	std::vector<UVDData *> datas;

	for( std::vector<UVDRelocatableData *>::iterator iter = m_relocatableDatas.begin();
			iter != m_relocatableDatas.end(); ++iter )
	{
		UVDRelocatableData *relocatableData = *iter;
		UVDData *data = NULL;
		
		uv_assert_ret(relocatableData);
		uv_assert_err_ret(relocatableData->getDefaultRelocatableData(&data));
		datas.push_back(data);
	}
	
	//FIXME: we should probably delete the old m_defaultRelocatableData, realloc, copy or something it
	uv_assert_err_ret(UVDData::concatenate(datas, &m_defaultRelocatableData));
	
	return UV_ERR_OK;
}

uv_err_t UVDMultiRelocatableData::addFixup(UVDRelocationFixup *)
{
	printf_error("addFixup() not supported for UVDMultiRelocatableData\n");
	//Not needed/supported for now	
	return UV_DEBUG(UV_ERR_GENERAL);
}

uv_err_t UVDMultiRelocatableData::applyRelocationsCore(bool useDefaultValue)
{
	for( std::vector<UVDRelocatableData *>::iterator iter = m_relocatableDatas.begin();
			iter != m_relocatableDatas.end(); ++iter )
	{
		UVDRelocatableData *relocatableData = *iter;
		
		uv_assert_ret(relocatableData);
		uv_assert_err_ret(relocatableData->applyRelocationsCore(useDefaultValue));
	}
	
	return UV_ERR_OK;
}

uv_err_t UVDMultiRelocatableData::isEmpty(uint32_t *isEmpty)
{
	uv_assert_ret(isEmpty);
	*isEmpty = m_relocatableDatas.empty();
	return UV_ERR_OK;
}

bool UVDMultiRelocatableData::requiresDataSync()
{
	return false;
}

