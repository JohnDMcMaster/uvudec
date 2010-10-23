/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "bfd.h"
#include "uvdbfd/object.h"
#include <typeinfo>

uv_err_t UVDBFDObject::canLoad(const UVDData *data, const UVDRuntimeHints &hints, uvd_priority_t *confidence)
{
	uv_err_t rc = UV_ERR_GENERAL;
	bfd *abfd = NULL;
	std::string file;
	UVDDataFile *dataFile = NULL;
	
	uv_assert_ret(data);
	if( typeid(data) != typeid(UVDDataFile) )
	{
		return UV_ERR_NOTSUPPORTED;
	}
	dataFile = (UVDDataFile *)data;
	file = dataFile->m_sFile;

	//If its a junk file, don't think we get a good handle
	abfd = bfd_openr(file.c_str(), "default");
	if( abfd == NULL )
	{
		printf_error("Could not open file <%s>\n", file.c_str());
		return UV_ERR_GENERAL;
	}

	//Needs to be an object...maybe an archive?	
	if( bfd_check_format(abfd, bfd_object) == TRUE
			|| bfd_check_format(abfd, bfd_archive) == TRUE )
	{
		rc = UV_ERR_OK;
	}
	else
	{
		rc = UV_ERR_NOTSUPPORTED;
	}
	bfd_close(abfd);
printf_debug("rc %d\n", rc);

	return rc;
}

uv_err_t UVDBFDObject::tryLoad(UVDData *data, const UVDRuntimeHints &hints, UVDObject **out)
{
	UVDBFDObject *ret = NULL;
	uv_err_t rc;
	
	ret = new UVDBFDObject();
	uv_assert_ret(ret);
	rc = UV_DEBUG(ret->init(data));
	if( UV_FAILED(rc) )
	{
		delete ret;
		return rc;
	}
	
	uv_assert_ret(out);
	*out = ret;
	
	return UV_ERR_OK;
}

UVDBFDObject::UVDBFDObject()
{
	m_bfd = NULL;
}

UVDBFDObject::~UVDBFDObject()
{
	if( m_bfd )
	{
		bfd_close(m_bfd);
		m_bfd = NULL;
	}
}

uv_err_t UVDBFDObject::init(UVDData *data)
{
	std::string file;
	UVDDataFile *dataFile = NULL;

	uv_assert_err_ret(UVDObject::init(data));

	if( typeid(*data) != typeid(UVDDataFile) )
	{
		return UV_DEBUG(UV_ERR_NOTSUPPORTED);
	}
	dataFile = (UVDDataFile *)data;
	file = dataFile->m_sFile;

	//FIXME: make default bfd architecture a command line option
	m_bfd = bfd_openr(file.c_str(), "default");
	if( m_bfd == NULL )
	{
		printf_error("Could not open file <%s>\n", file.c_str());
		return UV_DEBUG(UV_ERR_GENERAL);
	}

	//Needs to be an object or an archive
	if( !bfd_check_format(m_bfd, bfd_object)
			&& !bfd_check_format(m_bfd, bfd_archive) )
	{
		bfd_close(m_bfd);
		return UV_DEBUG(UV_ERR_NOTSUPPORTED);
	}
	
	//TODO: we should build the section table
	uv_assert_err_ret(rebuildSections());

	return UV_ERR_OK;
}

uv_err_t UVDBFDObject::rebuildSections()
{
	//FIXME: implement this
	return UV_ERR_OK;
}

