/*
Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under the terms of the BSD license.  See LICENSE for details.
*/


/*
Certain items must be placed and then have references placed among some set of items
This inter-dependent setup can only be done after ALL items are placed, making sequential setup difficult
Fix this issue by recording all locations that need to be patched up and then apply the patch after all items are placed


Header/footer/in between can be represented as a relocatable object with no relocation targets
Create relocatable data from some buffer
For each spot in that data that needs to be relocated, insert a relocatable object at each address that needs to be modified
Append all objects into one large data object
Now that all locations have been fixed, apply the relocations to the known locations
*/

#pragma once

#include <set>
#include "uvd_data.h"
#include "uvd_types.h"

/*
A spot within a peice of data requiring a fixup
Tracks the specific way to apply the patch
Ex:
-where g_debug is used
-a reference to another data section within the ELF file
We may need to do a relative jump patch, which is not just an absolute address
Base class does absolute addresses as this is the most common behavior
Size is in bytes
*/
class UVDRelocatableElement;
class UVDRelocationFixup
{
public:
	UVDRelocationFixup();
	UVDRelocationFixup(UVDRelocatableElement *symbol, unsigned int offset, unsigned int size);
	~UVDRelocationFixup();
	
	//Apply the patch to this data
	//m_symbol should hold a valid value by this point (or be calculable by call)
	uv_err_t applyPatch(UVDData *data);
	
public:
	//The value we were waiting on
	//This will eventually give us some value to work off of
	UVDRelocatableElement *m_symbol;
	//The position to apply the patch
	//Relative to the data element passed in
	unsigned int m_offset;
	//How many bytes to apply to, in bytes
	//Some data may use 4 byte addressing, other spots 1, for example
	unsigned int m_size;
};

/*
This can include things with and without symbolic values (symbolic values are repr by UVDRelocatableElement)
-ELF section headers
-functions that need variable addresses filled in
*/
class UVDRelocatableData
{
public:
	UVDRelocatableData();
	UVDRelocatableData(UVDData *data);
	~UVDRelocatableData();
	
	//Assume all symbolic values have been placed and now have symbols
	uv_err_t applyRelocations();
	void addFixup(UVDRelocationFixup *);
	
public:
	//The temporary peice of data
	//This will be compiled into a larger chunk
	UVDData *m_data;

	//Locations that require fixups
	//Addresses specified are relative to m_data
	std::set<UVDRelocationFixup *> m_fixups;
};

/*
A value that will later be updated to have a concrete value
eg: the address of g_debug
*/
class UVDRelocatableElement
{
public:
	UVDRelocatableElement();
	virtual ~UVDRelocatableElement();
	
	virtual void setDynamicValue(int dynamicValue);
	virtual uv_err_t getDynamicValue(char const **dynamicValue);
	/*
	virtual int getDynamicValue(void);
	virtual uv_err_t getDynamicValue(int *dynamicValue);
	*/
	
public:
	//Needed once we fix up all the values
	int m_dynamicValue;
	//Have we at least made a token effort to put a valid value in?
	int m_isDynamicValueValid;
};

/*
Capable of finding its own final address
Its value is some relocatable data chunk that was found in the final output list
*/
class UVDRelocationManager;
class UVDSelfLocatingRelocatableElement : public UVDRelocatableElement
{
public:
	UVDSelfLocatingRelocatableElement();
	//Look for the encapsulated data
	UVDSelfLocatingRelocatableElement(UVDRelocationManager *manager, UVDRelocatableData *relocatableData, unsigned int offset = 0);
	//Raw data structure
	UVDSelfLocatingRelocatableElement(UVDRelocationManager *manager, UVDData *data, unsigned int offset = 0);
	virtual uv_err_t getDynamicValue(char const **dynamicValue);

public:
	//List list we will be searching
	//We must search this for data coming before our element
	UVDRelocationManager *m_manager;
	//The item we must find
	//We will find first instance
	UVDRelocatableData *m_relocatableData;
	//If above is not specified, fall back on this
	UVDData *m_data;
	//Offset from the data start position
	unsigned int m_offset;
};

/*
Contains all the data to be fixed up as well as the symbols
*/
class UVDRelocationManager
{
public:
	UVDRelocationManager();
	~UVDRelocationManager();
	
	//A symbol or similar concept that will be resolved later
	uv_err_t addRelocatableElement(UVDRelocatableElement *element);
	//A peice of data requiring the symbols above to be placed
	//The symbols above will likely be contained somehow in these peices 
	uv_err_t addRelocatableData(UVDRelocatableData *data);
	
	//Do linker like actions to relocate the data
	//Generates a resulting final peice of data from concatentating all the peices together
	//This version allocates the data
	uv_err_t applyPatch(UVDData **data);
	
	//uv_err_t getDataSize(int *dataSize);

public:
	//All of the "symbols" we must keep track of
	//It would be ideal, but might not be required, for all symbols to be registered here before added as a section
	std::set<UVDRelocatableElement *> m_relocatableElements;
	//Subsets we must operate on
	std::vector<UVDRelocatableData *> m_data;
};

