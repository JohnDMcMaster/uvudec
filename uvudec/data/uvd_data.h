/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#ifndef UV_DISASM_DATA_H
#define UV_DISASM_DATA_H

#include <string>
#include "uvd_types.h"

/*
Abstracts how we get the data for the disassembly
Similar in concept to BFD
*/
class UVDData
{
public:
	UVDData();
	virtual ~UVDData();
	virtual void deinit();
	
	//Returns human readable string representation of the source
	virtual std::string getSource();
	
	//Read all of the data
	virtual uv_err_t readData(char **buffer) const;	
	virtual uv_err_t readData(unsigned int offset, char **buffer) const;	
	virtual uv_err_t readData(unsigned int offset, char **buffer, unsigned int bufferSize) const;	
	virtual uv_err_t readData(unsigned int offset, std::string &s, unsigned int readSize) const;	
	//Core readData() implementation: child classes should implement this
	//By default, this calls read()
	virtual uv_err_t readData(unsigned int offset, char *buffer, unsigned int bufferSize) const;	
	
	//WARNING: next 2 read implementations will rely on each other, you must implement at least one
	//Somewhat dangerous for new classes...maybe should do something different
	virtual int read(unsigned int offset, char *buffer, unsigned int bufferSize) const;	
	virtual int read(unsigned int offset) const;
	virtual int read(unsigned int offset, std::string &s, unsigned int readSize) const;	

	//Try to move away from returning int
	virtual uv_err_t writeData(unsigned int offset, const char *buffer, unsigned int bufferSize);

	//Saves to a file
	virtual uv_err_t saveToFile(const std::string &file) const;

	//Read as if an array, equivilent to read with bad error checking
	//Made to make legacy functions using buffer conform easier to new code, will be deprecated
	unsigned char operator[](unsigned int offset);
		
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

	virtual std::string getSource();

	//These always return 0
	virtual int read(unsigned int offset, char *buffer, unsigned int bufferSize) const;	
	virtual int read(unsigned int offset) const;
	virtual int read(unsigned int offset, std::string &s, unsigned int readSize) const;	
	
	//Also 0
	virtual uint32_t size() const;
	virtual uv_err_t size(uint32_t *sizeOut) const;

	//Always an error for nonzero inputs
	uv_err_t writeData(unsigned int offset, const char *buffer, unsigned int bufferSize);
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
	virtual ~UVDDataFile();	
	void deinit();

	//Returns human readable string representation of the source
	std::string getSource();	

	int read(unsigned int offset, char *buffer, unsigned int bufferSize) const;
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
	UVDDataMemory(unsigned int bufferSize);
	//Copy given buffer into this object
	UVDDataMemory(const char *buffer, unsigned int bufferSize);
	//static uv_err_t getUVDDataMemory(UVDDataMemory **data);
	//Transfer buffer to this object
	//The buffer will in effect be shared by this object
	static uv_err_t getUVDDataMemoryByTransfer(UVDDataMemory **data,
			char *buffer, unsigned int bufferSize,
			//Should free() be called on the buffer at object destruction?
			int freeAtDestruction = true);
	//Do a copy
	static uv_err_t getUVDDataMemoryByCopy(const UVDData *dataIn, UVDData **dataOut);
	virtual ~UVDDataMemory();

	std::string getSource();
	//Reallocate storage.  Buffer data and pointer is invalidated
	uv_err_t realloc(unsigned int bufferSize);
	
	int read(unsigned int offset, char *buffer, unsigned int bufferSize) const;
	uv_err_t writeData(unsigned int offset, const char *buffer, unsigned int bufferSize);
	uint32_t size() const;
	
public:
	char *m_buffer;
	unsigned int m_bufferSize;
	//Should m_buffer be freed at destruction?
	int m_freeAtDestruction;
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
	uv_err_t init(UVDData *data, unsigned int minAddr, unsigned int maxAddr);
	~UVDDataChunk();
	
	bool operator==(UVDDataChunk &other);

	//min/max representation
	uint32_t getMin();
	uint32_t getMax();

	//offset/size representation
	uint32_t getOffset();
	uint32_t size() const;

	virtual int read(unsigned int offset, char *buffer, unsigned int bufferSize) const;	

private:
	//Internal function to get a reference to the data buffer
	uv_err_t getData(const char * &buffer) const;
#ifdef UGLY_READ_HACK
	//If data has not been read yet, read it
	uv_err_t ensureRead();
#endif //UGLY_READ_HACK

public:
	UVDData *m_data;
	unsigned int m_offset;
	unsigned int m_bufferSize;
#ifdef UGLY_READ_HACK
	char *m_buffer;
#endif //UGLY_READ_HACK
};

#ifdef __cplusplus
extern "C"
{
#endif /* ifdef __cplusplus */

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* UV_DISASM_DATA_H */
