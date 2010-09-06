/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_FLIRT_FUNCTION_H
#define UVD_FLIRT_FUNCTION_H

#include "uvd_types.h"

//class UVDFLIRTSignatureTreeLeafNode;
class UVDFLIRTSignatureReference
{
public:
	UVDFLIRTSignatureReference();
	~UVDFLIRTSignatureReference();

public:
	//.sig file uses m_offset from data area, but here offset should be from chain 
	uint32_t m_offset;
	//If we scale this up, switch this to a char * or std::string *
	//Or should this be UVDFLIRTSignatureTreeLeafNode and store a list of names in the main node struct?
	//Keep both for now, eliminate m_name if it seems useless, it may only be needed for construction
	std::string m_name;
	//The symbol we are referencing, if present
	//Don't do this as we might have multiple implementations and it really should be by name
	//UVDFLIRTSignatureTreeLeafNode *m_node;
	//I'm not too familar with what to do with these
	//File had it at 8 bits
	uint8_t m_attributeFlags;
};

class UVDFLIRTSignatureRawSequence
{
public:	
	//This is me playing around with how to make an STL like iterator class
	class const_iterator
	{
	public:
		class deref
		{
		public:
			bool operator==(const deref &other) const;
			bool operator!=(const deref &other) const;
		
		public:
			uint8_t m_isReloc;
			uint8_t m_byte;
		};
		
	public:
		const_iterator();
		const_iterator(const UVDFLIRTSignatureRawSequence *seq, const uint8_t *cur);
		~const_iterator();
		
		uv_err_t next();
		
		bool operator==(const const_iterator &other) const;
		bool operator!=(const const_iterator &other) const;
		
		//Only read for now
		deref operator*();

		static const_iterator getEnd(const UVDFLIRTSignatureRawSequence *seq);
	
	protected:
		uv_err_t makeEnd();
		
	public:
		//We might not actually need this
		union
		{
			const UVDFLIRTSignatureRawSequence *m_seq;
			UVDFLIRTSignatureRawSequence *m_seqNoConst;
		};
		union
		{
			const uint8_t *m_cur;
			uint8_t *m_curNoConst;
		};

		//uint8_t m_isReloc;
		//Only valid if !m_isReloc
		//uint8_t m_byte;		
	};
	//Add non-const functions
	class iterator : public const_iterator
	{
	public:
		iterator();
		iterator(UVDFLIRTSignatureRawSequence *seq, uint8_t *cur);
		~iterator();

		static iterator getEnd(UVDFLIRTSignatureRawSequence *seq);
	};

public:
	UVDFLIRTSignatureRawSequence();
	~UVDFLIRTSignatureRawSequence();

	iterator begin();
	iterator end();
	const_iterator const_begin() const;
	const_iterator const_end() const;

	//Transfer allocated  memory to given other
	void transfer(UVDFLIRTSignatureRawSequence *other);
	
	//We may later need to specify the length
	uv_err_t fromString(const std::string &s);
	
	//For debugging
	//Upon error we truncate the print, ending in a ?
	std::string toString() const;
	
	//How long is it in bytes
	//If relocations are at the end, they will be included
	uint32_t size() const;
	
	/*
	thisMatchPoint and otherStartEnd returned
	otherStartEnd: position to start on other as input
	position is not inclusive
		.begin(): no match
		.end(): full match
	*/
	uv_err_t matchPosition(const UVDFLIRTSignatureRawSequence *other, const_iterator &otherStartEnd, const_iterator &thisMatchPoint) const;

	bool operator==(const UVDFLIRTSignatureRawSequence *other) const;
	bool operator!=(const UVDFLIRTSignatureRawSequence *other) const;
	//Returns positive if this is greater than other
	int compare(const UVDFLIRTSignatureRawSequence *other) const;

