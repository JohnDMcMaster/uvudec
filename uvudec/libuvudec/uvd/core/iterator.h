/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_ITERATOR_H
#define UVD_ITERATOR_H

#include "uvd/util/types.h"
#include "uvd/assembly/address.h"
#include "uvd/assembly/instruction.h"

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

/*
Exposes functionality for analyzing instructions
If you use the standard print iterator this will also be used to fetch instructions
*/

class UVDAbstractInstructionIterator
{
public:
	//Including the UVD makes assumptions about how it works, they might want to ref arch instead for example
	UVDAbstractInstructionIterator(/*UVD *uvd*/);	
	virtual ~UVDAbstractInstructionIterator();
	
	//FIXME: this should return a UVDAddress
	//virtual uv_addr_t getPosition() = 0;
	virtual uv_err_t getPosition(UVDAddress *out) = 0;
	virtual uv_err_t next() = 0;

	//Default implemented using some tricks with next()
	//If fixed length instructions, consider overriding
	virtual uv_err_t previous();

	//UVDInstructionIterator operator++();
	//bool operator==(const UVDAbstractInstructionIterator &other) const;
	//bool operator!=(const UVDAbstractInstructionIterator &other) const;
	virtual int compare(const UVDAbstractInstructionIterator &other) const = 0;

	virtual uv_err_t get(UVDInstruction **out) const = 0;

	//TODO: figure out below, sounds like a hack
	//Reminder: this is called directly for analysis
	//uv_err_t parseCurrentInstruction();

	//Want this for previous() algorithm
	//Otherwise would have to do some UVD only stuff in UVDInstructionIterator
	//virtual int copyFrom(const UVDAbstractInstructionIterator *other) = 0;
	
public:
};

class UVDInstructionIterator
{
public:
	inline UVDInstructionIterator() {
		m_iter = NULL;
	}
	
	inline UVDInstructionIterator(UVDAbstractInstructionIterator *iter) {
		m_iter = iter;
	}
	
	inline ~UVDInstructionIterator() {
		delete m_iter;
	}
	
	//FIXME: this should return a UVDAddress
	//deprecated in favor of below
	//virtual uv_addr_t getPosition() = 0;
	inline uv_err_t getPosition(UVDAddress *out) {
		uv_assert_ret(m_iter != NULL);
		return m_iter->getPosition(out);
	}
	inline uv_err_t getAddress(UVDAddress *out) {
		uv_assert_ret(m_iter != NULL);
		return m_iter->getPosition(out);
	}
	
	inline uv_err_t next() {
		uv_assert_ret(m_iter != NULL);
		return m_iter->next();
	}

	//Default implemented using some tricks with next()
	//If fixed length instructions, consider overriding
	inline uv_err_t previous() {
		uv_assert_ret(m_iter != NULL);
		return m_iter->previous();
	}
	
	inline bool operator==(const UVDInstructionIterator &other) const {
		uv_assert_ret(m_iter != NULL);
		return m_iter->compare(*other.m_iter) == 0;
	}
	
	inline bool operator!=(const UVDInstructionIterator &other) const {
		uv_assert_ret(m_iter != NULL);
		return m_iter->compare(*other.m_iter) != 0;
	}
	
	inline int compare(const UVDInstructionIterator &other) const {
		uv_assert_ret(m_iter != NULL);
		return m_iter->compare(*other.m_iter);
	}

	//WARNING: out will bet set to NULL if we are at data, not instruction
	//this interface is still being refined
	inline uv_err_t get(UVDInstruction **out) const {
		uv_assert_ret(m_iter != NULL);
		return m_iter->get(out);
	}

	/*
	//TODO: figure out below, sounds like a hack
	//Reminder: this is called directly for analysis
	inline uv_err_t parseCurrentInstruction() {
		uv_assert_ret(m_iter != NULL);
		return m_iter->parseCurrentInstruction();
	}
	*/
	
public:
	//The wrapped implementation
	UVDAbstractInstructionIterator *m_iter;
};

/*
Output printing iterator
*/
class UVDAnalyzedMemoryRange;
class UVDAbstractPrintIterator {
public:
	UVDAbstractPrintIterator();
	virtual ~UVDAbstractPrintIterator();
	virtual uv_err_t init(UVD *uvd, UVDAddressSpace *memorySpace);
	virtual uv_err_t init(UVD *uvd, UVDAddress address, uint32_t index) = 0;
	
	virtual int compare(const UVDAbstractPrintIterator &other) const = 0;
	//Error checked version of operator *
	virtual uv_err_t getCurrent(std::string &out) = 0;
	//Needed for printing address columns and such
	virtual uv_err_t getAddress(UVDAddress *out) = 0;
	std::string operator*();

	virtual uv_err_t next() = 0;
	/*
	This is implemented by default as not support
	Warning: UVD GUI requires this to scroll back
	try to wrap around the standard iterator if this is an issue,
	it will try to figure out previous based on known positions
	This is not expected to be a fast operator as its only currently used by the GUI
	*/
	virtual uv_err_t previous();

	//static uv_err_t getEnd(UVD *uvd, UVDPrintIterator &iter);
};

class UVDPrintIterator {
public:
	inline UVDPrintIterator(UVDAbstractPrintIterator *iter = NULL) {
		m_iter = iter;
	}
	
	inline ~UVDPrintIterator() {
		delete m_iter;
	}
	
	inline uv_err_t init(UVD *uvd, UVDAddressSpace *memorySpace) {
		return m_iter->init(uvd, memorySpace);
	}
	
	inline uv_err_t init(UVD *uvd, UVDAddress address, uint32_t index) {
		return m_iter->init(uvd, address, index);
	}
	
	inline UVDPrintIterator operator++() {
		//FIXME: this isn't technically correct
		m_iter->next();
		return *this;
	}
	
	inline bool operator==(const UVDPrintIterator &other) const {
		return m_iter->compare(*other.m_iter) == 0;
	}
	
	inline bool operator!=(const UVDPrintIterator &other) const {
		return m_iter->compare(*other.m_iter) != 0;
	}
	
	inline int compare(const UVDPrintIterator &other) const {
		return m_iter->compare(*other.m_iter);
	}
	
	inline uv_err_t getCurrent(std::string &out) {
		return m_iter->getCurrent(out);
	}
	
	inline uv_err_t getAddress(UVDAddress *out) {
		return m_iter->getAddress(out);
	}
	
	inline std::string operator*() {
		return m_iter->operator*();
	}

	inline uv_err_t next() {
		return m_iter->next();
	}

	inline uv_err_t previous() {
		return m_iter->previous();
	}

	//Moved to UVDArchitecture virtual function
	//static uv_err_t getEnd(UVD *uvd, UVDPrintIterator &iter);

public:
	UVDAbstractPrintIterator *m_iter;
};

#endif

