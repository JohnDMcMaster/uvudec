/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_CORE_ARCHITECTURE_H
#define UVD_CORE_ARCHITECTURE_H

#include "uvd_types.h"
//#include "uvd_instruction.h"
#include "uvd_config_symbol.h"
#include "uvd_register.h"
#include "uvd_instruction.h"

/*
A complete description of a system under analysis
This may include "hints" such as an operating system, peripherals, etc
This is the base for plugins that define new CPU's and such

TODO: break this into a base class with only virtual functions and one with data members
*/
class UVDArchitecture
{
public:
	UVDArchitecture();
	virtual ~UVDArchitecture();

	virtual uv_err_t init();	
	virtual uv_err_t deinit();	
	
	//Allocate an instruction object compatbile with this architecture
	virtual uv_err_t getInstruction(UVDInstruction **out) = 0;

	//FIXME: this is a hack until the address spaces can be standardized into this arch object
	virtual uv_err_t getAddresssSpaceNames(std::vector<std::string> &names) = 0;

	virtual uv_err_t parseCurrentInstruction(UVDIteratorCommon &iterCommon) = 0;
};

#endif

