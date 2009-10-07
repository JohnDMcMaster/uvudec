/*
Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under the terms of the BSD license.  See LICENSE for details.
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
	void deinit();
	
	//Returns human readable string representation of the source
	virtual std::string getSource();
	
	//Read implementations will rely on each other, you must implement at least one
	//like read(2)
	virtual int read(unsigned int offset, char *buffer, unsigned int bufferSize);	
	//like getchar(1)
	virtual int read(unsigned int offset);
	virtual int read(unsigned int offset, std::string &s, unsigned int readSize);	

	//Saves to a file
	uv_err_t saveToFile(const std::string &file);

	//Read as if an array, equivilent to read with bad error checking
	//Made to make legacy functions using buffer conform easier to new code, will be deprecated
	unsigned char operator[](unsigned int offset);
		
	//How big the object is
	//That is, the first value of read(offset) that would fail
	virtual unsigned int size();
	
	/*
	Given a list, concatenate in order and produce output data
	*/
	static uv_err_t concatenate(const std::vector<UVDData *> dataVector, UVDData **dataOut);
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

	int read(unsigned int offset, char *buffer, unsigned int bufferSize);
	unsigned int size();

public:	
	std::string m_sFile;
	FILE *m_pFile;
};

class UVDCompressedDataFile : UVDDataFile
{
public:
};

//Takes in a memory buffer to use for the tracking
class UVDDataMemory : public UVDData
{
public:
	//Create a buffer, reserving given size
	UVDDataMemory(unsigned int bufferSize);
	//Copy given buffer into this object
	UVDDataMemory(const char *buffer, unsigned int bufferSize);
	virtual ~UVDDataMemory();
	std::string getSource();
	
	int read(unsigned int offset, char *buffer, unsigned int bufferSize);
	
	char *m_buffer;
	unsigned int m_bufferSize;
};

/*
A discrete peice of data
It is undefined if the read will occur during init or during a request to get actual data
Idea was to be able to setup for file I/O, but delay the action unless needed
This is attached to a specific UVDData that it is read from

This class is misguided, it should have descended from UVDData
...and now it does.  Stay to watch the fireworks...hopefully none
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
	
	uv_err_t getData(const char *&buffer);
	//Copy to preallocated buffer
	uv_err_t copyData(char *buffer);
	//Allocate memory and return it
	uv_err_t copyData(char **buffer);
	//Saves particular section to a file
	uv_err_t saveToFile(const std::string &file);
	
	bool operator==(UVDDataChunk &other);

	//min/max representation
	uint32_t getMin();
	uint32_t getMax();

	//offset/size representation
	uint32_t getOffset();
	uint32_t getSize();


private:
	//If data has not been read yet, read it
	uv_err_t ensureRead();

public:
	UVDData *m_data;
	unsigned int m_offset;
	unsigned int m_bufferSize;
	char *m_buffer;
};

#ifdef __cplusplus
extern "C"
{
#endif /* ifdef __cplusplus */

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* UV_DISASM_DATA_H */
