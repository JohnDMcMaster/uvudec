/*
UVNet Universal Decompiler (uvudec)
Copyright 2011 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVQT_SIMPLE_DYNAMIC_TEXT_H
#define UVQT_SIMPLE_DYNAMIC_TEXT_H

#include "uvqt/dynamic_text.h"

/*
An implementation that can rely solely on an offset and a constant index
*/

class UVQtSimpleDynamicTextData : public UVQtDynamicTextData
{
public:
	class iterator_impl : public UVQtDynamicTextData::iterator_impl
	{
	public:
		iterator_impl();
		iterator_impl(UVDGUIStringData *impl, unsigned int offset, unsigned int index = 0);
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
		UVQtSimpleDynamicTextData *m_dataImpl;
	};
	
public:
	UVDGUIStringData();
	
	//Return a subclass iterator so we can prep it for begin and end
	virtual UVQtSimpleDynamicTextData::iterator_impl *getIterator() = 0;
	virtual uv_err_t begin(unsigned int offset, unsigned int index, UVQtDynamicTextData::iterator *out);
	virtual uv_err_t end(iterator *out);
	virtual uv_err_t getMinOffset(unsigned int *out);
	virtual uv_err_t getMaxOffset(unsigned int *out);

public:
	//The pitch
	unsigned int m_indexesPerOffset;
	//Either hard set these or override the virtual funcs
	unsigned int m_minOffset;
	unsigned int m_maxOffset;
};

#endif

