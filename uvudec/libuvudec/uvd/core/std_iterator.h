/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_STD_ITERATOR_H
#define UVD_STD_ITERATOR_H

#include "uvd/util/types.h"
#include "uvd/assembly/address.h"
#include "uvd/assembly/instruction.h"
#include "uvd/core/iterator.h"

/*
To implement you must define the parseInstruction() function in UVDArchitecture
	It was given as address, but probably should define an address and an iterator version?
	Hmm no, I think address is correct, iterator will follow
	Iterator was good though because it dealt with very clearly defining size and other parameters
	Current solution: provide an instruction convenience function, but the core will be iterator since its clear how to get
	an instruction from an iterator (if possible for current), but not necessarily the other way around

Some (current?) limitations of the common iterator:
-It will not disassemble instructions spanning multiple address spaces
	This would add complexity and I'm not sure if it makes sense on any architectures
*/

/*
Current high level iteration scheme seems more of a hack than anything else
Proper way to iterate is probably to have a collection of address spaces to iterate over
and get the address from each in turn
Or should that be a fundamental constraint to instruction iterator and it has to be dealt with at a higher level?
*/

/*
Used for keeping track of position when outputting files
Position is suppose to be the offset in a file
positionIndex is for when multiple details are on that line
Could keep internal or external cache of what is to come on that line
and generate all indexes at once
*/
class UVD;
class UVDAddressSpace;
//Address space instruction iterator
//As of this writing is the lowest level instruction iterator
class UVDASInstructionIterator
{
public:
	UVDASInstructionIterator();
	//uv_err_t init(UVD *disassembler, uint32_t position = g_addr_min, uint32_t index = 0);
	//uv_err_t init(UVD *uvd, UVDAddressSpace *addressSpace);
	uv_err_t init(UVD *uvd, UVDAddress address);
	uv_err_t deinit();
	virtual ~UVDASInstructionIterator();
	
	//Make it as UVD::end() would return
	//FIXME: this should be protected?
	uv_err_t makeEnd();

	virtual uv_addr_t getPosition();

	//Since the return type is not a pointer, needs to be defined in each subclass
	//UVDIteratorCommon operator++();
	//Advance to next output line
	virtual uv_err_t next();
	virtual uv_err_t previous();
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
	static uv_err_t getEnd(UVD *uvd, UVDAddressSpace *addressSpace, UVDASInstructionIterator &iter);
	UVDASInstructionIterator operator++();
	bool operator==(const UVDASInstructionIterator &other) const;
	bool operator!=(const UVDASInstructionIterator &other) const;
	int compare(const UVDASInstructionIterator &other) const;

	//If we are out of bytes
	//Outside of this class, use iter != uvd->end()
	bool isEnd();
	
	uv_err_t check();

protected:
	//sets up begin()
	uv_err_t prime();

	//Some sort of disassembly issue
	uv_err_t addWarning(const std::string &lineRaw);	

public:
	//TODO: should have a list of address spaces?

	/*
	Source of data to disassemble, there what and where
	*/
	UVDAddress m_address;

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
	
	void *m_objectUser;
	void *m_architectureUser;
};

class UVDStdInstructionIterator : public UVDAbstractInstructionIterator
{
public:
	UVDStdInstructionIterator(UVD *uvd = NULL);
	
	uv_err_t init(UVD *uvd, UVDAddressSpace *addressSpace);
	uv_err_t init(UVD *uvd, UVDAddress address);
	uv_err_t deinit();
	virtual ~UVDStdInstructionIterator();
	
	//virtual uv_addr_t getPosition();

	//Advance to next output line
	virtual uv_err_t next();
	virtual uv_err_t previous();
	//Reminder: this is called directly for analysis
	//uv_err_t parseCurrentInstruction();
	//Being done is pretty type specific

	virtual uv_err_t copy(UVDAbstractInstructionIterator **out) const;

	//Imported from old UVDIterator code
	//if addres space is given will return an iterator at the end of that addres space
	//NOTE: in order for this scheme to work we'll need to return elements at end() with no instruction as metadata
	//If address space is NULL will get the end of all address spaces
	static uv_err_t getEnd(UVD *uvd, UVDAddressSpace *addressSpace, UVDStdInstructionIterator **out);
	//static uv_err_t getEndFromExisting(UVD *uvd, UVDAddressSpace *addressSpace, UVDStdInstructionIterator *iter);
	//Should not be used directly, don't support this
	//UVDInstructionIterator operator++();
	virtual int compare(const UVDAbstractInstructionIterator &other) const;
	
	virtual uv_err_t getPosition(UVDAddress *out);
	virtual uv_err_t get(UVDInstruction **out) const;
	uv_err_t beginningOfCurrentAddressSpace();
		
	virtual uv_err_t check();
	
protected:
	uv_err_t setAddress(UVDAddress address);
	//uv_err_t makeEnd();
	bool isEnd();

public:
	//The spaces we want to iterate over
	std::vector<UVDAddressSpace *> m_addressSpaces;
	//The space we are currently iterating over
	std::vector<UVDAddressSpace *>::size_type m_addressSpacesIndex;
	//An iterator operating on a single address space
	UVDASInstructionIterator m_iter;
	UVD *m_uvd;
};

class UVDStdPrintIterator : public UVDAbstractPrintIterator
{
public:
	UVDStdPrintIterator();
	virtual ~UVDStdPrintIterator();
	//uv_err_t init(UVD *uvd, UVDAddressSpace *memorySpace);
	uv_err_t init(UVD *uvd, UVDAddress address, uint32_t index);
	
	//UVDPrintIterator operator++();
	//Original reason for differentiation
	virtual int compare(const UVDAbstractPrintIterator &other) const;
	virtual uv_err_t copy(UVDAbstractPrintIterator **out) const;
	//Error checked version of operator *
	uv_err_t getCurrent(std::string &out);
	virtual uv_err_t getAddress(UVDAddress *out);
	std::string operator*();

	uv_err_t next();
	uv_err_t previous();

	uv_err_t initialProcess();
	//Subparts to organize better
	uv_err_t initialProcessHeader();
	uv_err_t initialProcessStringTable();

	//uv_err_t makeEnd();
	//uv_err_t makeNextEnd();

	//Current iterator position/status
	//void debugPrint() const;
	//Print a (tabbed) list of memory locations for current address based on type given
	uv_err_t printReferenceList(UVDAnalyzedMemoryRange *memLoc, uint32_t type);

	uv_err_t nextAddressLabel(UVDAddress startPosition);
	uv_err_t nextAddressComment(UVDAddress startPosition);
	uv_err_t nextCalledSources(UVDAddress startPosition);
	uv_err_t nextJumpedSources(UVDAddress startPosition);

	static uv_err_t getEnd(UVD *uvd, UVDAddressSpace *addressSpace, UVDStdPrintIterator **out);
	
	virtual uv_err_t check();

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
	//UVDInstructionIterator *m_iter;
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

