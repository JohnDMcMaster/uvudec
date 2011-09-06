/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVDBFD_OBJECT_H
#define UVDBFD_OBJECT_H

#include "bfd.h"
#include "uvd/object/object.h"

class UVDBFDSection : public UVDSection {
public:
	UVDBFDSection();
	~UVDBFDSection();

	virtual uv_err_t toAddressSpace(UVDAddressSpace **out);

public:
	asection *m_section;
	//UVDAddressSpace *m_addressSpace;
};

class UVDBFDObject : public UVDObject
{
public:
	static uv_err_t canLoad(const UVDData *data, const UVDRuntimeHints &hints, uvd_priority_t *confidence, void *user);
	static uv_err_t tryLoad(UVDData *data, const UVDRuntimeHints &hints, UVDObject **out, void *user);

public:
	UVDBFDObject();
	virtual ~UVDBFDObject();
	
	virtual uv_err_t init(UVDData *data);
	//Rebuild section table off of m_bfd
	uv_err_t rebuildSections();

	//Return UV_ERR_NOTFOUND if it doesn't match
	uv_err_t sectionToSection( asection *section, UVDBFDSection **out );
	uv_err_t bfdSectionToAddressSpace( asection *section, UVDAddressSpace **out );
	uv_err_t sectionToAddressSpace( UVDBFDSection *section, UVDAddressSpace **out );
	uv_err_t addressSpaceToSection( UVDAddressSpace *addressSpace, UVDBFDSection **out );
	uv_err_t addressSpaceToBfdSection( UVDAddressSpace *addressSpace, asection **out );
	
public:
	bfd *m_bfd;
	
	std::map<asection *, UVDBFDSection *> m_sectionsToSections;
	//Not necessarily equal, each address space doesn't necessarily map to a section
	//and each section doesn't necessarily have an address space (ex: debug info)
	//std::map<asection *, UVDAddressSpace *> m_sectionsToAddressSpaces;
	std::map<UVDBFDSection *, UVDAddressSpace *> m_sectionsToAddressSpaces;
	//std::map<UVDAddressSpace *, asection *> m_addressSpacesToSections;
	std::map<UVDAddressSpace *, UVDBFDSection *> m_addressSpacesToSections;
};

#endif

