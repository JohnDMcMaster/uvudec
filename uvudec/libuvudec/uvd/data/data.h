/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UV_DISASM_DATA_H
#define UV_DISASM_DATA_H

#include <string>
#include "uvd/util/types.h"

#define UVD_DATA_ENDIAN_UNKNOWN				0
/*
MSB at lowest address
*/
#define UVD_DATA_ENDIAN_BIG					1
/*
LSB at lowest address
*/
#define UVD_DATA_ENDIAN_LITTLE				2
/*
PDP mixed endian
*/
#define UVD_DATA_ENDIAN_PDP					3
//Default to x86 since eventually this will be the lazy common use case
//I always remember x86 bi tordering because I RE'd an API and got the data size wronng, but LSB first meant it worked anyway
//This might be a really bad idea if people use inline funcs, but they should be explicit
#ifdef UVD_DATA_ENDIAN_USER
#define UVD_DATA_ENDIAN_DEFAULT				UVD_DATA_ENDIAN_USER
#else
#define UVD_DATA_ENDIAN_DEFAULT				UVD_DATA_ENDIAN_LITTLE
#endif

/*
Abstracts how we get the data for the disassembly
Similar in concept to BFD
*/
class UVDData
{
public:
	virtual ~UVDData();

	virtual void deinit();
	
	//Returns human readable string representation of the source
	virtual std::string getSource() const;
	
	//FIXME: de-overload all of these
	//Some of these may be deprecated
	//Read all of the data
	virtual uv_err_t readData(char **buffer) const;	
	virtual uv_err_t readData(uint32_t offset, char **buffer) const;
	virtual uv_err_t readData(uint32_t offset, char **buffer, uint32_t bufferSize) const;	
	virtual uv_err_t readData(uint32_t offset, std::string &s, uint32_t readSize) const;	
	//Core readData() implementation: child classes should implement this
	//By default, this calls read()
	virtual uv_err_t readData(uint32_t offset, char *buffer, uint32_t bufferSize) const;	
	virtual uv_err_t readData(uint32_t offset, char *c) const;	
	virtual uv_err_t readData(uint32_t offset, uint8_t *c) const;	
	
	//Omit endianness on single byte values
	uv_err_t readU8(uint32_t offset, uint8_t *out) const;
	uv_err_t read8(uint32_t offset, int8_t *out) const;
	uv_err_t readU16(uint32_t offset, uint16_t *out, uint32_t endianness = UVD_DATA_ENDIAN_DEFAULT) const;
	uv_err_t read16(uint32_t offset, int16_t *out, uint32_t endianness = UVD_DATA_ENDIAN_DEFAULT) const;
	uv_err_t readU32(uint32_t offset, uint32_t *out, uint32_t endianness = UVD_DATA_ENDIAN_DEFAULT) const;
	uv_err_t read32(uint32_t offset, int32_t *out, uint32_t endianness = UVD_DATA_ENDIAN_DEFAULT) const;
	
	//Omit endianness on single byte values
	uv_err_t writeU8(uint32_t offset, uint8_t in);
	uv_err_t write8(uint32_t offset, int8_t in);
	uv_err_t writeU16(uint32_t offset, uint16_t in, uint32_t endianness = UVD_DATA_ENDIAN_DEFAULT);
	uv_err_t write16(uint32_t offset, int16_t in, uint32_t endianness = UVD_DATA_ENDIAN_DEFAULT);
	uv_err_t writeU32(uint32_t offset, uint32_t in, uint32_t endianness = UVD_DATA_ENDIAN_DEFAULT);
	uv_err_t write32(uint32_t offset, int32_t in, uint32_t endianness = UVD_DATA_ENDIAN_DEFAULT);

	//WARNING: next 2 read implementations will rely on each other, you must implement at least one
	//Somewhat dangerous for new classes...maybe should do something different
	virtual int read(uint32_t offset, char *buffer, uint32_t bufferSize) const;	
	virtual int read(uint32_t offset) const;
	virtual int read(uint32_t offset, std::string &s, uint32_t readSize) const;	

	//Try to move away from returning int
	virtual uv_err_t writeData(uint32_t offset, const char *buffer, uint32_t bufferSize);
	virtual uv_err_t writeData(uint32_t offset, const UVDData *data);

	//Saves to a file
	virtual uv_err_t saveToFile(const std::string &file) const;

	//Read as if an array, equivilent to read with bad error checking
	//Made to make legacy functions using buffer conform easier to new code, will be deprecated
	unsigned char operator[](uint32_t offset);
		
	//How big the object is
	//That is, the first value of read(offset) that would fail
	virtual uint32_t size() const = 0;
	virtual uv_err_t size(uint32_t *sizeOut) const;
	
	/*
	Given a list, concatenate in order and produce output data
	*/
	static uv_err_t concatenate(const std::vector<UVDData *> &dataVector, UVDData **dataOut);
	//static uv_err_t concatenate(const std::vector<UVDData *> dataVector, UVDDataMemory *dataOut);

	/*
	TODO: implement this memory management scheme
	UVDData objects are passed all about and are difficult to track
	*/
	static void incrementReferences(UVDData *data);
	static void decreaseReferences(UVDData *data);

	//Copy as appropriete to maintain own object that can be deleted
	virtual uv_err_t deepCopy(UVDData **out);
	
	//For debugging
	void hexdump();

protected:
	//Do not instantiate this class by itself ... it has pure virtual funcs anyway
	UVDData();

private:
	//NOTE: this is not currently implemented, but may be in the future
	//When created, this should be 1
	//When this reaches 0, the object should be destroyed
	//int m_references;
};

