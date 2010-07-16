/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
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
class UVDBinarySymbol;
class UVDRelocationFixup
{
public:
	UVDRelocationFixup();
	//A symbol value and an offset to apply the patch (along with data size, if needed)
	UVDRelocationFixup(UVDRelocatableElement *symbol, uint32_t offset, uint32_t sizeBytes);
	virtual ~UVDRelocationFixup();
	
	//Apply the patch to this data
	//m_symbol should hold a valid value by this point (or be calculable by call)
	uv_err_t applyPatch(UVDData *data);
	uv_err_t applyPatchCore(UVDData *data, bool useDefaultValue);
	
	//For current analysis where we do not know which function (or abstract block) we are apart of yet
	static uv_err_t getUnknownSymbolRelocationFixup(UVDRelocationFixup **fixupOut, uint32_t offsetBytes, uint32_t sizeBytes);
	static uv_err_t getUnknownSymbolRelocationFixupByBits(UVDRelocationFixup **fixupOut, uint32_t offsetBytes, uint32_t sizeBits);
	/*
	A relocation fixup for the symbol given
	Gives a fixup such that it will get the address value of the symbol and patch the offset + size as needed
	Default system endianess is assumed
	*/
	static uv_err_t getSymbolRelocationFixup(UVDRelocationFixup **fixupOut, UVDBinarySymbol *symbol, uint32_t offsetBytes, uint32_t sizeBytes);
	static uv_err_t getSymbolRelocationFixupByBits(UVDRelocationFixup **fixupOut, UVDBinarySymbol *binarySymbol, uint32_t offsetBytes, uint32_t sizeBits);
		
	//offset in bytes
	virtual uv_err_t setOffset(uint32_t offset);
	virtual uv_err_t getOffset(uint32_t *offset);
	
	uint32_t getSizeBits();
	uint32_t getSizeBytes();
	virtual uv_err_t getSizeBytes(uint32_t *bytes);
	virtual uv_err_t setSizeBits(uint32_t bits);
	virtual uv_err_t setSizeBytes(uint32_t bytes);
	
	
public:
	//The value we were waiting on
	//This will eventually give us some value to work off of
	UVDRelocatableElement *m_symbol;
	//The position to apply the patch
	//Relative to the data element passed in
	//In bytes
	uint32_t m_offset;

private:
	//How many bytes to apply to, in bytes
	//Some data may use 4 byte addressing, other spots 1, for example
	uint32_t m_sizeBits;
};

#if 0
/*
Uses bitmask to do partial fixup
*/
class UVDMaskedRelocationFixup
{
};
#endif

/*
Applies to a single location rather than batch data processing schemes
*/
class UVDSimpleRelocationFixup
{
public:
	UVDSimpleRelocationFixup();
	~UVDSimpleRelocationFixup();
	uv_err_t deinit();
	
	//Creates a simplified version of a previously created fixup 
	//This may be removed, it seems below was real intention
	//uv_err_t getUVDSimpleRelocationFixup(UVDSimpleRelocationFixup **simpleFixupOut, UVDRelocationFixup *fixup,
	//		char *data, int offset, int size);
	//Creates a fixup from scratch
	static uv_err_t getUVDSimpleRelocationFixup(
			UVDSimpleRelocationFixup **simpleFixupOut, UVDRelocatableElement *relocatableElement,
			char *data, int size);
	
	//The simply "everything was already setup before" function
	uv_err_t applyPatch();

public:
	//Calculates where to apply value
	//The calculation will occur internally as part of a UVDRelocatableElement
	//We own this
	UVDRelocationFixup *m_relocationFixup;
	//The target
	//We do not own this
	UVDData *m_data;
};

/*
Some data that has items that need relocations applied to it
This can include things with and without symbolic values (symbolic values are repr by UVDRelocatableElement)
-ELF section headers
-functions that need variable addresses filled in
*/
class UVDRelocatableData
{
public:
	UVDRelocatableData();
	//Doing deep copy now makes this memory unsafish
	//UVDRelocatableData(UVDData *data);
	virtual ~UVDRelocatableData();
	virtual uv_err_t deinit();

	/*
	For table structures, sometimes a placeholder is needed at the beginning
	of the table to reference instead of the first element, which can be more
	complicated to aquire
	Its m_data is gauranteed to have a unique valid address and have a zero
	size
	*/
	static uv_err_t getUVDRelocatableDataPlaceholder(UVDRelocatableData **data);
	
