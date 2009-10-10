/*
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
*/

#include "uv_md5.h"
#include "uvd_function.h"
#include "uvd_language.h"

UVDBinaryFunctionCodeShared::UVDBinaryFunctionCodeShared()
{
	m_compiler = NULL;
	m_compilerOptions = NULL;
	m_dataChunk = NULL;

	m_langauge = UVD_LANGUAGE_UNKNOWN;
}

uv_err_t UVDBinaryFunctionCodeShared::getHash(std::string &hash)
{
	if( m_MD5.empty() )
	{
		uv_assert_err_ret(computeHash());
	}
	hash = m_MD5;
	return UV_ERR_OK;
}

uv_err_t UVDBinaryFunctionCodeShared::computeHash()
{
	char *buff = NULL;
	uint32_t buffSize = 0;

	//Fetch our data
	uv_assert_ret(m_dataChunk);
	uv_assert_err_ret(m_dataChunk->readData(&buff));
	buffSize = m_dataChunk->size();

	//And compute an MD5
	uv_assert_err_ret(uv_md5(buff, buffSize, m_MD5));
	uv_assert_ret(!m_MD5.empty());
	free(buff);
	
	return UV_ERR_OK;
}

UVDBinaryFunctionShared::UVDBinaryFunctionShared()
{
}

UVDBinaryFunction::UVDBinaryFunction()
{
	m_dataChunk = NULL;
	m_uvd = NULL;
	m_shared = NULL;
}


