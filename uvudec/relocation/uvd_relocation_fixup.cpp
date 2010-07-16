/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd_relocation.h"
#include "uvd_binary_symbol.h"

/*
UVDRelocationFixup
*/

UVDRelocationFixup::UVDRelocationFixup()
{
	m_symbol = NULL;
	m_offset = 0;
	//Assume one byte by default
	m_sizeBits = 8;
}

UVDRelocationFixup::UVDRelocationFixup(UVDRelocatableElement *symbol, uint32_t offset, uint32_t sizeBytes)
{
	m_symbol = symbol;
	m_offset = offset;
	m_sizeBits = sizeBytes * 8;
}

UVDRelocationFixup::~UVDRelocationFixup()
{
}

uv_err_t UVDRelocationFixup::applyPatch(UVDData *data)
{
	return UV_DEBUG(applyPatchCore(data, false));
}

uv_err_t UVDRelocationFixup::applyPatchCore(UVDData *data, bool useDefaultValue)
{
	const char *dynamicValue = NULL;
	uint32_t defaultValue = RELOCATION_DEFAULT_VALUE;

	//XXX: endianness, size issues
	if( useDefaultValue )
	{
		dynamicValue = (const char *)&defaultValue;
	}
	else
	{
		uv_assert_ret(m_symbol);
		uv_assert_err_ret(m_symbol->getDynamicValue(&dynamicValue));
	}
	
	uint32_t sizeBytes = getSizeBytes();
	uv_assert_err_ret(data->writeData(m_offset, dynamicValue, sizeBytes));
	
	return UV_ERR_OK;
}

uv_err_t UVDRelocationFixup::getUnknownSymbolRelocationFixup(UVDRelocationFixup **fixupOut, uint32_t offset, uint32_t sizeBytes)
{
	/*
	Will have to be cooridanted later
	Shouldn't we be able to pass in the symbol target? 
	*/
	return UV_DEBUG(getSymbolRelocationFixup(fixupOut, NULL, offset, sizeBytes));
}

uv_err_t UVDRelocationFixup::getSymbolRelocationFixup(UVDRelocationFixup **fixupOut, UVDBinarySymbol *binarySymbol, uint32_t offsetBytes, uint32_t sizeBytes)
{
	return UV_DEBUG(UVDRelocationFixup::getSymbolRelocationFixupByBits(fixupOut, binarySymbol, offsetBytes, sizeBytes * 8));
}

uv_err_t UVDRelocationFixup::getSymbolRelocationFixupByBits(UVDRelocationFixup **fixupOut, UVDBinarySymbol *binarySymbol, uint32_t offsetBytes, uint32_t sizeBits)
{
	/*
	at offset and size apply the the address at symbol
	*/

	UVDRelocationFixup *fixup = NULL;
	UVDRelocatableElement *element = NULL;
	
	element = new UVDBinarySymbolElement(binarySymbol);
	uv_assert_ret(element);
	
	//FIXME: debug check, most relocations don't exceed 32 bits, remove later
	uv_assert_ret(sizeBits <= 32);
	
	fixup = new UVDRelocationFixup(element, offsetBytes, 0);
	uv_assert_ret(fixup);
	fixup->setSizeBits(sizeBits);
	
	uv_assert_ret(fixupOut);
	*fixupOut = fixup;
	
	return UV_ERR_OK;
}

uint32_t UVDRelocationFixup::getSizeBits()
{
	//m_size is in bytes
	return m_sizeBits;
}

uint32_t UVDRelocationFixup::getSizeBytes()
{
	//m_size is already in bytes
	return m_sizeBits / 8;
}

uv_err_t UVDRelocationFixup::getSizeBytes(uint32_t *size)
{
	//m_size is already in bytes
	uv_assert_ret(size);
	*size = m_sizeBits / 8;
	return UV_ERR_OK;
}

uv_err_t UVDRelocationFixup::setSizeBits(uint32_t bits)
{
	m_sizeBits = bits;
	return UV_ERR_OK;
}

uv_err_t UVDRelocationFixup::setSizeBytes(uint32_t bytes)
{
	m_sizeBits = bytes * 8;
	return UV_ERR_OK;
}

uv_err_t UVDRelocationFixup::setOffset(uint32_t offset)
{
	m_offset = offset;
	return UV_ERR_OK;
}

uv_err_t UVDRelocationFixup::getOffset(uint32_t *offset)
{
	uv_assert_ret(offset);
	*offset = m_offset;
	return UV_ERR_OK;
}