	//modeled off of std::string::substr
	//Return starting at pos and of length n
	//UVDFLIRTSignatureRawSequence *subseq(uint32_t pos, uint32_t n);
	//uv_err_t subseqTo(UVDFLIRTSignatureRawSequence *dest, const_iterator pos) const;
	uv_err_t subseqTo(UVDFLIRTSignatureRawSequence *dest, const_iterator pos, uint32_t n = npos) const;
	//uv_err_t subseqToNew(UVDFLIRTSignatureRawSequence *dest, iterator pos, uint32_t n);
	//Truncate everthing including and after pos
	uv_err_t truncate(iterator pos);
	UVDFLIRTSignatureRawSequence *copy();
	uv_err_t copyTo(UVDFLIRTSignatureRawSequence *dest) const;

protected:
	uint32_t allocSize() const;
	uint32_t allocSizeFrom(const_iterator start) const;

public:
	static uint32_t npos;
	
	//Try linear technique now
	//Provides for easier substrings, although slightly less memory efficient
	//We own this
	//Maximum size should be around 64
	uint8_t *m_bytes;
};

#if 0
class UVDFLIRTSignatureRawSequence
{
public:
	UVDFLIRTSignatureRawSequence();
	~UVDFLIRTSignatureRawSequence();

	//Transfer allocated  memory to given other
	void transfer(UVDFLIRTSignatureRawSequence *other);
	
	//We may later need to specify the length
	uv_err_t fromString(const std::string &s);
	
	//For debugging
	//Upon error we truncate the print, ending in a ?
	std::string toString();
	
	bool operator==(UVDFLIRTSignatureRawSequence *other) const;
	//Returns positive if this is greater than other
	int compare(UVDFLIRTSignatureRawSequence *other) const;

	//modeled off of std::string::substr
	UVDFLIRTSignatureRawSequence *subseq(uint32_t pos, uint32_t n);
	UVDFLIRTSignatureRawSequence *copy();

public:
	//Try escaped technique for now
	//We own this
	//Maximum size should be around 64
	uint8_t *m_bytes;
};
#endif

/*
A basic function not stored in a tree or resolved for conflicts
Basically what you'd find in a .pat file
Intended as an intermediate representation for exchange between sig/pat processors
*/
class UVDFLIRTFunction
{
public:
	UVDFLIRTFunction();
	~UVDFLIRTFunction();
	
	//Implement if we need to get a fully copy
	//uv_err_t getSequence(UVDFLIRTSignatureRawSequence **out);
	//uv_err_t setSequence(const UVDFLIRTSignatureRawSequence *in);
	//For now, for practical reasons, its best to store the leading and tailing bytes separatly
	
	//Fill in the crc16 field based on m_tailingSequence
	uv_err_t computeCRC16();
	//Split sequence into areas
	//uv_err_t compute();
	
	//Sequence in will be invalidated
	uv_err_t transferSequence(UVDFLIRTSignatureRawSequence *m_sequence);
	//Not invalidated
	uv_err_t setSequence(const UVDFLIRTSignatureRawSequence *m_sequence);

protected:
	//Get a copy of sequence with relocations 0 filled
	//Originally for computing crc16 for functions
	//Allocated with malloc and callees responsibility to free
	//uv_err_t zeroedBuffer(uint8_t **out) const;

public:
	//Entire length including prefix and tailing bytes, if present
	uint32_t m_totalLength;
	//This admitedly can be somewhat vague
	//Its up to the definer to make sure these don't collide
	//This code should NOT map these to current code objects, this should be handeled externally
	std::vector<std::string> m_publicNames;
	std::vector<UVDFLIRTSignatureReference> m_references;
	/*
	How many bytes were used to calc the crc16
	If this is 0, m_crc16 should also be 0
	*/
	uint32_t m_crc16Length;
	uint16_t m_crc16;
	//Need to define these
	uint32_t m_attributeFlags;
	
	//Because of split relocations, much easier to just have a single sequence
	//Maximum of length 0x20
	//UVDFLIRTSignatureRawSequence m_leadingSequence;
	//Post 0x20 length stuff used for 
	//UVDFLIRTSignatureRawSequence m_tailingSequence;
	UVDFLIRTSignatureRawSequence m_sequence;
};

#endif

