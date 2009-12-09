/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#include "uvd_binary_symbol.h"
#include "uvd_binary_function.h"
#include "uvd.h"

/*
UVDBinarySymbol
*/

UVDBinarySymbol::UVDBinarySymbol()
{
	m_data = NULL;
	m_relocatableData = NULL;
}

UVDBinarySymbol::~UVDBinarySymbol()
{
}

uv_err_t UVDBinarySymbol::init()
{
	m_relocatableData = new UVDRelocatableData();
	uv_assert_ret(m_relocatableData);
	
	return UV_ERR_OK;
}

void UVDBinarySymbol::setSymbolName(const std::string &name)
{
	m_symbolName = name;
}

uv_err_t UVDBinarySymbol::getSymbolName(std::string &name)
{
	name = m_symbolName;
	return UV_ERR_OK;
}

uv_err_t UVDBinarySymbol::addRelocation(uint32_t relocatableDataOffset, uint32_t relocatableDataSize)
{
	UVDRelocationFixup *fixup = NULL;
	
	//static uv_err_t getSymbolRelocationFixup(UVDRelocationFixup **fixupOut, UVDBinarySymbol *symbol, uint32_t offset, uint32_t size);
	//We do not know the symbol it is assicated with yet, these will be collected and fixed in collectRelocations()
	uv_assert_err_ret(UVDRelocationFixup::getUnknownSymbolRelocationFixup(&fixup, relocatableDataOffset, relocatableDataSize));

	/*
	uv_assert_err_ret(UVDBinaryRelocation::getUVDBinaryRelocation(&relocation,
			relocatableDataOffset, relocatableDataSize));
	uv_assert_ret(fixup);
	
	uv_assert_ret(m_relocatableData);
	*/
	m_relocatableData->addFixup(fixup);

	/*
	UVDBinaryRelocation *relocation = NULL;
	
	uv_assert_err_ret(UVDBinaryRelocation::getUVDBinaryRelocation(&relocation,
			relocatableDataOffset, relocatableDataSize));
	uv_assert_ret(relocation);
	m_relocations.push_back(relocation);
	
	*/
	return UV_ERR_OK;
}

uv_err_t UVDBinarySymbol::getSymbolAddress(uint32_t *symbolAddress)
{
	return UV_DEBUG(m_symbolAddress.getDynamicValue(symbolAddress));
}

uv_err_t UVDBinarySymbol::setSymbolAddress(uint32_t symbolAddress)
{
	m_symbolAddress.setDynamicValue(symbolAddress);
	return UV_ERR_OK;
}

/*
uv_err_t UVDBinarySymbol::getOffset(uint32_t *offset)
{
	uv_assert_ret(offset);
	uv_assert_err_ret(m_symbolAddress.getDynamicValue(offset)).;
	return UV_ERR_OK;
}
*/

uv_err_t UVDBinarySymbol::addRelocations(const UVDBinarySymbol *otherSymbol)
{
	uv_assert_ret(m_relocatableData);	
	//We can simply copy the relocation list over
	uv_assert_ret(otherSymbol);
	uv_assert_ret(otherSymbol->m_relocatableData);
	//Copy for now..do add later if needed
	m_relocatableData->m_fixups = otherSymbol->m_relocatableData->m_fixups;

	/*
	FIXME: does this still need to be done?
	reason may actually just be more due to seperated analysis objects that need merging
	
	Register the fixups to being to this symbol
	This is due to during analysis not knowing where functions are until after the first pass is complete
	Ex: 
		say we find the first function call
		we have a function we called from and a function entry point
		But we haven't found enough function entry points to know our source function
		This code patches up that source function now that we know what it is
	*/

	return UV_ERR_OK;	
}

/*
UVDLabelBinarySymbol
*/

UVDLabelBinarySymbol::UVDLabelBinarySymbol()
{
}

UVDLabelBinarySymbol::~UVDLabelBinarySymbol()
{
}

/*
UVDBinarySymbolManager
*/

UVDBinarySymbolManager::UVDBinarySymbolManager()
{
	m_analyzer = NULL;
}

UVDBinarySymbolManager::~UVDBinarySymbolManager()
{
}

uv_err_t UVDBinarySymbolManager::findSymbol(std::string &name, UVDBinarySymbol **symbolIn)
{
	std::map<std::string, UVDBinarySymbol *>::iterator iter = m_symbols.find(name);
	UVDBinarySymbol *symbol = NULL;
	
	if( iter == m_symbols.end() )
	{
		uv_assert_ret(symbolIn);
		*symbolIn = NULL;
		return UV_ERR_NOTFOUND;
	}
	
	symbol = (*iter).second;
	uv_assert_ret(symbol);
	
	uv_assert_ret(symbolIn);
	*symbolIn = symbol;
	
	return UV_ERR_OK;
}

