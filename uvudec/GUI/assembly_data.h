/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVDGUI_ASSEMBLY_DATA_H
#define UVDGUI_ASSEMBLY_DATA_H

#include "uvqt/dynamic_text.h"
#include "uvd/core/iterator.h"
#include "uvd/data/data.h"

/*
This functionality uses the UVD engine
It is expected to be locked externally
*/

class UVDGUIAssemblyData : public UVQtDynamicTextData
{
public:
	class iterator_impl : public UVQtDynamicTextData::iterator_impl
	{
	public:
		iterator_impl();
		iterator_impl(UVDGUIAssemblyData *impl, unsigned int offset, unsigned int index = 0);
		iterator_impl(UVDGUIAssemblyData *impl, UVDPrintIterator iter);
		~iterator_impl();
	
		virtual UVQtDynamicTextData::iterator_impl *copy();
		virtual uv_err_t get(std::string &ret);
		virtual uv_err_t previous();
		virtual uv_err_t next();
		virtual uv_err_t changePositionByLineDelta(int delta);
		virtual uv_err_t changePositionToLine(unsigned int offset, unsigned int index);	
		virtual int compare(const UVQtDynamicTextData::iterator_impl *other);
		virtual std::string toString();
		virtual unsigned int offset();
		
	public:
		UVDPrintIterator m_iter;
		UVDGUIAssemblyData *m_dataImpl;
	};
	
public:
	UVDGUIAssemblyData();
	
	UVD *getUVD();
	UVDAddressSpace *getAddressSpace();
	
	virtual uv_err_t begin(unsigned int offset, unsigned int index, UVQtDynamicTextData::iterator *out);
	virtual uv_err_t end(iterator *out);
	virtual uv_err_t getMinOffset(unsigned int *out);
	virtual uv_err_t getMaxOffset(unsigned int *out);

public:
};

#endif

