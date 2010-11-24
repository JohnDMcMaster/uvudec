/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/object/object.h"

UVDObject::UVDObject()
{
	m_data = NULL;
}

UVDObject::~UVDObject()
{
	delete m_data;
	for( std::vector<UVDSection *>::iterator iter = m_sections.begin();
			iter != m_sections.end(); ++iter )
	{
		delete *iter;
	}
}
	
uv_err_t UVDObject::init(UVDData *data)
{
	m_data = data;
	return UV_ERR_OK;
}

uv_err_t UVDObject::addRelocation(UVDRelocationFixup *analysisRelocation)
{
	return UV_ERR_NOTSUPPORTED;
}

uv_err_t UVDObject::addFunction(UVDBinaryFunction *function)
{
	return UV_ERR_NOTSUPPORTED;
}

