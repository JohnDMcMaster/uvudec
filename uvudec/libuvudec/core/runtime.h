/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_RUNTIME_H
#define UVD_RUNTIME_H

#include "uvd_address.h"
#include "object/object.h"
#include "architecture/architecture.h"

/*
Represents what we'd need to know to actually run a peice of code
Also, any derrived data after combining that data such as the actual memory accessible
*/
class UVDArchitecture;
class UVDObject;
class UVDRuntime
{
public:
	UVDRuntime();
	~UVDRuntime();
	
	uv_err_t init(UVDObject *object, UVDArchitecture *architecture);

	//Use all availible architecture, object, OS, etc hints to figure out what is runnable
	virtual uv_err_t rebuildAddressSpaces();
	virtual uv_err_t clearAddressSpaces();
	
	//XXX: hack, will be removed in future
	//Convenience function, mostly to aid transition to using UVDObject() and UVDRuntime()
	//out is still owned by us, do not delete it
	//Will return UV_ERR_NOTFOUND and set *out to NULL if none exist
	virtual uv_err_t getPrimaryExecutableAddressSpace(UVDAddressSpace **out);

public:
	//Source of data to disassemble
	//We do not own this
	//...but we probably should
	//UVDData *m_data;
	//...and a big change
	UVDObject *m_object;

	/*
	XXX
	Need to group the following into a logical unit that defins a program environment
	Is virtual machine an appropriete term?
	Program?
	Runtime probably
	*/
	//ISA and such
	UVDArchitecture *m_architecture;
	
	//To consider for the future
	//A way to debug this
	//UVDDebugger *m_debugger;
	//Operating system
	//UVDOS *m_OS;
	
	/*
	The actual active memory spaces after we combine what it really means to run this program with the best hints given
	This may be a direct copy from either the architecture or the object file
	I'd like us to own these all and just map onto spaces
	Presumably not that hard to map an address space
	Eventually we should try to merge section together, but for now we might have duplicate spaces...so beware
	*/
	UVDAddressSpaces m_addressSpaces;
};

#endif

