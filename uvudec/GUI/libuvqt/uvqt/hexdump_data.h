/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVQT_HEXDUMP_DATA_H
#define UVQT_HEXDUMP_DATA_H

#include "uvqt/dynamic_text.h"
#include "uvd/data/data.h"

class UVQtHexdumpData : public UVQtDynamicTextData
{
public:
	class iterator_impl : public UVQtDynamicTextData::iterator_impl
	{
	public:
		iterator_impl();
		//index can probably be dropped, but good for consistency
		iterator_impl(UVQtHexdumpData *impl, unsigned int offset, unsigned int index = 0);
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
		
		char read(unsigned int offset);
		unsigned int size();

		unsigned int hexdumpHalfRow(uint32_t start, std::string &ret);
		void hexdump(const uint8_t *data, size_t size);
		unsigned int scrollbarPositionToOffset(unsigned int offset);
		int scrollbarPositionDeltaToOffsetDelta(int offset);
		unsigned int maxValidOffset();

	public:
		//Represented as data offset, NOT lines in scroll view
		//Will be much more stable if we resize window and such
		//we have to translate each input though
		unsigned int m_offset;
		UVQtHexdumpData *m_dataImpl;
	};
	
public:
	UVQtHexdumpData();
	
	virtual uv_err_t begin(unsigned int offset, unsigned int index, UVQtDynamicTextData::iterator *out);
	virtual uv_err_t end(iterator *out);
	virtual uv_err_t getMinOffset(unsigned int *out);
	virtual uv_err_t getMaxOffset(unsigned int *out);

public:
	unsigned int m_bytesPerRow;
	unsigned int m_bytesPerSubRow;
	unsigned int m_numberRows;
	//Target data
	UVDData *m_data;
};

#endif

