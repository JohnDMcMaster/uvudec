/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_ITERATOR_H
#define UVD_ITERATOR_H

#include "uvd/util/types.h"

/*
Used for keeping track of position when outputting files
Position is suppose to be the offset in a file
positionIndex is for when multiple details are on that line
Could keep internal or external cache of what is to come on that line
and generate all indexes at once
*/
class UVD;
class UVDAddressSpace;
class UVDIteratorCommon
{
public:
	//uv_err_t init(UVD *disassembler, uint32_t position = g_addr_min, uint32_t index = 0);
	virtual uv_err_t init(UVD *uvd, UVDAddressSpace *addressSpace);
	virtual uv_err_t init(UVD *uvd, UVDAddress address);
	virtual uv_err_t deinit();
	virtual ~UVDIteratorCommon();
	
	//Make it as UVD::end() would return
	//FIXME: this should be protected?
	virtual uv_err_t makeEnd();

	uv_addr_t getPosition();

	//Since the return type is not a pointer, needs to be defined in each subclass
	//UVDIteratorCommon operator++();
	//Advance to next output line
	virtual uv_err_t next();
	//Reminder: this is called directly for analysis
	uv_err_t parseCurrentInstruction();
	//Being done is pretty type specific

	//For instruction parsing
	//Will return UV_ERR_DONE on no more next data availible
	//If we successfully advanced but next is invalid,
	//makeNextEnd() will be called to prevent further advancement
	uv_err_t nextValidExecutableAddress();
	//Like above, but will not increment our current address first
	uv_err_t nextValidExecutableAddressIncludingCurrent();
	//Read current address and and advance
	//Return UV_ERR_DONE if current address is invalid
	uv_err_t consumeCurrentExecutableAddress(uint8_t *out);
	
protected:
	UVDIteratorCommon();
	//UVDIteratorCommon(UVD *disassembler = NULL);
	//UVDIteratorCommon(UVD *disassembler, uv_addr_t position, uint32_t index = 0);

	//Can still grab buffered strings, but next address iteration will result in .end()
	//virtual uv_err_t makeNextEnd();

	//Advance to next output group
	//If instructionIn is NULL, it will be ignored
	//Otherwise, the parsed instruction will be returned in place
	virtual uv_err_t nextCore();

	//Force printing of header information
	//Return UV_ERR_DONE if the first instruction should NOT be processed
	virtual uv_err_t initialProcess();
	
	//Clear buffers that were filled from previous instruction proces
	//Called before parsing next instruction
	virtual uv_err_t clearBuffers();
	
	//Some sort of disassembly issue
	virtual uv_err_t addWarning(const std::string &lineRaw);	

	//If we are out of bytes
	//Outside of this class, use iter != uvd->end()
	bool isEnd();

	//sets up begin()
	uv_err_t prime();

public:
	/*
	Source of data to disassemble
	Usually equal to m_uvd.m_data, but may be a smaller segment for finer analysis
	We do NOT own this, it must be deleted by the user
	Is this still needed?  Now favoring generating appropriete iters and having funcs pass in start and end iters
	XXX
	What we actually should have is a list of memory areas to iterate over
	Forget list, give a single area and they can iterate over their mult areas if needed
	Do a wrapper multi-iterater if really want it and keep that functionality separate
	/*/
	//UVDData *m_data;
	UVDAddressSpace *m_addressSpace;

	//Next position in file to check
	//FIXME FIXME FIXME
	//FIXME: this is now being used as current position
	//Also combine with m_addressSpace to form a UVDAddress m_address
	uv_addr_t m_nextPosition;
	//How many bytes we consumed to create currently analyzed instruction
	//Think this is actually more used to calculate instruction size than actual iteration
	//Maybe we should move it to a next() helper object
	uint32_t m_currentSize;
	//Last parsed instruction
	//Valid until nextInstruction() is called again
	//This may result in this being NULL if we aren't at a coding address
	UVDInstruction *m_instruction;

protected:	
	//Object we are iterating on
	UVD *m_uvd;
	//Printed startup information yet?
	int m_initialProcess;
	//Otherwise there are weird corner cases not knowing if we are actually at the end
	//or the highest address or looped back to start
	//int m_isEnd;
};

/*
Output printing iterator
*/
class UVDAnalyzedMemoryRange;
class UVDIterator : public UVDIteratorCommon
{
public:
	UVDIterator();
	~UVDIterator();
	virtual uv_err_t init(UVD *uvd, UVDAddressSpace *memorySpace);
	virtual uv_err_t init(UVD *uvd, UVDAddress address, uint32_t index);
	
	UVDIterator operator++();
	//Original reason for differentiation
	virtual bool operator==(const UVDIterator &other) const;
	bool operator!=(const UVDIterator &other) const;
	//Error checked version of operator *
	uv_err_t getCurrent(std::string &s);
	std::string operator*();

	virtual uv_err_t next();

	virtual uv_err_t initialProcess();
	//Subparts to organize better
	uv_err_t initialProcessHeader();
	uv_err_t initialProcessUselessASCIIArt();
	uv_err_t initialProcessStringTable();

	virtual uv_err_t makeEnd();
	//virtual uv_err_t makeNextEnd();
	static uv_err_t getEnd(UVD *uvd, UVDIterator &iter);

	//Current iterator position/status
	//void debugPrint() const;
	//Print a (tabbed) list of memory locations for current address based on type given
	uv_err_t printReferenceList(UVDAnalyzedMemoryRange *memLoc, uint32_t type);

	uv_err_t nextAddressLabel(uint32_t startPosition);
	uv_err_t nextAddressComment(uint32_t startPosition);
	uv_err_t nextCalledSources(uint32_t startPosition);
	uv_err_t nextJumpedSources(uint32_t startPosition);

protected:
	uv_err_t clearBuffers();
	virtual uv_err_t nextCore();

	//Some sort of disassembly issue
	virtual uv_err_t addWarning(const std::string &lineRaw);	
	//Add a comment to the end of the print buffer
	uv_err_t addComment(const std::string &lineRaw);

public:
	//Next index to check on generated lines
	//Public because seeing heavy use in instruction parsing
	//and got too messy passing a seperate ref var
	//Don't use it outside of that
	uint32_t m_positionIndex;
	//Lines to come from current index
	std::vector<std::string> m_indexBuffer;
};

/*
The issue here was primarily tracking when an iterator was done as well as clean advancement
While the UVDIterator for printing advances per line, this advances each instruction for analysis
*/
class UVDInstructionIterator : public UVDIteratorCommon
{
public:
	UVDInstructionIterator();
	~UVDInstructionIterator();
	
	static uv_err_t getEnd(UVD *uvd, UVDInstructionIterator &iter);

	UVDInstructionIterator operator++();
	bool operator==(const UVDInstructionIterator &other) const;
	bool operator!=(const UVDInstructionIterator &other) const;
};

#endif

