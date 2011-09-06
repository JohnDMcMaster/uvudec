/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "bfd.h"
#include "uvdbfd/object.h"
#include <typeinfo>
#include <string.h>


UVDBFDSection::UVDBFDSection() {
	m_section = NULL;
	m_addressSpace = NULL;
}

UVDBFDSection::~UVDBFDSection() {
}

uv_err_t UVDBFDObject::canLoad(const UVDData *data, const UVDRuntimeHints &hints, uvd_priority_t *confidence, void *user)
{
	bfd *abfd = NULL;
	std::string file;
	UVDDataFile *dataFile = NULL;
	
	uv_assert_ret(data);
	uv_assert_ret(confidence);
	if( typeid(*data) != typeid(UVDDataFile) )
	{
		printf_plugin_debug("not UVDDataFile, is %s\n", typeid(data).name());
		*confidence = UVD_MATCH_NONE;
		return UV_ERR_OK;
	}
	dataFile = (UVDDataFile *)data;
	file = dataFile->m_sFile;

	//If its a junk file, don't think we get a good handle
	abfd = bfd_openr(file.c_str(), "default");
	if( abfd == NULL )
	{
		printf_plugin_debug("Could not open file <%s>\n", file.c_str());
		*confidence = UVD_MATCH_NONE;
		return UV_ERR_OK;
	}

	//Needs to be an object...maybe an archive?	
	if( bfd_check_format(abfd, bfd_object) == TRUE
			|| bfd_check_format(abfd, bfd_archive) == TRUE )
	{
		printf_plugin_debug("good format\n");
		*confidence = UVD_MATCH_ACCEPTABLE;
	}
	else
	{
		printf_plugin_debug("bad format\n");
		*confidence = UVD_MATCH_NONE;
	}
	bfd_close(abfd);

	return UV_ERR_OK;
}

uv_err_t UVDBFDObject::tryLoad(UVDData *data, const UVDRuntimeHints &hints, UVDObject **out, void *user)
{
	uv_err_t rc = UV_ERR_GENERAL;
	UVDBFDObject *ret = NULL;
	
	ret = new UVDBFDObject();
	uv_assert_ret(ret);
	uv_assert_err(ret->init(data));
	
	uv_assert_ret(out);
	*out = ret;
	
	return UV_ERR_OK;

error:
	delete ret;
	return UV_DEBUG(rc);
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
	uv_err_t rc = UV_ERR_GENERAL;

	uv_assert_ret(data);
	if( typeid(*data) != typeid(UVDDataFile) )
	{
		return UV_DEBUG(UV_ERR_NOTSUPPORTED);
	}

	uv_assert_err(UVDObject::init(data));

	dataFile = (UVDDataFile *)data;
	file = dataFile->m_sFile;

	//FIXME: make default bfd architecture a command line option
	m_bfd = bfd_openr(file.c_str(), "default");
	if( m_bfd == NULL )
	{
		printf_error("Could not open file <%s>\n", file.c_str());
		rc = UV_DEBUG(UV_ERR_GENERAL);
		goto error;
	}

	//Needs to be an object or an archive
	if( !bfd_check_format(m_bfd, bfd_object)
			&& !bfd_check_format(m_bfd, bfd_archive) )
	{
		bfd_close(m_bfd);
		rc = UV_DEBUG(UV_ERR_NOTSUPPORTED);
		goto error;
	}
	
	//TODO: we should build the section table
	uv_assert_err(rebuildSections());

	return UV_ERR_OK;

error:
	m_data = NULL;
	return UV_DEBUG(rc);
}


uv_err_t UVDBFDSection::toAddressSpace(UVDAddressSpace **out) {
	//printf("to address space\n");
	return UV_DEBUG(UVDSection::toAddressSpace(out));
}


