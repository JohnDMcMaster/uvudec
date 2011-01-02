/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the GPL V3 or later, see COPYING for details
*/

#include "uvqt/hexdump_data.h"
#include "uvd/util/debug.h"

/*
UVQtHexdumpData::iterator_impl
*/

UVQtHexdumpData::iterator_impl::iterator_impl()
{
	m_offset = 0;
	m_dataImpl = NULL;
}

UVQtHexdumpData::iterator_impl::iterator_impl(UVQtHexdumpData *impl, unsigned int offset, unsigned int index)
{
	//printf("new UVQtHexdumpData = 0x%08X\n", (int)this);
	m_dataImpl = impl;
	/*
	if( m_dataImpl == NULL )
	{
	printf("bad data impl\n");
	//UVD_PRINT_STACK();
	}
	*/
	m_offset = scrollbarPositionToOffset(offset);
}

UVQtHexdumpData::iterator_impl::~iterator_impl()
{
	//printf("killing UVQtHexdumpData::iterator_impl = 0x%08X\n", (int)this);
	//UVD_PRINT_STACK();
}

unsigned int UVQtHexdumpData::iterator_impl::scrollbarPositionToOffset(unsigned int offset)
{
	return offset * m_dataImpl->m_bytesPerRow;
}

int UVQtHexdumpData::iterator_impl::scrollbarPositionDeltaToOffsetDelta(int offset)
{
	return offset * m_dataImpl->m_bytesPerRow;
}

UVQtDynamicTextData::iterator_impl *UVQtHexdumpData::iterator_impl::copy()
{
	UVQtHexdumpData::iterator_impl *ret = new UVQtHexdumpData::iterator_impl();
	*ret = *this;
	//printf("copy with %s\n", toString().c_str());
	return ret;
}

char UVQtHexdumpData::iterator_impl::read(unsigned int offset)
{
//printf("reading offset %d on 0x%08X\n", offset, (int)m_dataImpl->m_data);
	char c = (char)m_dataImpl->m_data->read(offset);
//printf("done\n");
//fflush(stdout);
return c;
}

unsigned int UVQtHexdumpData::iterator_impl::size()
{
	return m_dataImpl->m_data->size();
}

std::string UVQtHexdumpData::iterator_impl::toString()
{
	return UVDSprintf("m_offset=0x%08X", m_offset);
}

unsigned int UVQtHexdumpData::iterator_impl::hexdumpHalfRow(uint32_t start, std::string &ret)
{
	uint32_t col = 0;

	for( ; col < m_dataImpl->m_bytesPerSubRow && start + col < size(); ++col )
	{
		uint32_t index = start + col;
		//printf("index: %d, size: %d\n", index, size());
		uint8_t c = read(index);
printf("read %c (%d) @ 0x%04X\n", c, (int)c, index);
		
		(void)c;
		ret += UVDSprintf(" %02X ", (unsigned int)c);
	}

	//pad remaining
	while( col < m_dataImpl->m_bytesPerSubRow )
	{
		ret += "   ";
		++col;
	}

	//End pad
	ret += " ";

	return start + m_dataImpl->m_bytesPerSubRow;
}

unsigned int UVQtHexdumpData::iterator_impl::offset()
{
	return m_offset / m_dataImpl->m_bytesPerRow;
}

uv_err_t UVQtHexdumpData::iterator_impl::get(std::string &ret)
{
	/*
	[mcmaster@gespenst icd2prog-0.3.0]$ hexdump -C /bin/ls |head
	00000000  7f 45 4c 46 01 01 01 00  00 00 00 00 00 00 00 00  |.ELF............|
	00000010  02 00 03 00 01 00 00 00  f0 99 04 08 34 00 00 00  |............4...|
	00017380  00 00 00 00 01 00 00 00  00 00 00 00			    |............    |
	*/
//printf("get()\n");
//fflush(stdout);
//UVD_PRINT_STACK();	
	unsigned int pos = m_offset;
	std::string curLine;
	uint32_t row_start = pos;
	const char fillerChar = '.';

	curLine += UVDSprintf("%04X:  ", pos);

	for( unsigned int curRow = 0; curRow < m_dataImpl->m_bytesPerRow; curRow += m_dataImpl->m_bytesPerSubRow )
	{
		pos = hexdumpHalfRow(pos, curLine);
	}
	
	curLine += "|";

	uint32_t i = 0;
	//Char view
	for( i = row_start; i < row_start + m_dataImpl->m_bytesPerRow && i < size(); ++i )
	{
		char c = read(i);
		if( isprint(c) )
		{
			curLine += c;
		}
		else
		{
			curLine += '.';
		}

	} 
	for( ; i < row_start + m_dataImpl->m_bytesPerRow; ++i )
	{
		curLine += fillerChar;
	}

	curLine += "|";
printf("%s\n", curLine.c_str());
	ret = curLine;
	return UV_ERR_OK;
}

unsigned int UVQtHexdumpData::iterator_impl::maxValidOffset()
{
	/*
	Say 16 bytes per row
	0x0000...0x00FF -> 0x0000
	0x0010...0x01FF -> 0x0100
	*/
	if( size() == 0 )
	{
		return 0;
	}
	unsigned int maxIndex = size() - 1;
	printf("size: %d\n", size());
	return maxIndex - maxIndex % m_dataImpl->m_bytesPerRow;
}

uv_err_t UVQtHexdumpData::iterator_impl::previous()
{
	if( m_offset < m_dataImpl->m_bytesPerRow )
	{
		m_offset = 0;
	}
	else
	{
		m_offset -= m_dataImpl->m_bytesPerRow;
	}

	return UV_ERR_OK;
}
	
uv_err_t UVQtHexdumpData::iterator_impl::next()
{
	/*
	Say 256 bytes
	
	0000: 
	0010:
	...
	00F0:
	
	m_offset >= 240 => 256 - 16
	*/
	//Advancing to end()?
	if( m_offset >= size() - m_dataImpl->m_bytesPerRow )
	{
printf("next overflow, m_offset: %d, size: %d, bytes per row: %d\n", m_offset, size(), m_dataImpl->m_bytesPerRow);
		m_offset = size();
	}
	else
	{
		m_offset += m_dataImpl->m_bytesPerRow;
	}

	return UV_ERR_OK;
}

uv_err_t UVQtHexdumpData::iterator_impl::changePositionByLineDelta(int delta)
{
printf("**got a delta: %d\n", delta);
	int offsetDelta = scrollbarPositionDeltaToOffsetDelta(delta);
	if( delta > 0 )
	{
		m_offset += offsetDelta;
		
		printf("%d\n", maxValidOffset());
		if( m_offset > maxValidOffset() )
		{
			printf("overflow\n");
			m_offset = maxValidOffset();
		}
	}
	else
	{
		if( (unsigned)-offsetDelta > m_offset )
		{
			printf("predicted underflow\n");
			m_offset = 0;
		}
		else
		{
			m_offset += offsetDelta;
		}
	}
	printf("end offset: %d\n", m_offset);
	return UV_ERR_OK;
}

uv_err_t UVQtHexdumpData::iterator_impl::changePositionToLine(unsigned int offset, unsigned int index)
{
	//Index is ignored
	m_offset = scrollbarPositionToOffset(offset);
	return UV_ERR_OK;
}

int UVQtHexdumpData::iterator_impl::compare(const UVQtDynamicTextData::iterator_impl *otherIn)
{
	//Should be of this type
	const iterator_impl *other = static_cast<const iterator_impl *>(otherIn);
	
	printf("compare %d to %d\n", m_offset, other->m_offset);
	return m_offset - other->m_offset;
}