/*
A UVDData that holds no data
Used for special purposes, sort of like a NULL element
*/
class UVDDataPlaceholder : public UVDData
{
public:
	UVDDataPlaceholder();
	~UVDDataPlaceholder();

	virtual std::string getSource() const;

	//These always return 0
	virtual int read(uint32_t offset, char *buffer, uint32_t bufferSize) const;	
	virtual int read(uint32_t offset) const;
	virtual int read(uint32_t offset, std::string &s, uint32_t readSize) const;	
	
	//Also 0
	virtual uint32_t size() const;
	virtual uv_err_t size(uint32_t *sizeOut) const;

	//Always an error for nonzero inputs
	uv_err_t writeData(uint32_t offset, const char *buffer, uint32_t bufferSize);
	
	virtual uv_err_t deepCopy(UVDData **out);
};

/*
A file based data source
This is roughly equivilent to binutils BFD
*/
class UVDDataFile : public UVDData
{
public:
	UVDDataFile();
	uv_err_t init(const std::string &file);
	static uv_err_t getUVDDataFile(UVDData** pDataFile, const std::string &file);
	static uv_err_t getUVDDataFile(UVDDataFile** pDataFile, const std::string &file);
	virtual ~UVDDataFile();	
	void deinit();

	//Returns human readable string representation of the source
	std::string getSource() const;

	int read(uint32_t offset, char *buffer, uint32_t bufferSize) const;
	uint32_t size() const;

public:	
	std::string m_sFile;
	FILE *m_pFile;
};

class UVDCompressedDataFile : public UVDDataFile
{
public:
};

//Takes in a memory buffer to use for the tracking
class UVDDataMemory : public UVDData
{
public:
	UVDDataMemory();
	//Create a buffer, reserving given size
	UVDDataMemory(uint32_t bufferSize);
	//Copy given buffer into this object
	UVDDataMemory(const char *buffer, uint32_t bufferSize);
	//static uv_err_t getUVDDataMemory(UVDDataMemory **data);
	//Transfer buffer to this object
	//The buffer will in effect be shared by this object
	static uv_err_t getUVDDataMemoryByTransfer(UVDDataMemory **data,
			char *buffer, uint32_t bufferSize,
			//Should free() be called on the buffer at object destruction?
			int freeAtDestruction = true);
	//Do a copy
	static uv_err_t getUVDDataMemoryByCopy(const UVDData *dataIn, UVDData **dataOut);
	virtual ~UVDDataMemory();

	std::string getSource() const;
	//Reallocate storage.  Buffer data and pointer is invalidated
	uv_err_t realloc(uint32_t bufferSize);
	
	int read(uint32_t offset, char *buffer, uint32_t bufferSize) const;
	uv_err_t writeData(uint32_t offset, const char *buffer, uint32_t bufferSize);
	uint32_t size() const;
	
	uv_err_t deepCopy(UVDData **out);

public:
	char *m_buffer;
	uint32_t m_bufferSize;
	//Should m_buffer be freed at destruction?
	int m_freeAtDestruction;
};

class UVDBufferedDataMemory : public UVDDataMemory
{
public:
	UVDBufferedDataMemory(uint32_t bufferSize = 0);
	UVDBufferedDataMemory(const char *buffer, uint32_t bufferSize);
	~UVDBufferedDataMemory();
	
	uv_err_t operator+=(const UVDData *other);
	uv_err_t operator+=(const std::string &other);
	uv_err_t append(const char *buffer, uint32_t bufferLength);
	uv_err_t appendByte(uint8_t in);
	
	uint32_t size() const;
	//Down-allocate space to minimum required
	uv_err_t compact();

public:
	//Size as returned by size()
	uint32_t m_virtualSize;
	//How much to grow beyond new min size by when we need more room
	uint32_t m_growScalar;
	uint32_t m_growConstant;
};

/*
Purpose of this class was to given another peice of data, be able to treat it as a discrete peice over a certain range

A discrete peice of data
It is undefined if the read will occur during init or during a request to get actual data
Idea was to be able to setup for file I/O, but delay the action unless needed
T mkjhis is attached to a specific UVDData that it is read from

This class is misguided, it should have descended from UVDData
...and now it does.  Stay to watch the fireworks...hopefully none
FIXME: convert these to the UVDData APIs
*/
class UVDDataChunk : public UVDData
{
public:
	UVDDataChunk();
	//Init with all data
	uv_err_t init(UVDData *data);
	//Init with select data
	uv_err_t init(UVDData *data, uint32_t minAddr, uint32_t maxAddr);
	~UVDDataChunk();
	
	bool operator==(UVDDataChunk &other);

	//min/max representation
	uint32_t getMin();
	uint32_t getMax();

	//offset/size representation
	uint32_t getOffset();
	uint32_t size() const;

	virtual int read(uint32_t offset, char *buffer, uint32_t bufferSize) const;	

	uv_err_t deepCopy(UVDData **out);

private:
	//Internal function to get a reference to the data buffer
	uv_err_t getData(const char * &buffer) const;
#ifdef UGLY_READ_HACK
	//If data has not been read yet, read it
	uv_err_t ensureRead();
#endif //UGLY_READ_HACK

public:
	//We do NOT own this data as the whole point of this class was to map onto external data
	//At best it would just be a placeholder
	UVDData *m_data;
	uint32_t m_offset;
	uint32_t m_bufferSize;
#ifdef UGLY_READ_HACK
	char *m_buffer;
#endif //UGLY_READ_HACK
};

#endif /* UV_DISASM_DATA_H */
