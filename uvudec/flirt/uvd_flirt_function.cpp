/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd_crc.h"
#include "uvd_flirt_function.h"
#include "uvd_flirt_pattern.h"
#include <string.h>

/*
UVDFLIRTSignatureRawSequence
*/

#define SIGNATURE_ESCAPE_CHAR					'\\'
#define SIGNATURE_ESCAPED_CHAR_ESCAPED			SIGNATURE_ESCAPE_CHAR
//IDA calls these "variable bytes"
#define SIGNATURE_ESCAPED_CHAR_RELOCATION		UVD_FLIRT_PAT_RELOCATION_CHAR
#define SIGNATURE_ESCAPED_CHAR_END				((char)0)

/*
UVDFLIRTSignatureReference
*/

UVDFLIRTSignatureReference::UVDFLIRTSignatureReference()
{
	m_offset = 0;
	//m_node = NULL;
	m_attributeFlags = 0;
}

UVDFLIRTSignatureReference::~UVDFLIRTSignatureReference()
{
}

bool UVDFLIRTSignatureRawSequence::const_iterator::deref::operator==(const deref &other) const
{
	return (m_isReloc && other.m_isReloc) || (m_byte == other.m_byte);
}

bool UVDFLIRTSignatureRawSequence::const_iterator::deref::operator!=(const deref &other) const
{
	return !operator==(other);
}

/*
UVDFLIRTSignatureRawSequence
*/

uint32_t UVDFLIRTSignatureRawSequence::npos = 0xFFFFFFFF;

UVDFLIRTSignatureRawSequence::const_iterator::const_iterator()
{
	m_seq = NULL;
	m_cur = NULL;
}

UVDFLIRTSignatureRawSequence::const_iterator::const_iterator(const UVDFLIRTSignatureRawSequence *seq, const uint8_t *cur)
{
	m_seq = seq;
	m_cur = cur;
}

UVDFLIRTSignatureRawSequence::const_iterator::~const_iterator()
{
}

uv_err_t UVDFLIRTSignatureRawSequence::const_iterator::next()
{
	uv_assert_ret(m_seq);
	//Should not be at end()
	uv_assert_ret(m_cur);
	
	//Advance to next pos
	if( *m_cur == SIGNATURE_ESCAPE_CHAR )
	{
		++m_cur;
		if( *m_cur == SIGNATURE_ESCAPED_CHAR_END )
		{
			uv_assert_err_ret(makeEnd());
		}
		else
		{
			++m_cur;
		}
	}
	else
	{
		++m_cur;
	}

	return UV_ERR_OK;
}

bool UVDFLIRTSignatureRawSequence::const_iterator::operator==(const const_iterator &other) const
{
	return m_seq == other.m_seq && m_cur == other.m_cur;
}

bool UVDFLIRTSignatureRawSequence::const_iterator::operator!=(const const_iterator &other) const
{
	return !operator!=(other);
}

UVDFLIRTSignatureRawSequence::const_iterator::deref UVDFLIRTSignatureRawSequence::const_iterator::operator*()
{
	const_iterator::deref ret;
	
	if( !m_cur )
	{
		printf_error("bad iter deref, at end\n");
		return ret;
	}
	
	//And parse out our current info?
	if( *m_cur == SIGNATURE_ESCAPE_CHAR )
	{
		uint8_t escaped = *(m_cur + 1);
		
		if( escaped == SIGNATURE_ESCAPED_CHAR_RELOCATION )
		{
			ret.m_isReloc = true;
		}
		else if( escaped == SIGNATURE_ESCAPED_CHAR_ESCAPED )
		{
			ret.m_isReloc = false;
			ret.m_byte = SIGNATURE_ESCAPE_CHAR;
		}
		else if( escaped == SIGNATURE_ESCAPED_CHAR_END )
		{
			printf_error("escaped char and not end()\n");
			return ret;
		}
		else
		{
			printf_error("unknown escaped char: 0x%.2X\n", escaped);
			return ret;
		}
	}
	else
	{
		ret.m_isReloc = false;
		ret.m_byte = *m_cur;
	}

	return ret;
}

uv_err_t UVDFLIRTSignatureRawSequence::const_iterator::makeEnd()
{
	m_cur = NULL;
	return UV_ERR_OK;
}

UVDFLIRTSignatureRawSequence::const_iterator UVDFLIRTSignatureRawSequence::const_iterator::getEnd(const UVDFLIRTSignatureRawSequence *seq)
{
	return const_iterator(seq, NULL);
}

/*
UVDFLIRTSignatureRawSequence::iterator
*/

UVDFLIRTSignatureRawSequence::iterator::iterator()
{
}

UVDFLIRTSignatureRawSequence::iterator::iterator(UVDFLIRTSignatureRawSequence *seq, uint8_t *cur)
{
	m_seqNoConst = seq;
	m_curNoConst = cur;
}

UVDFLIRTSignatureRawSequence::iterator::~iterator()
{
}

UVDFLIRTSignatureRawSequence::iterator UVDFLIRTSignatureRawSequence::iterator::getEnd(UVDFLIRTSignatureRawSequence *seq)
{
	return iterator(seq, NULL);
}

