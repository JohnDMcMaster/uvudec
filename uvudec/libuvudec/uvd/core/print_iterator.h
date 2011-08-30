/*
UVNet Universal Decompiler (uvudec)
Copyright 2008-2011 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_PRINT_ITERATOR_H
#define UVD_PRINT_ITERATOR_H

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
	
	virtual uv_err_t copy(UVDAbstractPrintIterator **out) const = 0;
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

	//Perform error checking
	virtual uv_err_t check();
};

class UVDPrintIterator {
public:
	inline UVDPrintIterator(UVDAbstractPrintIterator *iter = NULL) {
		m_iter = iter;
	}
	
	inline UVDPrintIterator(const UVDPrintIterator &other) {
		m_iter = NULL;
		*this = other;
	}
	
	~UVDPrintIterator();
	/*
	inline ~UVDPrintIterator() {
		printf("UVDPrintIterator: deleting 0x%08X\n", (int)m_iter);
		delete m_iter;
	}
	*/
	
	inline uv_err_t init(UVD *uvd, UVDAddressSpace *memorySpace) {
		uv_assert_ret(m_iter);
		return m_iter->init(uvd, memorySpace);
	}
	
	inline uv_err_t init(UVD *uvd, UVDAddress address, uint32_t index) {
		uv_assert_ret(m_iter);
		return m_iter->init(uvd, address, index);
	}
	
	inline UVDPrintIterator operator++() {
		//FIXME: this isn't technically correct
		m_iter->next();
		return *this;
	}
	
	inline bool operator==(const UVDPrintIterator &other) const {
		uv_assert_ret(m_iter);
		return m_iter->compare(*other.m_iter) == 0;
	}
	
	inline bool operator!=(const UVDPrintIterator &other) const {
		uv_assert_ret(m_iter);
		return m_iter->compare(*other.m_iter) != 0;
	}
	
	inline int compare(const UVDPrintIterator &other) const {
		uv_assert_ret(m_iter);
		return m_iter->compare(*other.m_iter);
	}
	
	inline uv_err_t operator=(const UVDPrintIterator &other) {
		uv_assert_ret(other.m_iter);
		uv_assert_err_ret(other.m_iter->copy(&m_iter));
		uv_assert_ret(m_iter);
		uv_assert_err_ret(m_iter->check());
		//return *this;
		return UV_ERR_OK;
	}
	
	inline uv_err_t getCurrent(std::string &out) {
		uv_assert_ret(m_iter);
		return m_iter->getCurrent(out);
	}
	
	inline uv_err_t getAddress(UVDAddress *out) {
		uv_assert_ret(m_iter);
		return m_iter->getAddress(out);
	}
	
	inline std::string operator*() {
		return m_iter->operator*();
	}

	inline uv_err_t next() {
		uv_assert_ret(m_iter);
		return m_iter->next();
	}

	inline uv_err_t previous() {
		uv_assert_ret(m_iter);
		return m_iter->previous();
	}

	//Perform error checking
	uv_err_t check();
	
	//Moved to UVDArchitecture virtual function
	//static uv_err_t getEnd(UVD *uvd, UVDPrintIterator &iter);

public:
	UVDAbstractPrintIterator *m_iter;
};

#endif

