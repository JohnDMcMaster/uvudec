/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_CORE_ARCHITECTURE_H
#define UVD_CORE_ARCHITECTURE_H

#include "uvd/util/types.h"
#include "uvd/assembly/register.h"
#include "uvd/assembly/instruction.h"
//#include "uvd/assembly/cpu.h"

class UVDPrintIterator;
class UVDInstructionIterator;
class UVDAbstractInstructionIterator;
class UVDAbstractPrintIterator;
class UVDASInstructionIterator;
class UVDInstructionIteratorFactory {
public:
	UVDInstructionIteratorFactory();
	~UVDInstructionIteratorFactory();
	
	/*
	Instruction iterator factory methods
	*/
	//Begin at an architecture specific logical beginning
	uv_err_t instructionIteratorBegin( UVDInstructionIterator *out );
	//Default will use the first address space it can find
	virtual uv_err_t abstractInstructionIteratorBegin( UVDAbstractInstructionIterator **out );
	//Begin at a specific address
	uv_err_t instructionIteratorBeginByAddress( UVDInstructionIterator *out, UVDAddress address );
	virtual uv_err_t abstractInstructionIteratorBeginByAddress( UVDAbstractInstructionIterator **out, UVDAddress address );
	//Architecture specific end
	//Iterating past this should provide no more results
	uv_err_t instructionIteratorEnd( UVDInstructionIterator *out );
	//Default will use the first address space it can find
	virtual uv_err_t abstractInstructionIteratorEnd(UVDAbstractInstructionIterator **out);
	//End of an architecture specific address space
	uv_err_t instructionIteratorEndByAddressSpace( UVDInstructionIterator *out, UVDAddressSpace *addressSpace );	
	virtual uv_err_t abstractInstructionIteratorEndByAddressSpace(UVDAbstractInstructionIterator **out, UVDAddressSpace *addressSpace);
};

class UVDPrintIteratorFactory {
public:
	UVDPrintIteratorFactory();
	~UVDPrintIteratorFactory();
	
	/*
	Print iterator factory methods
	*/
	uv_err_t printIteratorBegin( UVDPrintIterator *out );
	virtual uv_err_t abstractPrintIteratorBegin( UVDAbstractPrintIterator **out );
	//Begin at a specific address
	uv_err_t printIteratorBeginByAddress( UVDPrintIterator *out, UVDAddress address );
	virtual uv_err_t abstractPrintIteratorBeginByAddress( UVDAbstractPrintIterator **out, UVDAddress address );
	//Architecture specific end
	//Iterating past this should provide no more results
	uv_err_t printIteratorEnd( UVDPrintIterator *out );
	virtual uv_err_t abstractPrintIteratorEnd(UVDAbstractPrintIterator **out);
	//End of an architecture specific address space
	uv_err_t printIteratorEndByAddressSpace( UVDPrintIterator *out, UVDAddressSpace *addressSpace );	
	virtual uv_err_t abstractPrintIteratorEndByAddressSpace(UVDAbstractPrintIterator **out, UVDAddressSpace *addressSpace);
};


/*
A complete description of a system under analysis
This may include "hints" such as an operating system, peripherals, etc
This is the base for plugins that define new CPU's and such

TODO: break this into a base class with only virtual functions and one with data members
*/
class UVD;
class UVDCPUVector;
class UVDPrintIterator;
class UVDArchitecture
{
public:
	UVDArchitecture();
	virtual ~UVDArchitecture();

	virtual uv_err_t init();
	virtual uv_err_t deinit();
	
	/*
	If the architecture and the object file are ALWAYS coupled together
	(eg: UVDBFDArchitecture only works on a UVDBFDObject), then override this
	Otherwise, better to leave as default since default implementation is
	for generic address lookup
	
	Main purpose was that if a library ONLY gave the following routines you could still use it:
	-begin()
	-next()
	-end()
	IE no random access
	*/
	
	//Defaults to StdInstructionIterator
	virtual uv_err_t getInstructionIteratorFactory(UVDInstructionIteratorFactory **out);
	//Defaults to StdPrintIterator
	virtual uv_err_t getPrintIteratorFactory(UVDPrintIteratorFactory **out);
	
	//Allocate an instruction object compatbile with this architecture
	//virtual uv_err_t getInstruction(UVDInstruction **out, UVDAddress address) = 0;

	//FIXME: this is a hack until the address spaces can be standardized into this arch object
	//...and added below, so we should make this instead (at least by default) simply dump the below
	virtual uv_err_t getAddresssSpaceNames(std::vector<std::string> &names) = 0;

	//Optional
	//Define to use the standard iterator model
	//You will provide the instruction, if such can be generated, at current address
	virtual uv_err_t parseCurrentInstruction(UVDASInstructionIterator &iter);

	/*
	In the future this might even return something if we are debugging
	This should rely on m_uvd->m_object to read sections as appropriete
	*/
	virtual uv_err_t readByte(UVDAddress address, uint8_t *out);

	/*
	Check things that look like they weren't specified and fill in a reasonable default
	Issue was CPU vectors should only be logically added to and super init code should execute after us
	*/
	virtual uv_err_t fixupDefaults();

	uv_err_t doInit();
	
	//vector is still owned by this architecture object
	//return UV_ERR_NOTFOUND if doesn't exist and out will be set to NULL
	uv_err_t getVector(const UVDAddress *address, UVDCPUVector **out);

public:
	UVD *m_uvd;
	//Address spaces inherently defined by this architecture
	//Actual code may or may not be able to access them based on MMU/OS configuration
	UVDAddressSpaces m_addressSpaces;

	//Start addresses
	//We might try to force 0 if you don't specify something
	std::vector<UVDCPUVector *> m_vectors;

	UVDInstructionIteratorFactory *m_instructionIteratorFactory;
	UVDPrintIteratorFactory *m_printIteratorFactory;
};

#endif