/*
UVDFLIRTSignatureRawSequence
*/

UVDFLIRTSignatureRawSequence::UVDFLIRTSignatureRawSequence()
{
	m_bytes = NULL;
}

UVDFLIRTSignatureRawSequence::~UVDFLIRTSignatureRawSequence()
{
	free(m_bytes);
}

UVDFLIRTSignatureRawSequence::iterator UVDFLIRTSignatureRawSequence::begin()
{
	return iterator(this, m_bytes);
}

UVDFLIRTSignatureRawSequence::iterator UVDFLIRTSignatureRawSequence::end()
{
	return iterator::getEnd(this);
}

UVDFLIRTSignatureRawSequence::const_iterator UVDFLIRTSignatureRawSequence::const_begin() const
{
	return const_iterator(this, m_bytes);
}

UVDFLIRTSignatureRawSequence::const_iterator UVDFLIRTSignatureRawSequence::const_end() const
{
	return const_iterator::getEnd(this);
}

void UVDFLIRTSignatureRawSequence::transfer(UVDFLIRTSignatureRawSequence *other)
{
	other->m_bytes = m_bytes;
	m_bytes = NULL;
}

uv_err_t UVDFLIRTSignatureRawSequence::fromString(const std::string &s)
{
	//Hmm okay so heres a first problem with our technique, it is not as fast to allocate these, we must first guess the size
	//Scan over it first
	//2 bytes for terminator (escape + termination)
	uint32_t size = 2;
	for( std::string::size_type i = 0; i < s.size(); ++i )
	{
		char c = s[i];
		
		//We escape this
		if( c == SIGNATURE_ESCAPE_CHAR )
		{
			++size;
		}
		++size;
	}
	
	m_bytes = (uint8_t *)malloc(size);
	uv_assert_ret(m_bytes);
	
	uint32_t pos = 0;
	for( std::string::size_type i = 0; i < s.size(); ++i, ++pos )
	{
		char c = s[i];
		if( c == SIGNATURE_ESCAPE_CHAR )
		{
			m_bytes[pos] = SIGNATURE_ESCAPE_CHAR;
			++pos;
			m_bytes[pos] = SIGNATURE_ESCAPED_CHAR_ESCAPED;
		}
		else if( c == UVD_FLIRT_PAT_RELOCATION_CHAR )
		{
			m_bytes[pos] = SIGNATURE_ESCAPE_CHAR;
			++pos;
			m_bytes[pos] = SIGNATURE_ESCAPED_CHAR_RELOCATION;
		}
		else
		{
			m_bytes[pos] = c;
		}
	}
	m_bytes[pos] = SIGNATURE_ESCAPE_CHAR;
	++pos;
	m_bytes[pos] = SIGNATURE_ESCAPED_CHAR_END;
	++pos;
	
	uv_assert_ret(pos == size);
	
	return UV_ERR_OK;
}

std::string UVDFLIRTSignatureRawSequence::toString() const
{
	std::string ret;
	uint8_t *cur = m_bytes;
	 
	if( !m_bytes )
	{
		return "";
	}
	
	for( ;; )
	{
		if( *cur == SIGNATURE_ESCAPE_CHAR )
		{
			++cur;
			if( *cur == '.' )
			{
				ret += ".";
				continue;
			}
			else if( *cur == 0 )
			{
				break;
			}
			//Fall through to print if the actual char is '\'
			else if( *cur != SIGNATURE_ESCAPE_CHAR )
			{
				printf_error("corrupted raw signature char: %c\n", *cur);
				ret += "?";
				return ret;
			}
		}
		
		char buff[3];
		snprintf(buff, 3, "%.2X", *cur);
		ret += buff;
	}

	return ret;
}

bool UVDFLIRTSignatureRawSequence::operator==(const UVDFLIRTSignatureRawSequence *other) const
{
	return compare(other) == 0;
}

bool UVDFLIRTSignatureRawSequence::operator!=(const UVDFLIRTSignatureRawSequence *other) const
{
	return compare(other) != 0;
}

int UVDFLIRTSignatureRawSequence::compare(const UVDFLIRTSignatureRawSequence *other) const
{
	//Walk down the sequence
	const_iterator iterThis = const_begin();
	const_iterator iterOther = other->const_begin();
	
	for( ;; )
	{
		if( iterThis == const_end() && iterOther == other->const_end() )
		{
			return 0;
		}
		//Consider smaller sequences to be less than others for same prefix
		if( iterThis == const_end() )
		{
			return INT_MIN;
		}
		if( iterOther == other->const_end() )
		{
			return INT_MAX;
		}
		
		const_iterator::deref derefThis = *iterThis;
		const_iterator::deref derefOther = *iterOther;
		if( derefThis != derefOther )
		{
			uv_assert_ret(!(*iterThis).m_isReloc);
			uv_assert_ret(!(*iterOther).m_isReloc);
			return (*iterThis).m_byte - (*iterOther).m_byte;
		}
	}
}

uint32_t UVDFLIRTSignatureRawSequence::size() const
{
	uint32_t ret = 0;
	for( const_iterator iter = const_begin(); iter != const_end(); UV_DEBUG(iter.next()) )
	{
		++ret;
	}
	return ret;
}

