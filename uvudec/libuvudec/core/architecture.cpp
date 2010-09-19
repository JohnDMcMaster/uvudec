/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "core/architecture.h"

UVDArchitecture::UVDArchitecture()
{
#ifdef USING_VECTORS
	m_CPU = NULL;
#endif
	m_opcodeTable = NULL;
	m_symMap = NULL;
	m_interpreter = NULL;
	m_architecture = 0;
}

UVDArchitecture::~UVDArchitecture()
{
	UV_DEBUG(deinit());
}

uv_err_t UVDArchitecture::init()
{
	return UV_ERR_OK;
}

uv_err_t UVDArchitecture::deinit()
{
	delete m_opcodeTable;
	m_opcodeTable = NULL;

	delete m_symMap;
	m_symMap = NULL;

	delete m_interpreter;
	m_interpreter = NULL;

	for( std::map<std::string, UVDRegisterShared *>::iterator iter = m_registers.begin(); iter != m_registers.end(); ++iter )
	{
		UVDRegisterShared *regShared = (*iter).second;

		if( !regShared )
		{
			printf_warn("bad regShared entry\n");
		}
		else
		{
			delete regShared;
		}
	}
	m_registers.clear();

	return UV_ERR_OK;
}


