/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVQT_DYNAMIC_TEXT_PLUGIN_IMPL_H
#define UVQT_DYNAMIC_TEXT_PLUGIN_IMPL_H

#include "uvqt/dynamic_text.h"

class UVQtDynamicTextDataPluginImpl : public UVQtDynamicTextData
{
public:
	class iterator_impl : public UVQtDynamicTextData::iterator_impl
	{
	public:
		iterator_impl();
		iterator_impl(UVQtDynamicTextDataPluginImpl *impl, unsigned int offset, unsigned int index);
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
		unsigned int m_offset;
		unsigned int m_index;
		UVQtDynamicTextDataPluginImpl *m_dataImpl;
	};
	
public:
	UVQtDynamicTextDataPluginImpl();
	
	virtual uv_err_t begin(unsigned int offset, unsigned int index, UVQtDynamicTextData::iterator *out);
	virtual uv_err_t end(iterator *out);
	virtual uv_err_t getMinOffset(unsigned int *out);
	virtual uv_err_t getMaxOffset(unsigned int *out);

public:
	//How many elements per offset
	unsigned int m_numberOffsetIndexes;
	unsigned int m_offsetMin;
	unsigned int m_offsetMax;
};

#endif

