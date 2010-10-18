/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_FLIRT_PATTERN_BFD_FUNCTION_PRINTER_H
#define UVD_FLIRT_PATTERN_BFD_FUNCTION_PRINTER_H

#include "uvd/util/types.h"

class UVDBFDPatFunctionPrinter
{
public:
	UVDBFDPatFunctionPrinter(UVDBFDPatFunction *func);

	void printRelocationByte();
	uv_err_t printLeadingBytes();
	uv_err_t printRelocations();
	//End will be first position that has a relocation from m_iter
	uv_err_t nextRelocation(UVDBFDPatFunction::const_iterator &end) const;
	uv_err_t printCRC();
	//start, end relative to function start
	//end is not inclusive
	uv_err_t printPatternBytes(UVDBFDPatFunction::const_iterator start, UVDBFDPatFunction::const_iterator end);
	uv_err_t printTailingBytes();
	uv_err_t print();
	uv_err_t shouldPrintFunction();

	UVDStringWriter *getStringWriter();

public:
	UVDBFDPatFunction *m_func;
	UVDBFDPatFunction::const_iterator m_iter;
};

#endif

