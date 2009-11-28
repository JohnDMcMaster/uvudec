/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
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

//Byte value used for relocations on hashed functions
#define RELOCATION_DEFAULT_VALUE		0

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
	//A symbol value and an offset to apply the patch (along with data size, if needed)
	UVDRelocationFixup(UVDRelocatableElement *symbol, unsigned int offset, unsigned int size);
	~UVDRelocationFixup();
	
	//Apply the patch to this data
	//m_symbol should hold a valid value by this point (or be calculable by call)
	uv_err_t applyPatch(UVDData *data);
	uv_err_t applyPatchCore(UVDData *data, bool useDefaultValue);
	
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
Applies to a single location rather than batch data processing schemes
*/
class UVDSimpleRelocationFixup
{
public:
	UVDSimpleRelocationFixup();
	//Creates a simplified version of a previously created fixup 
	//This may be removed, it seems below was real intention
	//uv_err_t getUVDSimpleRelocationFixup(UVDSimpleRelocationFixup **simpleFixupOut, UVDRelocationFixup *fixup,
	//		char *data, int offset, int size);
	//Creates a fixup from scratch
	static uv_err_t getUVDSimpleRelocationFixup(
			UVDSimpleRelocationFixup **simpleFixupOut, UVDRelocatableElement *relocatableElement,
			char *data, int size);
	~UVDSimpleRelocationFixup();
	
	//The simply "everything was already setup before" function
	uv_err_t applyPatch();

public:
	//Calculates where to apply value
	//The calculation will occur internally as part of a UVDRelocatableElement
	UVDRelocationFixup *m_relocationFixup;
	//The target
	UVDData *m_data;
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
	/*
	For table structures, sometimes a placeholder is needed at the beginning
	of the table to reference instead of the first element, which can be more
	complicated to aquire
	Its m_data is gauranteed to have a unique valid address and have a zero
	size
	*/
	static uv_err_t getUVDRelocatableDataPlaceholder(UVDRelocatableData **data);
	~UVDRelocatableData();
	
	//Assume all symbolic values have been placed and now have symbols
	uv_err_t applyRelocations();
	uv_err_t applyRelocationsCore(bool useDefaultValue);
	void addFixup(UVDRelocationFixup *);
	
	//Get a raw copy of the data
	//Relocatable elements will have the last appliex fixup value, if any
	//It will be freed at the destruction of this object
	uv_err_t getRelocatableData(UVDData **data);

	//Get a default representation of the relocatable data
	//Fills in relocatable entries with 0's and returns
	//It will be freed at the destruction of this object
	uv_err_t getDefaultRelocatableData(UVDData **data);
	
public:
	//The temporary peice of data
	//This will be compiled into a larger chunk
	UVDData *m_data;
	//Cache of the default (0'd) relocatable data
	UVDData *m_defaultRelocatableData;

	//Locations that require fixups
	//Addresses specified are relative to m_data
	std::set<UVDRelocationFixup *> m_fixups;
};

/*
A value that will later be updated to have a concrete value
eg:
	the address of g_debug
	number of items in something
*/
class UVDRelocatableElement
{
public:
	UVDRelocatableElement();
	virtual ~UVDRelocatableElement();
	
	//dynamicValue is in the local system encoding
	virtual void setDynamicValue(int32_t dynamicValue);
	//If it must be calculated on the fly as needed
	virtual uv_err_t updateDynamicValue();

	//Making this work nicely is still under dev
	//Encoding will be returned as m_encoding, not as local system encoding
	//Signed vs unsigned may not be necessary then?
	uv_err_t getDynamicValue(int8_t *dynamicValue);
	uv_err_t getDynamicValue(uint8_t *dynamicValue);
	uv_err_t getDynamicValue(int16_t *dynamicValue);
	uv_err_t getDynamicValue(uint16_t *dynamicValue);
	uv_err_t getDynamicValue(int32_t *dynamicValue);
	//Currently the core functionality of all other primatives
	virtual uv_err_t getDynamicValue(uint32_t *dynamicValue);
	//Size in bytes
	//virtual uv_err_t getDynamicValue(char const **dynamicValue, int dynamicValueSize);
	virtual uv_err_t getDynamicValue(char const **dynamicValue);

	/*
	virtual int getDynamicValue(void);
	virtual uv_err_t getDynamicValue(int *dynamicValue);
	*/
	
	//If this symbol has a name, get it
	std::string getName();
	void setName(const std::string &sName);
	
protected:
	//To make solving endianess issues later easier
	//static virtual uv_err_t applyDynamicValue(char const **dynamicValue, int *value);
	
public:
	//The output encoding if the result is a number
	//Meant to solve big/little endian issues
	int m_encoding;
	//Needed once we fix up all the values
	//Most all values are 32 bit or less, use special class later if needed
	uint32_t m_dynamicValue;
	//Have we at least made a token effort to put a valid value in?
	int m_isDynamicValueValid;
	//Name, if applicable
	std::string m_sName;
};

class UVDRelocationManager;

/*
Capable of finding its own final address
Its value is some relocatable data chunk that was found in the final output list
*/
class UVDSelfLocatingRelocatableElement : public UVDRelocatableElement
{
public:
	UVDSelfLocatingRelocatableElement();
	//Look for the encapsulated data
	UVDSelfLocatingRelocatableElement(UVDRelocationManager *manager, UVDRelocatableData *relocatableData, uint32_t padding = 0);
	//Raw data structure
	UVDSelfLocatingRelocatableElement(UVDRelocationManager *manager, UVDData *data, uint32_t padding = 0);
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
	//Additional offset from the data start position
	uint32_t m_offset;
};

/*
Capable of finding a UVDData/UVDRelocatableData's size
Designed to be used as an offset + size combo of above
*/
class UVDSelfSizingRelocatableElement : public UVDRelocatableElement
{
public:
	UVDSelfSizingRelocatableElement();
	//Get the size from given data
	UVDSelfSizingRelocatableElement(UVDRelocatableData *relocatableData, uint32_t padding = 0);
	UVDSelfSizingRelocatableElement(UVDData *data, uint32_t padding = 0);
	virtual uv_err_t getDynamicValue(char const **dynamicValue);

public:
	//The item we must use
	UVDRelocatableData *m_relocatableData;
	//If above is not specified, fall back on this
	UVDData *m_data;
	//Additional padding from calculated size
	uint32_t m_padding;
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
	//This has shown to not be used in practice
	//uv_err_t addRelocatableElement(UVDRelocatableElement *element);
	
	//A peice of data requiring the symbols above to be placed
	//The symbols above will likely be contained somehow in these peices 
	uv_err_t addRelocatableData(UVDRelocatableData *data);
	
	//Do linker like actions to relocate the data
	//Generates a resulting final peice of data from concatentating all the peices together
	//This version allocates the data
	uv_err_t applyPatch(UVDData **data);
	uv_err_t applyPatchCore(UVDData **dataOut, bool useDefaultValue);
	
	//uv_err_t getDataSize(int *dataSize);

public:
	//All of the "symbols" we must keep track of
	//It would be ideal, but might not be required, for all symbols to be registered here before added as a section
	//These will be applied in order
	//std::set<UVDRelocatableElement *> m_relocatableElements;
	//Subsets we must operate on
	//Usually these will also be used for calculating the relocation values
	std::vector<UVDRelocatableData *> m_data;
};

