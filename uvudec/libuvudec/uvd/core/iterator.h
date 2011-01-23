/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_ITERATOR_H
#define UVD_ITERATOR_H

#include "uvd/util/types.h"

/*
TODO: redesign this so that disassembly iterator contains an instruction iterator
It should make the code much cleaner

Used for keeping track of position when outputting files
Position is suppose to be the offset in a file
positionIndex is for when multiple details are on that line
Could keep internal or external cache of what is to come on that line
and generate all indexes at once
*/
class UVD;
class UVDAddressSpace;
class UVDInstructionIterator
{
public:
	//uv_err_t init(UVD *disassembler, uint32_t position = g_addr_min, uint32_t index = 0);
	uv_err_t init(UVD *uvd, UVDAddressSpace *addressSpace);
	uv_err_t init(UVD *uvd, UVDAddress address);
	uv_err_t deinit();
	~UVDInstructionIterator();
	
	//Make it as UVD::end() would return
	//FIXME: this should be protected?
	uv_err_t makeEnd();

	uv_addr_t getPosition();

	//Since the return type is not a pointer, needs to be defined in each subclass
	//UVDIteratorCommon operator++();
	//Advance to next output line
	uv_err_t next();
	uv_err_t previous();
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
	
	//Imported from old UVDIterator code
	static uv_err_t getEnd(UVD *uvd, UVDInstructionIterator &iter);
	UVDInstructionIterator operator++();
	bool operator==(const UVDInstructionIterator &other) const;
	bool operator!=(const UVDInstructionIterator &other) const;

//protected:	
	UVDInstructionIterator();
protected:
	//UVDInstructionIterator(UVD *disassembler = NULL);
	//UVDInstructionIterator(UVD *disassembler, uv_addr_t position, uint32_t index = 0);

	//sets up begin()
	uv_err_t prime();

	//Can still grab buffered strings, but next address iteration will result in .end()
	//uv_err_t makeNextEnd();
		
	//Some sort of disassembly issue
	uv_err_t addWarning(const std::string &lineRaw);	

	//If we are out of bytes
	//Outside of this class, use iter != uvd->end()
	bool isEnd();

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
	uv_addr_t m_curPosition;
	//How many bytes we consumed to create currently analyzed instruction
	//Think this is actually more used to calculate instruction size than actual iteration
	//Maybe we should move it to a next() helper object
	uint32_t m_currentSize;
	//Last parsed instruction
	//Valid until nextInstruction() is called again
	//This may result in this being NULL if we aren't at a coding address
	UVDInstruction *m_instruction;

	//Object we are iterating on
	UVD *m_uvd;
};

/*
Output printing iterator
*/
class UVDAnalyzedMemoryRange;
class UVDPrintIterator
{
public:
	UVDPrintIterator();
	~UVDPrintIterator();
	uv_err_t init(UVD *uvd, UVDAddressSpace *memorySpace);
	uv_err_t init(UVD *uvd, UVDAddress address, uint32_t index);
	
	UVDPrintIterator operator++();
	//Original reason for differentiation
	bool operator==(const UVDPrintIterator &other) const;
	bool operator!=(const UVDPrintIterator &other) const;
	//Error checked version of operator *
	uv_err_t getCurrent(std::string &out);
	std::string operator*();

	uv_err_t next();
	uv_err_t previous();

	uv_err_t initialProcess();
	//Subparts to organize better
	uv_err_t initialProcessHeader();
	uv_err_t initialProcessStringTable();

	uv_err_t makeEnd();
	//uv_err_t makeNextEnd();
	static uv_err_t getEnd(UVD *uvd, UVDPrintIterator &iter);

	//Current iterator position/status
	//void debugPrint() const;
	//Print a (tabbed) list of memory locations for current address based on type given
	uv_err_t printReferenceList(UVDAnalyzedMemoryRange *memLoc, uint32_t type);

	uv_err_t nextAddressLabel(uint32_t startPosition);
	uv_err_t nextAddressComment(uint32_t startPosition);
	uv_err_t nextCalledSources(uint32_t startPosition);
	uv_err_t nextJumpedSources(uint32_t startPosition);

protected:
	//sets up begin()
	uv_err_t prime();
	uv_err_t clearBuffers();
	uv_err_t parseCurrentLocation();

	//Some sort of disassembly issue
	uv_err_t addWarning(const std::string &lineRaw);	
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
	UVDInstructionIterator m_iter;

	//Printed startup information yet?
	//FIXME: get rid of this and instead check for start address condition maybe?
	//This yields lots of problems including more complicated code and annoying headers on small chunks we want to print
	//int m_initialProcess;
	//Otherwise there are weird corner cases not knowing if we are actually at the end
	//or the highest address or looped back to start
	//int m_isEnd;
};

#endif

