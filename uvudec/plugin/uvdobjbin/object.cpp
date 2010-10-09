/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvdobjbin/object.h"

UVDBinaryObject::UVDBinaryObject()
{
}

UVDBinaryObject::~UVDBinaryObject()
{
	//Since we directly map the data into the section, prevent double free
	if( !m_sections.empty() )
	{
		m_data = NULL;
	}
}

uv_err_t UVDBinaryObject::init(UVDData *data)
{
	uv_assert_err_ret(UVDObject::init(data));
	//We have a single section, a raw binary blob
	UVDSection *section = NULL;

	section = new UVDSection();
	uv_assert_ret(section);
	section->m_data = data;
	
	//Basic assumptions for a ROM image.  W is probably most debatable as we could be in flash or ROM
	section->m_R = UVD_TRI_TRUE;
	section->m_W = UVD_TRI_FALSE;
	section->m_X = UVD_TRI_TRUE;
	
	m_sections.push_back(section);
	
	return UV_ERR_OK;
}

uv_err_t UVDBinaryObject::canLoad(const UVDData *data, const std::string &object, const std::string &architecture, uvd_priority_t *confidence)
{
	//While this may work, likely its not a good loader and should be a last resort
	*confidence = UVD_MATCH_POOR;
	return UV_ERR_OK;
}

uv_err_t UVDBinaryObject::tryLoad(UVDData *data, const std::string &object, const std::string &architecture, UVDObject **out)
{
	UVDBinaryObject *binaryObject = NULL;
	
	binaryObject = new UVDBinaryObject();
	uv_assert_ret(binaryObject);
	uv_assert_err_ret(binaryObject->init(data));
	
	uv_assert_ret(out);
	*out = binaryObject;
	return UV_ERR_OK;
}

