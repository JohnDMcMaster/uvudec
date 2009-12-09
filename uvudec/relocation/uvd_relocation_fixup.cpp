/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#include "uvd_relocation.h"
#include "uvd_binary_symbol.h"


/*
UVDBinarySymbolElement
A relocation value based on a binary symbol
*/
class UVDBinarySymbolElement : public UVDRelocatableElement
{
public:
	UVDBinarySymbolElement();
	UVDBinarySymbolElement(UVDBinarySymbol *binarySymbol);
	~UVDBinarySymbolElement();

	virtual uv_err_t updateDynamicValue();
	
public:
	UVDBinarySymbol *m_binarySymbol;
};

UVDBinarySymbolElement::UVDBinarySymbolElement()
{
	m_binarySymbol = NULL;
}

UVDBinarySymbolElement::UVDBinarySymbolElement(UVDBinarySymbol *binarySymbol)
{
	m_binarySymbol = binarySymbol;
}

UVDBinarySymbolElement::~UVDBinarySymbolElement()
{
}

uv_err_t UVDBinarySymbolElement::updateDynamicValue()
{
	uint32_t symbolAddress = 0;

	uv_assert_ret(m_binarySymbol);
	uv_assert_err_ret(m_binarySymbol->getSymbolAddress(&symbolAddress));
	setDynamicValue(symbolAddress);

	return UV_ERR_OK;
}

/*
UVDRelocationFixup
*/

UVDRelocationFixup::UVDRelocationFixup()
{
	m_symbol = NULL;
	m_offset = 0;
	m_size = 1;
}

UVDRelocationFixup::UVDRelocationFixup(UVDRelocatableElement *symbol, unsigned int offset, unsigned int size)
{
	m_symbol = symbol;
	m_offset = offset;
	m_size = size;
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
	int defaultValue = RELOCATION_DEFAULT_VALUE;

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
	
	uv_assert_err_ret(data->writeData(m_offset, dynamicValue, m_size));
	
	return UV_ERR_OK;
}

uv_err_t UVDRelocationFixup::getUnknownSymbolRelocationFixup(UVDRelocationFixup **fixupOut, uint32_t offset, uint32_t size)
{
	return UV_DEBUG(getSymbolRelocationFixup(fixupOut, NULL, offset, size));
}

uv_err_t UVDRelocationFixup::getSymbolRelocationFixup(UVDRelocationFixup **fixupOut, UVDBinarySymbol *binarySymbol, uint32_t offset, uint32_t size)
{
	UVDRelocationFixup *fixup = NULL;
	UVDRelocatableElement *element = NULL;
	
	element = new UVDBinarySymbolElement(binarySymbol);
	uv_assert_ret(element);
	
	fixup = new UVDRelocationFixup(element, offset, size);
	uv_assert_ret(fixup);
	
	uv_assert_ret(fixupOut);
	*fixupOut = fixup;
	
	return UV_ERR_OK;
}
