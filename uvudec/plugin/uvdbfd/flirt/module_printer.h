/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_FLIRT_PATTERN_BFD_MODULE_PRINTER_H
#define UVD_FLIRT_PATTERN_BFD_MODULE_PRINTER_H

#include "uvd/util/types.h"
#include "uvdbfd/flirt/module.h"
//#include "uvdbfd/flirt/function.h"

class UVDBFDPatFunction;
class UVDBFDPatModulePrinter
{
public:
	UVDBFDPatModulePrinter(UVDBFDPatModule *module);

	void printRelocationByte();
	uv_err_t printLeadingBytes();
	uv_err_t printRelocations();
	//End will be first position that has a relocation from m_iter
	uv_err_t nextRelocation(UVDBFDPatModule::const_iterator &end) const;
	uv_err_t printCRC();
	//start, end relative to function start
	//end is not inclusive
	uv_err_t printPatternBytes(UVDBFDPatModule::const_iterator start, UVDBFDPatModule::const_iterator end);
	uv_err_t printTailingBytes();
	uv_err_t print();
	uv_err_t shouldPrint();
	uv_err_t addToPrint(UVDBFDPatFunction *toPrint);

	UVDStringWriter *getStringWriter();

public:
	//Sorted by (NOT strictly) increasing address
	std::vector<UVDBFDPatFunction *> m_toPrint;
	UVDBFDPatModule *m_module;
	UVDBFDPatModule::const_iterator m_iter;
};

#endif

