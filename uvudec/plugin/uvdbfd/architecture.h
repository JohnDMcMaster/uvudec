/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVDBFD_ARCHITECTURE_H
#define UVDBFD_ARCHITECTURE_H

#include "uvd/architecture/architecture.h"
#include "uvdasm/util.h"
#include "uvdasm/opcode_table.h"
#include "bfd.h"

/*
Configuration file based architecture
*/
class UVDBFDArchitecture : public UVDArchitecture
{
public:
	UVDBFDArchitecture();
	virtual ~UVDBFDArchitecture();

	virtual uv_err_t init();	
	virtual uv_err_t deinit();	

	virtual uv_err_t getInstruction(UVDInstruction **out);
	virtual uv_err_t getAddresssSpaceNames(std::vector<std::string> &names);

	//virtual uv_err_t parseCurrentInstruction(UVDInstructionIterator &iterCommon);

	static uv_err_t canLoad(const UVDObject *object, const UVDRuntimeHints &hints, uvd_priority_t *confidence, void *user);
	static uv_err_t tryLoad(UVDObject *object, const UVDRuntimeHints &hints, UVDArchitecture **out, void *user);

	virtual uv_err_t getInstructionIteratorFactory(UVDInstructionIteratorFactory **out);

	/*
	Should be in object, not architecture
	uv_err_t sectionToAddressSpace( const asection *section, UVDAddressSpace **out );
	uv_err_t addressSpaceToSection( const UVDAddressSpace *addressSpace, asection **out );
	*/
	
public:	
};

#endif