uv_err_t UVDBinarySymbolManager::addSymbol(UVDBinarySymbol *symbol)
{
	uv_assert_ret(symbol);
	uv_assert_ret(!symbol->m_symbolName.empty());
	m_symbols[symbol->m_symbolName] = symbol;
	return UV_ERR_OK;
}

uv_err_t UVDBinarySymbolManager::collectRelocations(UVDBinaryFunction *function)
{
	uint32_t functionAddress = 0;
	UVDBinaryFunctionInstance *functionInstance = NULL;
	
	uv_assert_ret(function);
	
	functionInstance = function->getFunctionInstance();
	uv_assert_ret(functionInstance);

	uv_assert_err_ret(functionInstance->getSymbolAddress(&functionAddress));

	for( std::map<std::string, UVDBinarySymbol *>::iterator iter = m_symbols.begin(); iter != m_symbols.end(); ++iter )
	{
		UVDBinarySymbol *binarySymbol = (*iter).second;
		uv_assert_ret(binarySymbol);
		uint32_t curSymbolAddress = 0;

		uv_assert_err_ret(binarySymbol->getSymbolAddress(&curSymbolAddress));
		
		//Is this the matching symbol we are looking for?
		if( functionAddress == curSymbolAddress )
		{
			uv_assert_err_ret(doCollectRelocations(function, binarySymbol));
			break;
		}
	}
	
	return UV_ERR_OK;
}

uv_err_t UVDBinarySymbolManager::doCollectRelocations(UVDBinaryFunction *function, UVDBinarySymbol *analysisSymbol)
{
	UVDBinaryFunctionInstance *functionInstance = NULL;
		
	functionInstance = function->getFunctionInstance();
	uv_assert_ret(functionInstance);

	uv_assert_err_ret(functionInstance->addRelocations(analysisSymbol));

	return UV_ERR_OK;	
}

uv_err_t UVDBinarySymbolManager::addAbsoluteFunctionRelocation(uint32_t functionAddress,
		uint32_t relocatableDataOffset, uint32_t relocatableDataSize)
{
	UVDBinarySymbol *symbol = NULL;
	UVDBinaryFunctionInstance *functionSymbol = NULL;
	UVD *uvd = NULL;
	std::string symbolName;

	uv_assert_ret(m_analyzer);
	uvd = m_analyzer->m_uvd;
	uv_assert_ret(uvd);

	//Start by getting the symbol, if it exists
	//Get name
	symbolName = uvd->analyzedSymbolName(functionAddress);
	//Query symbol
	if( UV_FAILED(findSymbol(symbolName, &symbol)) )
	{
		//Create it new then
		functionSymbol = new UVDBinaryFunctionInstance();
		uv_assert_ret(functionSymbol);
		uv_assert_err_ret(functionSymbol->init());
		functionSymbol->setSymbolAddress(functionAddress);
		functionSymbol->setSymbolName(symbolName);
		//Register it
		uv_assert_err_ret(addSymbol(functionSymbol));
	}
	else
	{
		functionSymbol = dynamic_cast<UVDBinaryFunctionInstance *>(symbol);
		//functionSymbol = (UVDBinaryFunctionInstance *)(symbol);
		uv_assert_ret(functionSymbol);
	}

	uv_assert_err_ret(functionSymbol->addRelocation(relocatableDataOffset, relocatableDataSize));
	
	return UV_ERR_OK;
}

uv_err_t UVDBinarySymbolManager::addAbsoluteLabelRelocation(uint32_t labelAddress,
		uint32_t relocatableDataOffset, uint32_t relocatableDataSize)
{
	UVDBinarySymbol *symbol = NULL;
	UVDLabelBinarySymbol *labelSymbol = NULL;
	UVD *uvd = NULL;
	std::string symbolName;

	uv_assert_ret(m_analyzer);
	uvd = m_analyzer->m_uvd;
	uv_assert_ret(uvd);

	//Start by getting the symbol, if it exists
	//Get name
	symbolName = uvd->analyzedSymbolName(labelAddress);
	//Query symbol
	if( UV_FAILED(findSymbol(symbolName, &symbol)) )
	{
		//Create it new then
		labelSymbol = new UVDLabelBinarySymbol();
		uv_assert_ret(labelSymbol);
		uv_assert_err_ret(labelSymbol->init());
		labelSymbol->setSymbolAddress(labelAddress);
		labelSymbol->setSymbolName(symbolName);
		//Register it
		uv_assert_err_ret(addSymbol(labelSymbol));
	}
	else
	{
		//labelSymbol = dynamic_cast<UVDLabelBinarySymbol *>(symbol);
		labelSymbol = (UVDLabelBinarySymbol *)(symbol);
		uv_assert_ret(labelSymbol);
	}

	uv_assert_err_ret(labelSymbol->addRelocation(relocatableDataOffset, relocatableDataSize));
	
	return UV_ERR_OK;
}
