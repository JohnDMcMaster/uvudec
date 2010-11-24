/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

/*
"In computer science, an object file is an organized collection of separate, named sequences of machine code"
http://en.wikipedia.org/wiki/Object_file
I wish I was more familar with libbfd to know what sort of curve balls I should expect in advance

Object files need to address the following basic concerns:
Immediatly
-Identify the area(s) of code that are executable
	-Relocations
		For FLIRT
-Architecture determines address space, but we may be given some hints here?
	need to figure out whats needed to have segmenting be sane under x86
	We can provide data WITHIN an address space though
-Can be loaded without a matching architecture object

Soon to come
-OS based objects
	Define an OS specific entry point
		Architecture defines the real entry point as the start vector

Also, it may be undesirable, but possible to separate an architecture and the binary format
Ex:
	libbfd cannot work as an architecture without an acceptable input file
	We'd either have to kludge together an ELF file or something or manually construct it so its happy
*/

#ifndef UVD_OBJECT_OBJECT_H
#define UVD_OBJECT_OBJECT_H

#include "uvd/assembly/address.h"
#include "uvd/util/types.h"
#include "uvd/core/runtime_hints.h"
#include "uvd/object/section.h"
#include "uvd/util/priority.h"
#include "uvd/assembly/symbol.h"
#include <vector>
#include <limits.h>

/*
A ELF, raw binary, COFF, etc type object
(there is no Object class everything in UVD descends from)
*/
class UVDObject
{
public:
	//Objects should implement the following functions for registration
	//They will be used with templates and/or macros to register classes

	//Function canLoad(...)
	//Used for probing
	//Intended to display an interactive menu of good architecture choices
	//confidence: priority level of being a good match
	typedef uv_err_t (*CanLoad)(const UVDData *data, const UVDRuntimeHints &hints, uvd_priority_t *confidence,
			void *user);

	//Function tryLoad(...)
	//Do the actual load
	//Note that we MIGHT NOT call CanLoad first
	//eg: user manually selects object format
	//Ownership of data is transfered to returned object upon success
	typedef uv_err_t (*TryLoad)(const UVDData *data, const UVDRuntimeHints &hints, UVDObject **out,
			void *user);

public:
	UVDObject();
	virtual ~UVDObject();
	
	//Load given data as this type of object
	virtual uv_err_t init(UVDData *data);
	//Convenience function to collect address spaces from each function	
	//virtual uv_err_t getAddressSpaces(UVDAddressSpaces *out);
	//All of the sections returned to the best of our ability
	//We own the returned pointers, possibly in the sections
	//virtual uv_err_t getSections(std::vector<UVDSection *> &out);

	//virtual uv_err_t getFunctions(

	virtual uv_err_t addRelocation(UVDRelocationFixup *relocation);

public:
	UVDBinarySymbolManager m_symbols;
	//Raw pointer to the data
	//We own this
	//Also, we are a loader...don't modify the data
	UVDData *m_data;
	//We own these sections
	std::vector<UVDSection *> m_sections;
};

#endif

