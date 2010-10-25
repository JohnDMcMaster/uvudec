/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/hash/crc.h"
#include "uvd/util/util.h"
#include "uvd/flirt/flirt.h"
#include "uvdbfd/flirt/function.h"
#include "uvdbfd/flirt/module.h"
#include "uvdbfd/flirt/relocation.h"
#include "uvdbfd/flirt/section.h"
#include "uvdbfd/flirt/core.h"

/*
UVDBFDPatFunction
*/

UVDBFDPatFunction::UVDBFDPatFunction()
{
	m_bfdAsymbol = NULL;
	m_offset = 0;
	m_size = 0;
	m_module = NULL;
}

UVDBFDPatFunction::~UVDBFDPatFunction()
{
}

uv_err_t UVDBFDPatFunction::getEffectiveEndPosition(uint32_t *endPos, uint32_t allowRelocations)
{
	//Looks like this isn't being used
	return UV_DEBUG(UV_ERR_GENERAL);
#if 0
	//TODO: find out what IDA does if we have a relocation on the end
	//I don't think we should trim off end relocations as even an ending wildcard IS part of a match
	//No relocations implies we end when its over
	if( allowRelocations || m_module->m_relocations.m_relocations.empty() )
	{
		*endPos = m_size;
	}
	else
	{
		//Keep going while we don't hit relocations
		*endPos = m_module->m_relocations.m_relocations[m_module->m_relocations.m_relocations.size() - 1]->m_offset;
	}
	return UV_ERR_OK;
#endif
}

uint8_t UVDBFDPatFunction::readByte(uint32_t offset)
{
	//Offset into section + offset within section
	return m_module->m_section->m_content[m_offset + offset];
}

