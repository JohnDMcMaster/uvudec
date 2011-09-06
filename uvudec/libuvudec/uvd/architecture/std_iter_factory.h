/*
UVNet Universal Decompiler (uvudec)
Copyright 2011 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_ARCHITECTURE_STD_INST_ITER_FACTORY_H
#define UVD_ARCHITECTURE_STD_INST_ITER_FACTORY_H

#include "uvd/architecture/architecture.h"

class UVDStdInstructionIteratorFactory : public UVDInstructionIteratorFactory {
public:
	UVDStdInstructionIteratorFactory();
	~UVDStdInstructionIteratorFactory();
	
	virtual uv_err_t abstractInstructionIteratorBeginByAddress( UVDAbstractInstructionIterator **out, UVDAddress address );
	virtual uv_err_t abstractInstructionIteratorEndByAddressSpace(UVDAbstractInstructionIterator **out, UVDAddressSpace *addressSpace);
};

class UVDStdPrintIteratorFactory : public UVDPrintIteratorFactory {
public:
	UVDStdPrintIteratorFactory();
	~UVDStdPrintIteratorFactory();
	
	virtual uv_err_t abstractPrintIteratorBeginByAddress( UVDAbstractPrintIterator **out, UVDAddress address );
	virtual uv_err_t abstractPrintIteratorEndByAddressSpace(UVDAbstractPrintIterator **out, UVDAddressSpace *addressSpace);
};


#endif