	//Assume all symbolic values have been placed and now have symbols
	uv_err_t applyRelocations();
	virtual uv_err_t applyRelocationsCore(bool useDefaultValue);
	virtual uv_err_t addFixup(UVDRelocationFixup *);
	
	//Get a raw copy of the data
	//Relocatable elements will have the last appliex fixup value, if any
	//It will be freed at the destruction of this object
	virtual uv_err_t getRelocatableData(UVDData **data);

	//Get a default representation of the relocatable data
	//Fills in relocatable entries with 0's and returns
	//It will be freed at the destruction of this object
	virtual uv_err_t getDefaultRelocatableData(UVDData **data);
	
	//The data here will be freed when this object is freed
	//Above comment is wrong
	//In actually, this does a depp copy
	virtual uv_err_t setData(UVDData *data);
	//Same as above, but not copied
	virtual uv_err_t transferData(UVDData *data, uint32_t freeAtDestruction);
	
	/*
	Empty is an unitialized object that has no data added to it
	If a zero sized data is added, it is not empty, it is filled with a 0 sized element
	*/
	virtual uv_err_t isEmpty(uint32_t *isEmpty);

	//For debugging
	void hexdump();
	void hexdumpDefault();
	
	//How big the encapsulated data is
	uv_err_t size(uint32_t *sizeOut);

	virtual bool requiresDataSync();
	
protected:
	//called before getData()
	virtual uv_err_t updateData();
	//called before getRelocatableData()
	virtual uv_err_t updateDefaultRelocatableData();
	
public:
	//Locations that require fixups
	//Addresses specified are relative to m_data
	//We own these
	std::set<UVDRelocationFixup *> m_fixups;

protected:
	//The temporary peice of data
	//This will be compiled into a larger chunk
	//We own this
	UVDData *m_data;
	//Whether or not m_data is governed externally
	//m_defaultRelocatableData is pecific to this object and unaffected by this
	uint32_t m_freeDataAtDestruction;

	//Cache of the default (0'd) relocatable data
	//Primary data is stored in m_data
	//We own this
	UVDData *m_defaultRelocatableData;
};

/*
Contains a number of UVDRelocatableData
*/
class UVDMultiRelocatableData : public UVDRelocatableData
{
public:
	UVDMultiRelocatableData();
	~UVDMultiRelocatableData();

	//All below implemented as error for now, otherwise would have to check for equal size and spread data across elements
	uv_err_t setData(UVDData *data);
	uv_err_t addFixup(UVDRelocationFixup *fixup);
	
	virtual uv_err_t isEmpty(uint32_t *isEmpty);
	
	//FIXME hack
	virtual bool requiresDataSync();

protected:
	//Iterate over all values and apply relocations
	uv_err_t applyRelocationsCore(bool useDefaultValue);

	virtual uv_err_t updateData();
	virtual uv_err_t updateDefaultRelocatableData();

public:
	//Order is preserved
	std::vector<UVDRelocatableData *> m_relocatableDatas;
};

/*
A value that will later be updated to have a concrete value
In fact, should probably rename to UVDRelocatableValue, was probably one of those 2AM naming decisions
eg:
	the address of g_debug
	number of items in something
*/
class UVDRelocatableElement
{
public:
	UVDRelocatableElement();
	//A const value of sorts, set it from the start
	UVDRelocatableElement(uint32_t dynamicValue);
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
	//virtual std::string getName();
	virtual uv_err_t getName(std::string &s);
	uv_err_t setName(const std::string &sName);
	
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
This and UVDData should probably be merged and done recursivly
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
	//FIXME: this is a poor name
	uv_err_t applyPatch(UVDData **data);
	uv_err_t applyPatchCore(UVDData **dataOut, bool useDefaultValue);
	
	//uv_err_t getDataSize(int *dataSize);
	//Find element and report the offset (that is, sum of size of all precending elements)
	//Returns error on missing element
	uv_err_t getOffset(const UVDRelocatableData *relocatableData, uint32_t *offsetOut);

public:
	//All of the "symbols" we must keep track of
	//It would be ideal, but might not be required, for all symbols to be registered here before added as a section
	//These will be applied in order
	//std::set<UVDRelocatableElement *> m_relocatableElements;
	//Subsets we must operate on
	//Usually these will also be used for calculating the relocation values
	std::vector<UVDRelocatableData *> m_data;
};

