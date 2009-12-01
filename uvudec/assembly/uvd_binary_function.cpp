/*
Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#include "uvd_md5.h"
#include "uvd_binary_function.h"
#include "uvd_language.h"
#include "uvd_elf.h"

UVDBinaryFunctionInstance::UVDBinaryFunctionInstance()
{
	m_compiler = NULL;
	m_compilerOptions = NULL;
	m_data = NULL;
	m_relocatableData = NULL;

	m_language = UVD_LANGUAGE_UNKNOWN;
}

UVDBinaryFunctionInstance::~UVDBinaryFunctionInstance()
{
}

uv_err_t UVDBinaryFunctionInstance::init()
{
	m_relocatableData = new UVDRelocatableData();
	uv_assert_ret(m_relocatableData);
	
	return UV_ERR_OK;
}

uv_err_t UVDBinaryFunctionInstance::getUVDBinaryFunctionInstance(UVDBinaryFunctionInstance **out)
{
	UVDBinaryFunctionInstance *instance = NULL;
	
	instance = new UVDBinaryFunctionInstance();
	uv_assert_ret(instance);
	
	uv_assert_err_ret(instance->init());

	uv_assert_ret(out);
	*out = instance;
	return UV_ERR_OK;
}

void UVDBinaryFunctionInstance::setData(UVDData *data)
{
	m_data = data;
	if( m_relocatableData )
	{
		m_relocatableData->m_data = data;
	}
}

UVDData *UVDBinaryFunctionInstance::getData()
{
	return m_data;
}

uv_err_t UVDBinaryFunctionInstance::toUVDElf(UVDElf **out)
{
	uv_assert_ret(m_relocatableData);
	uv_assert_ret(m_relocatableData->m_data);
	return UV_DEBUG(UVDElf::getFromRelocatableData(m_relocatableData, m_symbolName, out));
}

uv_err_t UVDBinaryFunctionInstance::getFromUVDElf(const UVDElf *in, UVDBinaryFunctionInstance **out)
{
	return UV_DEBUG(UV_ERR_GENERAL);
}

uv_err_t UVDBinaryFunctionInstance::getHash(std::string &hash)
{
	if( m_MD5.empty() )
	{
		uv_assert_err_ret(computeHash());
	}
	hash = m_MD5;
	return UV_ERR_OK;
}

uv_err_t UVDBinaryFunctionInstance::getRelocatableHash(std::string &hash)
{
	if( m_relocatableMD5.empty() )
	{
		uv_assert_err_ret(computeRelocatableHash());
	}
	hash = m_relocatableMD5;
	return UV_ERR_OK;
}

uv_err_t UVDBinaryFunctionInstance::getRawDataBinary(UVDData **dataRet)
{
	//This data is suppose to be inherent
	//Simply see if it looks halfway reasonable and return it
	uv_assert_ret(m_data);
	uv_assert_ret(dataRet);
	*dataRet = m_data;
	return UV_ERR_OK;
}

uv_err_t UVDBinaryFunctionInstance::getRelocatableDataBinary(UVDData **dataRet)
{
	UVDData *data = NULL;

	uv_assert_ret(m_relocatableData);
	uv_assert_err_ret(m_relocatableData->getDefaultRelocatableData(&data));
	uv_assert_ret(dataRet);
	*dataRet = data;
	
	return UV_ERR_OK;
}

uv_err_t UVDBinaryFunctionInstance::computeHash()
{
	char *buff = NULL;
	uint32_t buffSize = 0;

	//Fetch our data
	uv_assert_ret(m_data);
	uv_assert_err_ret(m_data->readData(&buff));
	buffSize = m_data->size();

	//And compute an MD5
	uv_assert_err_ret(uv_md5(buff, buffSize, m_MD5));
	uv_assert_ret(!m_MD5.empty());
	free(buff);
	
	return UV_ERR_OK;
}

uv_err_t UVDBinaryFunctionInstance::computeRelocatableHash()
{
	char *buff = NULL;
	uint32_t buffSize = 0;
	UVDData *data = NULL;

	//Fetch our data
	uv_assert_err_ret(getRelocatableDataBinary(&data));
	uv_assert_ret(data);
	uv_assert_err_ret(data->readData(&buff));
	uv_assert_ret(buff);
	buffSize = m_data->size();

	//And compute an MD5
	uv_assert_err_ret(uv_md5(buff, buffSize, m_relocatableMD5));
	uv_assert_ret(!m_relocatableMD5.empty());
	//Finished, free buffer
	free(buff);
	
	return UV_ERR_OK;
}

UVDBinaryFunctionShared::UVDBinaryFunctionShared()
{
}

UVDBinaryFunction::UVDBinaryFunction()
{
	m_data = NULL;
	m_uvd = NULL;
	m_shared = NULL;
	m_offset = 0;
}

UVDBinaryFunction::~UVDBinaryFunction()
{
}
