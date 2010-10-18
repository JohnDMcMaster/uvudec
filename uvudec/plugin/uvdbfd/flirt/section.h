/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_FLIRT_PATTERN_BFD_SECTION_H
#define UVD_FLIRT_PATTERN_BFD_SECTION_H

#include "uvdbfd/flirt/function.h"
#include "uvd/util/types.h"

/*
UVDBFDPatSection
*/
class UVDBFDPatCore;
class UVDBFDPatSection
{
public:
	UVDBFDPatSection();
	~UVDBFDPatSection();

	uv_err_t trimSignatures();
	uv_err_t addFunction(asymbol *functionSymbol);
	uv_err_t setFunctionSizes();
	uv_err_t assignRelocation(arelent *bfdRelocation);
	uv_err_t print();

public:
	//Do we own this?
	struct bfd_section *m_section;
	UVDBFDPatFunctions m_functions;
	//We own this
	unsigned char *m_content;
	//Do not own this
	UVDBFDPatCore *m_core;
};

/*
UVDBFDPatSections
*/
class UVDBFDPatSections
{
public:
	UVDBFDPatSections();
	~UVDBFDPatSections();

	uv_err_t add(UVDBFDPatFunction *function);
	uv_err_t add(UVDBFDPatSection *uvdSection);
	uv_err_t find(asection *bfdAsection, UVDBFDPatSection **uvdSectionOut);
	uv_err_t get(struct bfd_section *bfdSectionIn, uint32_t sectionSize, UVDBFDPatSection **uvdSectionOut);
	uv_err_t print();

public:
	//Currently no ordering is imposed on this
	std::vector<UVDBFDPatSection *> m_sections;
	//Do not own this
	UVDBFDPatCore *m_core;
};

#endif

