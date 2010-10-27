/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/hash/crc.h"
#include "uvd/flirt/function.h"
#include "uvd/flirt/pat/pat.h"
#include "uvd/flirt/flirt.h"
#include "uvd/util/util.h"
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

int UVDFLIRTSignatureReference::compare(const UVDFLIRTSignatureReference *r) const
{
	int delta = 0;
	
	if( this == r )
	{
		return 0;
	}
	if( this == NULL )
	{
		return INT_MIN;
	}
	if( r == NULL )
	{
		return INT_MAX;
	}

	//It doesn't make sense to have two references at the same location...I think
	//So it should be the only compare needed
	//But lets do both just to be careful
	delta = m_offset - r->m_offset;
	if( delta )
	{
		return delta;
	}

	delta = m_name.compare(r->m_name);
	if( delta )
	{
		return delta;
	}
	
	return m_attributeFlags - r->m_attributeFlags;
}

int UVDFLIRTSignatureRawSequence::const_iterator::deref::compare(const deref &other) const
{
	if( m_isReloc && other.m_isReloc )
	{
		return 0;
	}
	return m_byte - other.m_byte;
}

bool UVDFLIRTSignatureRawSequence::const_iterator::deref::operator==(const deref &other) const
{
	return compare(other) == 0;
}

bool UVDFLIRTSignatureRawSequence::const_iterator::deref::operator!=(const deref &other) const
{
	return compare(other) != 0;
}

std::string UVDFLIRTSignatureRawSequence::const_iterator::deref::toString() const
{
	if( m_isReloc )
	{
		return ".";
	}
	else
	{
		char buff[3];

		snprintf(buff, 3, "%02X", m_byte);
		return std::string(buff);
	}
}

/*
UVDFLIRTSignatureRawSequence
*/

/*
FIXME: treatment of end() is messy
Current
	end is when m_cur is NULL
Maybe better
	end is when m_cur is the last byte
Seemed like a good idea at the time, but caused a lot of special cases
such as making sure we begin() correctly
	although we probably shouldn't be dealing in empty seqs anyway

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
	if( m_cur )
	{
		UV_DEBUG(checkEnd());
	}
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
	}
	++m_cur;
	
	uv_assert_err_ret(checkEnd());

	return UV_ERR_OK;
}

uv_err_t UVDFLIRTSignatureRawSequence::const_iterator::checkEnd()
{
	/*
	FIXME
	This adds a lot of overhead
	We should see if theres a more efficien way to do this
	Maybe during construction we could store a pointer to end() so that .end() wouldn't have to scan each time?
	*/
	//See if we are at .end()
	if( *m_cur == SIGNATURE_ESCAPE_CHAR && *(m_cur + 1) == SIGNATURE_ESCAPED_CHAR_END )
	{
		uv_assert_err_ret(makeEnd());
	}
	return UV_ERR_OK;
}

int UVDFLIRTSignatureRawSequence::const_iterator::compare(const const_iterator &other) const
{
	return m_cur - other.m_cur;
}

bool UVDFLIRTSignatureRawSequence::const_iterator::operator==(const const_iterator &other) const
{
	return compare(other) == 0;
}

