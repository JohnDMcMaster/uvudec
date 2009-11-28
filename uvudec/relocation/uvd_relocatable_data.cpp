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

/*
UVDRelocatableData
*/

UVDRelocatableData::UVDRelocatableData()
{
	m_data = NULL;
	m_defaultRelocatableData = NULL;
}

UVDRelocatableData::UVDRelocatableData(UVDData *data)
{
	m_data = data;
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