uv_err_t UVDBFDObject::rebuildSections()
{
	asection *bfdAsection = NULL;
	
	printf("rebuilding sections\n");
	
	//XXX: is this the right place to do this?
	//also this is a memory leak...
	//for now expect that this is only called once
	m_sectionsToAddressSpaces.clear();
	m_addressSpacesToSections.clear();
	//m_addressSpaces.m_addressSpaces.clear();
	m_sectionsToSections.clear();
	
	for( bfdAsection = m_bfd->sections; bfdAsection != NULL; bfdAsection = bfdAsection->next ) {
		UVDBFDSection *section = NULL;

		printf("Processing section %s\n", bfdAsection->name );
		
		section = new UVDBFDSection();
		uv_assert_ret(section);
		section->m_section = bfdAsection;
		m_sectionsToSections[bfdAsection] = section;
		
		//Can this be NULL?
		uv_assert_ret(bfdAsection->name);
		section->m_name = bfdAsection->name;
		
		section->m_R = UVD_TRI_TRUE;
		section->m_W = UVD_TRI_FALSE;
		if( !strcmp(".text", bfdAsection->name) ) {
			section->m_X = UVD_TRI_TRUE;
	
			//This is the only section we really care about for now
			UVDAddressSpace *addressSpace = NULL;
			
			uv_assert_err_ret(section->toAddressSpace(&addressSpace));
			m_sectionsToAddressSpaces[section] = addressSpace;
			m_addressSpacesToSections[addressSpace] = section;
			//m_addressSpaces.m_addressSpaces.push_back(addressSpace);
		} else {
			section->m_X = UVD_TRI_FALSE;		
		}
		
		m_sections.push_back(section);
	}

	return UV_ERR_OK;
}

uv_err_t UVDBFDObject::sectionToSection( asection *section, UVDBFDSection **out ) {
	std::map<asection *, UVDBFDSection *>::iterator iter;
	
	uv_assert_ret(section);
	uv_assert_ret(out);
	
	iter = m_sectionsToSections.find(section);
	if( iter == m_sectionsToSections.end() ) {
		return UV_ERR_NOTFOUND;
	}
	*out = (*iter).second;
	
	return UV_ERR_OK;
}

uv_err_t UVDBFDObject::bfdSectionToAddressSpace( asection *bfdSection, UVDAddressSpace **out ) {
	uv_err_t rcTemp = UV_ERR_GENERAL;
	UVDBFDSection *section = NULL;
	
	rcTemp = sectionToSection( bfdSection, &section );
	uv_assert_err_ret(rcTemp);
	if (rcTemp == UV_ERR_NOTFOUND) {
		return rcTemp;
	}
	uv_assert_err_ret(rcTemp);
	
	printf("matched to section %s / %s\n", section->m_section->name, section->m_name.c_str() );
	
	rcTemp = sectionToAddressSpace( section, out );
	uv_assert_err_ret(rcTemp);
	if (rcTemp == UV_ERR_NOTFOUND) {
		return rcTemp;
	}
	uv_assert_err_ret(rcTemp);
	
	return UV_ERR_OK;
}

uv_err_t UVDBFDObject::sectionToAddressSpace( UVDBFDSection *section, UVDAddressSpace **out ) {
	std::map<UVDBFDSection *, UVDAddressSpace *>::iterator iter;
	
	uv_assert_ret(section);
	uv_assert_ret(out);

	iter = m_sectionsToAddressSpaces.find(section);
	if( iter == m_sectionsToAddressSpaces.end() ) {
		return UV_ERR_NOTFOUND;
	}
	*out = (*iter).second;

	return UV_ERR_OK;
}

uv_err_t UVDBFDObject::addressSpaceToSection( UVDAddressSpace *addressSpace, UVDBFDSection **out ) {
	std::map<UVDAddressSpace *, UVDBFDSection *>::iterator iter;
	
	uv_assert_ret(addressSpace);
	uv_assert_ret(out);

	iter = m_addressSpacesToSections.find( addressSpace );
	if( iter == m_addressSpacesToSections.end() ) {
		return UV_ERR_NOTFOUND;
	}
	*out = (*iter).second;

	return UV_ERR_OK;
}

uv_err_t UVDBFDObject::addressSpaceToBfdSection( UVDAddressSpace *addressSpace, asection **out ) {
	uv_err_t rcTemp = UV_ERR_GENERAL;
	UVDBFDSection *section = NULL;
	
	rcTemp = UVDBFDObject::addressSpaceToSection( addressSpace, &section );
	if (rcTemp == UV_ERR_NOTFOUND) {
		return rcTemp;
	}
	uv_assert_err_ret(rcTemp);
	
	uv_assert_ret( out );
	*out = section->m_section;
	
	return UV_ERR_OK;
}