bool UVDFLIRTSignatureRawSequence::const_iterator::operator!=(const const_iterator &other) const
{
	return compare(other) != 0;
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
			UVD_PRINT_STACK();
			ret.m_isReloc = false;
			ret.m_byte = 0;
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

std::string UVDFLIRTSignatureRawSequence::const_iterator::toString() const
{
	//Put paren around current seq pos
	//if at end, just empty paren at end
	std::string ret;
	for( const_iterator iter = m_seq->const_begin(); ; )
	{
		if( iter == *this )
		{
			ret += "(";
		}
		if( iter != m_seq->const_end() )
		{
			ret += (*iter).toString();
		}
		if( iter == *this )
		{
			ret += ")";
		}
		if( iter == m_seq->const_end() )
		{
			break;
		}
		UV_DEBUG(iter.next());
	}
	return ret;
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

uv_err_t UVDFLIRTSignatureRawSequence::fromStringAllocSize(const std::string &s, uint32_t lengthIn, uint32_t *sizeOut)
{
	//Hmm okay so heres a first problem with our technique, it is not as fast to allocate these, we must first guess the size
	//Scan over it first
	//2 bytes for terminator (escape + termination)
	uint32_t size = 2;
	//Takes two chars to represent a byte
	uv_assert_ret(s.size() % 2 == 0);
	for( std::string::size_type i = 0; i + 1 < s.size(); )
	{
		char first = s[i];
		
		//early termination?
		if( lengthIn != npos )
		{
			if( i >= lengthIn * 2 )
			{
				break;
			}
		}
		
		//We escape this
		if( first == UVD_FLIRT_PAT_RELOCATION_CHAR )
		{
			++size;
		}
		else
		{
			char buff[3];
			uint8_t byte = 0;
			
			buff[0] = s[i];
			buff[1] = s[i + 1];
			buff[2] = 0;
			
			byte = strtol(&buff[0], NULL, 16);
			if( byte == SIGNATURE_ESCAPE_CHAR )
			{
				++size;
			}	
		}
		++size;
		i += 2;
	}
	
	uv_assert_ret(sizeOut);
	*sizeOut = size;
	return UV_ERR_OK;
}

uv_err_t UVDFLIRTSignatureRawSequence::fromString(const std::string &s)
{
	return UV_DEBUG(fromStringCore(s, npos));
}

uv_err_t UVDFLIRTSignatureRawSequence::fromStringCore(const std::string &s, uint32_t lengthIn)
{
	printf_flirt_debug("loading raw sequence 0x%08X (len: %d) from string: %s\n", (int)this, s.size(), s.c_str());

	uint32_t size = 0;
	
	uv_assert_err_ret(fromStringAllocSize(s, lengthIn, &size));
	
	m_bytes = (uint8_t *)malloc(size);
	uv_assert_ret(m_bytes);
	//Poison
	memset(m_bytes, 0xCD, size);
	printf_flirt_debug("allocated %d bytes of pattern memory\n", size);
	
	uint32_t pos = 0;
	for( std::string::size_type i = 0; i + 1 < s.size(); i += 2, ++pos )
	{
		char first = s[i];
		char second = s[i + 1];
		
		//early termination?
		if( lengthIn != npos )
		{
			if( i >= lengthIn * 2 )
			{
				break;
			}
		}

		uv_assert_ret(pos < size);
		if( first == UVD_FLIRT_PAT_RELOCATION_CHAR )
		{
			if( second != UVD_FLIRT_PAT_RELOCATION_CHAR )
			{
				printf_error("relocation chars must be in pairs, invalid at position %d in sequence %s\n", i, s.c_str());
				return UV_DEBUG(UV_ERR_GENERAL);
			}
			m_bytes[pos] = SIGNATURE_ESCAPE_CHAR;
			++pos;
			uv_assert_ret(pos < size);
			m_bytes[pos] = SIGNATURE_ESCAPED_CHAR_RELOCATION;
		}
		else
		{
			char buff[3];
			uint8_t byte = 0;
		
			buff[0] = first;
			buff[1] = second;
			buff[2] = 0;
		
			byte = strtol(&buff[0], NULL, 16);
			if( byte == SIGNATURE_ESCAPE_CHAR )
			{
				m_bytes[pos] = SIGNATURE_ESCAPE_CHAR;
				++pos;
				uv_assert_ret(pos < size);
				m_bytes[pos] = SIGNATURE_ESCAPED_CHAR_ESCAPED;
			}
			else
			{
				m_bytes[pos] = byte;
			}
		}
	}
	uv_assert_ret(pos < size);
	m_bytes[pos] = SIGNATURE_ESCAPE_CHAR;
	++pos;
	if( pos >= size )
	{
		printf_flirt_debug("pos: %d, size: %d, target length: %d\n", pos, size, lengthIn);
		hexdump(m_bytes, size);
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	m_bytes[pos] = SIGNATURE_ESCAPED_CHAR_END;
	++pos;
	uv_assert_ret(pos == size);
	
	printf_flirt_debug("finished fromString()\n");
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
				ret += "..";
				++cur;
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
		++cur;
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
	if( this == other )
	{
		return 0;
	}
	if( this == NULL )
	{
		return INT_MIN;
	}
	if( other == NULL )
	{
		return INT_MAX;
	}

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
		
		UV_DEBUG(iterThis.next());
		UV_DEBUG(iterOther.next());
	}
	
	return 0;
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

bool UVDFLIRTSignatureRawSequence::empty() const
{
	if( !m_bytes )
	{
		return true;
	}
	if( *m_bytes == SIGNATURE_ESCAPE_CHAR && *(m_bytes + 1) == SIGNATURE_ESCAPED_CHAR_END )
	{
		return true;
	}
	return false;
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

uint32_t UVDFLIRTSignatureRawSequence::allocSizeFrom(const_iterator start, uint32_t n) const
{
	//Room needed for termination
	uint32_t ret = 2;
	//printf_flirt_debug("allocSizeFrom()\n");
	//hexdump(m_bytes, 0x40);
	for( const_iterator iter = start; iter != const_end(); UV_DEBUG(iter.next()) )
	{
		//printf_flirt_debug("iteration, offset: 0x%.8X, 0x%.2X\n", iter.m_cur, *iter.m_cur);
		++ret;
		if( *iter.m_cur == SIGNATURE_ESCAPE_CHAR )
		{
			++ret;
		}
		if( n != npos )
		{
			--n;
			if( n == 0 )
			{
				break;
			}
		}
	}
	return ret;
}

uv_err_t UVDFLIRTSignatureRawSequence::subseqTo(UVDFLIRTSignatureRawSequence *dest, const_iterator pos, uint32_t n) const
{
	//We need n to split and retain the leading node part

	uint32_t size = 0;

	size = allocSizeFrom(pos, n);
	//Should have at least 1 byte + 2 for terminate
	//maybe 0 if we want empty seq in future
	uv_assert_ret(size >= 3);
	uv_assert_ret(!empty());
	printf_flirt_debug("subseqTo, alloc size: 0x%.2X, seq: %s, pos: 0x%.8X (m_bytes: 0x%.8X), n: 0x%.8X\n", size, toString().c_str(), pos.m_cur, m_bytes, n);
	//Alloc
	//Should we free dest->m_bytes?
	uv_assert_ret(dest);
	dest->m_bytes = (uint8_t *)malloc(size);
	uv_assert_ret(dest->m_bytes);
	//Copy
	memcpy(dest->m_bytes, pos.m_cur, size);
	//And properly terminate
	dest->m_bytes[size - 2] = SIGNATURE_ESCAPE_CHAR;
	dest->m_bytes[size - 1] = SIGNATURE_ESCAPED_CHAR_END;
	
	printf_flirt_debug("constructed subseq (len = 0x%.2X): %s\n", dest->size(), dest->toString().c_str());
	if( n != npos )
	{
		uv_assert_ret(dest->size() == n);
	}	
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

uv_err_t UVDFLIRTSignatureRawSequence::differencePosition(const UVDFLIRTSignatureRawSequence *otherSeq, const_iterator &otherStartEndIter, const_iterator &thisMatchPointIter) const
{
	thisMatchPointIter = const_begin();
	//It is expected seq should be longer or equal to our seq
	//End position should be relative to 
	while( otherStartEndIter != otherSeq->const_end() && thisMatchPointIter != const_end() && (*otherStartEndIter) == (*thisMatchPointIter) )
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
UVDFLIRTModule
*/
UVDFLIRTModule::UVDFLIRTModule()
{
	m_totalLength = 0;
	m_crc16Length = 0;
	m_crc16 = 0;
	m_attributeFlags = 0;
}

UVDFLIRTModule::~UVDFLIRTModule()
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

uv_err_t UVDFLIRTModule::computeCRC16()
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

uv_err_t UVDFLIRTModule::transferSequence(UVDFLIRTSignatureRawSequence *sequence)
{
	sequence->transfer(&m_sequence);

	return UV_ERR_OK;
}

uv_err_t UVDFLIRTModule::setSequence(const UVDFLIRTSignatureRawSequence *sequence)
{
	sequence->copyTo(&m_sequence);
	
	return UV_ERR_OK;
}

std::string UVDFLIRTModule::debugString()
{
	std::string ret;

	for( std::vector<UVDFLIRTPublicName>::iterator iter = m_publicNames.begin();
			iter != m_publicNames.end(); ++iter )
	{
		UVDFLIRTPublicName &publicName = (*iter);
		char buff[256];
		
		snprintf(buff, sizeof(buff), ":%04X %s", publicName.m_offset, publicName.m_name.c_str());		
		if( iter != m_publicNames.begin() )
		{
			ret += " ";
		}
		ret += buff;
	}	
	return ret;	
}

/*
UVDFLIRTPublicName
*/

UVDFLIRTPublicName::UVDFLIRTPublicName()
{
	m_offset = 0;
	UVD_POKE(this);
}

UVDFLIRTPublicName::UVDFLIRTPublicName(const std::string &name, uint32_t offset)
{
	m_name = name;
	m_offset = offset;
	UVD_POKE(this);
}

UVDFLIRTPublicName::~UVDFLIRTPublicName()
{
}

int UVDFLIRTPublicName::compare(const UVDFLIRTPublicName *other) const
{
	int delta = 0;
	
	if( this == other )
	{
		return 0;
	}
	if( this == NULL )
	{
		return INT_MIN;
	}
	if( other == NULL )
	{
		return INT_MAX;
	}
	
	//Offset seems more important than name
	delta =  m_offset - other->m_offset;
	if( delta )
	{
		return delta;
	}
	
	return m_name.compare(other->m_name);
}

