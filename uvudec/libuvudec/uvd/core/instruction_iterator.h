/*
UVNet Universal Decompiler (uvudec)
Copyright 2008-2011 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_INSTRUCTION_ITERATOR_H
#define UVD_INSTRUCTION_ITERATOR_H

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

	virtual uv_err_t copy(UVDAbstractInstructionIterator **out) const = 0;
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
	
	//Perform error checking
	virtual uv_err_t check();
public:
};

class UVDInstructionIterator
{
public:
	inline UVDInstructionIterator() {
		m_iter = NULL;
	}
	
	inline UVDInstructionIterator(const UVDInstructionIterator &other) {
		m_iter = NULL;
		*this = other;
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
	
	int compare(const UVDInstructionIterator &other) const;
	/*
	inline int compare(const UVDInstructionIterator &other) const {
		uv_assert_ret(m_iter != NULL);
		return m_iter->compare(*other.m_iter);
	}
	*/

	uv_err_t operator=(const UVDInstructionIterator &other);
	/*
	inline uv_err_t operator=(const UVDInstructionIterator &other) {
		uv_assert_ret(other.m_iter);
		uv_assert_err_ret(other.m_iter->copy(&m_iter));
		uv_assert_ret(m_iter);
		//return *this;
		return UV_ERR_OK;
	}
	*/
	
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
	
	//Perform error checking
	uv_err_t check();
	
public:
	//The wrapped implementation
	UVDAbstractInstructionIterator *m_iter;
};

#endif