uint32_t UVDFLIRTSignatureRawSequence::allocSize() const
{
	//Room needed for termination
	uint32_t ret = 2;
	for( const_iterator iter = const_begin(); iter != const_end(); UV_DEBUG(iter.next()) )
	{
		++ret;
		if( (*iter).m_isReloc )
		{
			++ret;
		}
	}
	return ret;
}

uint32_t UVDFLIRTSignatureRawSequence::allocSizeFrom(const_iterator start) const
{
	//Room needed for termination
	uint32_t ret = 2;
	for( const_iterator iter = start; iter != const_end(); UV_DEBUG(iter.next()) )
	{
		++ret;
		if( (*iter).m_isReloc )
		{
			++ret;
		}
	}
	return ret;
}

uv_err_t UVDFLIRTSignatureRawSequence::subseqTo(UVDFLIRTSignatureRawSequence *dest, const_iterator pos, uint32_t n) const
{
	uint32_t size = 0;

	size = allocSizeFrom(pos);
	//Alloc
	//Should we free dest->m_bytes?
	dest->m_bytes = (uint8_t *)malloc(size);
	uv_assert_ret(dest->m_bytes);
	//Copy
	memcpy(dest->m_bytes, pos.m_cur, size);
		
	return UV_ERR_OK;
}

uv_err_t UVDFLIRTSignatureRawSequence::copyTo(UVDFLIRTSignatureRawSequence *dest) const
{
	uint32_t size = 0;
	
	size = allocSize();
	//Alloc
	dest->m_bytes = (uint8_t *)malloc(size);
	uv_assert_ret(dest->m_bytes);
	//Copy
	memcpy(dest->m_bytes, m_bytes, size);
		
	return UV_ERR_OK;
}

uv_err_t UVDFLIRTSignatureRawSequence::matchPosition(const UVDFLIRTSignatureRawSequence *seq, const_iterator &otherStartEndIter, const_iterator &thisMatchPointIter) const
{
	thisMatchPointIter = const_begin();
	//It is expected seq should be longer or equal to our seq
	//End position should be relative to 
	while( otherStartEndIter != seq->const_end() && thisMatchPointIter != const_end() && (*otherStartEndIter) != (*thisMatchPointIter) )
	{
		uv_assert_err_ret(otherStartEndIter.next());
		uv_assert_err_ret(thisMatchPointIter.next());
	}
	
	return UV_ERR_OK;
}

uv_err_t UVDFLIRTSignatureRawSequence::truncate(iterator pos)
{
	//Skip if at end()
	if( pos.m_cur )
	{
		//FIXME
		//Could switch out with a subseq trick?
		//We are guranteed to have at least as much space b/c of terminating block
		//Don't bother reallocating, probably not worth the effort for now
		pos.m_curNoConst[0] = SIGNATURE_ESCAPE_CHAR;
		pos.m_curNoConst[1] = SIGNATURE_ESCAPED_CHAR_END;
	}

	return UV_ERR_OK;
}

/*
UVDFLIRTFunction
*/
UVDFLIRTFunction::UVDFLIRTFunction()
{
	m_totalLength = 0;
	m_crc16Length = 0;
	m_crc16 = 0;
	m_attributeFlags = 0;
}

UVDFLIRTFunction::~UVDFLIRTFunction()
{
}

/*
static uv_err_t zeroedBuffer(const UVDFLIRTSignatureRawSequence *seq, uint8_t **out)
{
	uint32_t size = 0;
	uint8_t *ret = NULL;
	uint8_t *cur = NULL;
	
	size = seq->size();
	ret = (uint8_t *)malloc(size);
	uv_assert_ret(ret);
	cur = ret;
	for( UVDFLIRTSignatureRawSequence::const_iterator iter = seq->const_begin(); iter != seq->const_end(); UV_DEBUG(iter.next()) )
	{
		if( (*iter).m_isReloc )
		{
			*cur = 0;
		}
		else
		{
			*cur = (*iter).m_byte;
		}
		++cur;
	}
	
	uv_assert_ret(out);
	*out = ret;

	return UV_ERR_OK;
}

uv_err_t UVDFLIRTFunction::computeCRC16()
{
	uint8_t *crcBuff = NULL;
	
	m_crc16Length = m_tailingSequence.size();
	if( m_crc16Length )
	{
		uv_assert_err_ret(zeroedBuffer(&m_tailingSequence, &crcBuff));
		m_crc16 = uvd_crc16((const char *)crcBuff, m_crc16Length);
	}
	else
	{
		m_crc16 = 0x0000;
	}

	free(crcBuff);	
	return UV_ERR_OK;
}
*/

uv_err_t UVDFLIRTFunction::transferSequence(UVDFLIRTSignatureRawSequence *sequence)
{
	sequence->transfer(&m_sequence);

	return UV_ERR_OK;
}

uv_err_t UVDFLIRTFunction::setSequence(const UVDFLIRTSignatureRawSequence *sequence)
{
	sequence->copyTo(&m_sequence);
	
	return UV_ERR_OK;
}

