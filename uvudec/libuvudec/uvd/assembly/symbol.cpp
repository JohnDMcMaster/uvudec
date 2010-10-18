/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/assembly/symbol.h"
#include "uvd/assembly/function.h"
#include "uvd/util/util.h"
#include "uvd/core/runtime.h"
#include "uvd/core/uvd.h"
#include <stdio.h>
#include <string.h>

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
	deinit();
}

uv_err_t UVDBinarySymbol::init()
{
	m_relocatableData = new UVDRelocatableData();
	uv_assert_ret(m_relocatableData);

	return UV_ERR_OK;
}

uv_err_t UVDBinarySymbol::deinit()
{
	//FIXME: should probably move to storing these as two instances in the long run
	//If we shared the data, don't double free it
	if( m_relocatableData )
	{
		UVDData *relocatableDataData = NULL;
		uv_assert_err_ret(m_relocatableData->getRelocatableData(&relocatableDataData));

		if( relocatableDataData && relocatableDataData == m_data )
		{
			printf_deprecated("m_data in m_relocatableData must be unique, will cause crashes in future releases\n");
			m_data = NULL;
		}
		//***Not causing mem issues, still present when removed
		delete m_relocatableData;
		m_relocatableData = NULL;
	}

	//This should be a mapped type and not be the actual data
	//Its deletion should not cause issues
	delete m_data;
	m_data = NULL;

	for( std::set<UVDRelocationFixup *>::iterator iter = m_symbolUsageLocations.begin(); iter != m_symbolUsageLocations.end(); ++iter )
	{
		delete *iter;
	}
	m_symbolUsageLocations.clear();

	return UV_ERR_OK;
}

void UVDBinarySymbol::setSymbolName(const std::string &name)
{
	m_symbolName = name;
	if( m_symbolNames.find(name) == m_symbolNames.end() )
	{
		m_symbolNames.insert(name);
	}
}

void UVDBinarySymbol::addSymbolName(const std::string &name)
{
	if( m_symbolName.empty() )
	{
		m_symbolName = name;
	}
	if( m_symbolNames.find(name) == m_symbolNames.end() )
	{
		m_symbolNames.insert(name);
	}
}

uv_err_t UVDBinarySymbol::getSymbolName(std::string &name)
{
	name = m_symbolName;
	return UV_ERR_OK;
}

uv_err_t UVDBinarySymbol::getSymbolNames(std::set<std::string> &names)
{
	names = m_symbolNames;
	return UV_ERR_OK;
}

uv_err_t UVDBinarySymbol::addRelativeRelocation(uint32_t relocatableDataOffset, uint32_t relocatableDataSize)
{
	UVDRelocationFixup *fixup = NULL;

	//static uv_err_t getSymbolRelocationFixup(UVDRelocationFixup **fixupOut, UVDBinarySymbol *symbol, uint32_t offset, uint32_t size);
	//We do not know the symbol it is assicated with yet, these will be collected and fixed in collectRelocations()
	uv_assert_err_ret(UVDRelocationFixup::getUnknownSymbolRelocationFixup(&fixup, relocatableDataOffset, relocatableDataSize));
	uv_assert_ret(fixup);
	m_relocatableData->addFixup(fixup);

	return UV_ERR_OK;
}

uv_err_t UVDBinarySymbol::addAbsoluteRelocation(uint32_t relocatableDataOffset, uint32_t relocatableDataSize)
{
	uint32_t symbolStart = 0;
	uint32_t relativeOffset = 0;

	uv_assert_err_ret(getSymbolAddress(&symbolStart));

	if( relocatableDataOffset < symbolStart )
	{
		printf_error("relocatableDataOffset(0x%.8X) < symbolStart(0x%.8X)\n", relocatableDataOffset, symbolStart);
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	relativeOffset = relocatableDataOffset - symbolStart;

	return UV_DEBUG(addRelativeRelocation(relativeOffset, relocatableDataSize));
}

uv_err_t UVDBinarySymbol::addSymbolUse(uint32_t relocatableDataOffsetBytes, uint32_t relocatableDataSizeBytes)
{
	return UV_DEBUG(addSymbolUseByBits(relocatableDataOffsetBytes, relocatableDataSizeBytes * 8));
}

uv_err_t UVDBinarySymbol::addSymbolUseByBits(uint32_t relocatableDataOffsetBytes, uint32_t relocatableDataSizeBits)
{
	UVDRelocationFixup *fixup = NULL;

	//static uv_err_t getSymbolRelocationFixup(UVDRelocationFixup **fixupOut, UVDBinarySymbol *symbol, uint32_t offset, uint32_t size);
	//We do not know the symbol it is assicated with yet, these will be collected and fixed in collectRelocations()
	//uv_assert_err_ret(UVDRelocationFixup::getUnknownSymbolRelocationFixup(&fixup, relocatableDataOffset, relocatableDataSize));
	//CHECKME: this seems like it was actually obviously this symbol, not unknown
	uv_assert_err_ret(UVDRelocationFixup::getSymbolRelocationFixupByBits(&fixup, this, relocatableDataOffsetBytes, relocatableDataSizeBits));

	uv_assert_ret(fixup);
	//Asserts added to track down issue early
	uv_assert_ret(fixup->m_symbol);
	//uv_assert_ret(!fixup->m_symbol->getName().empty());
	m_symbolUsageLocations.insert(fixup);

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

uv_err_t UVDBinarySymbol::getSymbolSize(uint32_t *symbolSize)
{
	//return UV_DEBUG(m_symbolSize.getDynamicValue(symbolSize));
	uv_assert_ret(m_data);
	uv_assert_err_ret(m_data->size(symbolSize));
	return UV_ERR_OK;
}

/*
uv_err_t UVDBinarySymbol::setSymbolSize(uint32_t symbolSize)
{
	m_symbolSize.setDynamicValue(symbolSize);
	return UV_ERR_OK;
}
*/

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
	/*
	From UVDBinarySymbol:

	Must find the relocations contained in the input symbol:
	std::set<UVDRelocationFixup *> m_symbolUsageLocations;

	And move them to the set of relocations in the this symbol:
	UVDRelocatableData *m_relocatableData;
	*/
	uint32_t symbolStart = 0;
	uint32_t symbolEnd = 0;
	uint32_t symbolSize = 0;

	uv_assert_ret(m_relocatableData);
	uv_assert_ret(otherSymbol);

	uv_assert_err_ret(getSymbolAddress(&symbolStart));
	uv_assert_err_ret(getSymbolSize(&symbolSize));
	symbolEnd = symbolStart + symbolSize;

	uv_assert_ret(m_relocatableData);

	//Loop through all of the locations otherSymbol was used, seeing if any of them were inside of the this symbol
	for( std::set<UVDRelocationFixup *>::iterator iter = otherSymbol->m_symbolUsageLocations.begin();
			iter != otherSymbol->m_symbolUsageLocations.end(); ++iter )
	{
		UVDRelocationFixup *curFixup = *iter;
		uint32_t curFixupStart = 0;
		uint32_t curFixupEnd = 0;

		uv_assert_ret(curFixup);
		curFixupStart = curFixup->m_offset;
		curFixupEnd = curFixup->m_offset + curFixup->getSizeBytes();

		uv_assert_ret(curFixup->m_symbol);
		//It has instead been decided this will be solved by setting all relevent symbols after this analysis pass
		//This will become an issue later because ELF writting uses the relocations directly
		//uv_assert_ret(!curFixup->m_symbol->getName().empty());

		//Does the relocation apply within this symbol?
		if( curFixupStart >= symbolStart && curFixupEnd <= symbolEnd )
		{
			//We must subtract out the offset of this symbol
			UVDRelocationFixup *scaledFixup = NULL;

			scaledFixup = new UVDRelocationFixup();
			uv_assert_ret(scaledFixup);
			//Copy old
			*scaledFixup = *curFixup;
			//Do the scale
			scaledFixup->m_offset -= symbolStart;
			m_relocatableData->addFixup(scaledFixup);
		}
	}

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

std::string UVDBinarySymbol::mangleFileToSymbolName(const std::string &in)
{
	/*
	There are a lot of cases not covered for now
	*/

	//Start by getting the basename
	//basename may modify string, make a copy
	std::string ret = uv_basename(in);
	
	//remove everything after . (assume extension)
	for( ;; )
	{
		std::string::size_type pos = ret.rfind('.');
		if( pos == std::string::npos )
		{
			break;
		}
		//Get substring
		ret = ret.substr(0, pos);
	}
	
	//TODO:
	//All other illegal stuff (ex: ' ') replace with _
	//hmm actually is that techincally illegal?  Its just not possible in C
	
	return ret;
}

/*
UVDAnalyzedBinarySymbol
*/

UVDAnalyzedBinarySymbol::UVDAnalyzedBinarySymbol()
{
}

UVDAnalyzedBinarySymbol::~UVDAnalyzedBinarySymbol()
{
}

uv_err_t UVDAnalyzedBinarySymbol::getBestUVDBinarySymbol(UVDBinarySymbol **symbolOut)
{
	/*
	CHECKME: review this code, it does weird copy stuff
	*/
	UVDBinarySymbol *symbol = NULL;

	//Highest priority to funcs due to dual use on labels which are less important
	//Really though, labels are not used for now
	if( !m_functionHits.empty()
			&& m_functionHits.size() >= m_labelHits.size()
			&& m_functionHits.size() >= m_variableHits.size() )
	{
		symbol = new UVDBinaryFunctionInstance();
		uv_assert_ret(symbol);

		*symbol = *(UVDBinarySymbol *)this;
	}
	else if( !m_labelHits.empty()
			&& m_labelHits.size() >= m_variableHits.size() )
	{
		symbol = new UVDLabelBinarySymbol();
		uv_assert_ret(symbol);

		*symbol = *(UVDBinarySymbol *)this;
	}
	else if( !m_labelHits.empty() )
	{
		symbol = new UVDVariableBinarySymbol();
		uv_assert_ret(symbol);

		*symbol = *(UVDBinarySymbol *)this;
	}
	else
	{
		return UV_DEBUG(UV_ERR_GENERAL);
	}

	uv_assert_ret(symbolOut);
	*symbolOut = symbol;

	return UV_ERR_OK;
}

void UVDAnalyzedBinarySymbol::registerLabelUsage(uint32_t usageAddress)
{
	m_labelHits.push_back(usageAddress);
}

void UVDAnalyzedBinarySymbol::registerFunctionUsage(uint32_t usageAddress)
{
	m_functionHits.push_back(usageAddress);
}

void UVDAnalyzedBinarySymbol::registerVariableUsage(uint32_t usageAddress)
{
	m_variableHits.push_back(usageAddress);
}

/*
UVDVariableBinarySymbol
*/

UVDVariableBinarySymbol::UVDVariableBinarySymbol()
{
}

UVDVariableBinarySymbol::~UVDVariableBinarySymbol()
{
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
	deinit();
}

uv_err_t UVDBinarySymbolManager::deinit()
{
	for( std::map<std::string, UVDBinarySymbol *>::iterator iter = m_symbols.begin(); iter != m_symbols.end(); ++iter )
	{
		delete (*iter).second;
	}
	m_symbols.clear();
	m_symbolsByAddress.clear();

	return UV_ERR_OK;
}

uv_err_t UVDBinarySymbolManager::findSymbol(const std::string &name, UVDBinarySymbol **symbolIn)
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

uv_err_t UVDBinarySymbolManager::findAnalyzedSymbolByAddress(uv_addr_t address, UVDAnalyzedBinarySymbol **symbolOut)
{
	UVDBinarySymbol *symbolRaw = NULL;
	UVDAnalyzedBinarySymbol *symbol = NULL;
	uv_err_t rc = UV_ERR_GENERAL;

	//Don't print for errors
	rc = findSymbolByAddress(address, &symbolRaw);
	if( UV_FAILED(rc) )
	{
		return rc;
	}

	symbol = dynamic_cast<UVDAnalyzedBinarySymbol *>(symbolRaw);
	uv_assert_ret(symbol);

	uv_assert_ret(symbolOut);
	*symbolOut = symbol;

	return UV_ERR_OK;
}

uv_err_t UVDBinarySymbolManager::findAnalyzedSymbol(std::string &name, UVDAnalyzedBinarySymbol **symbolOut)
{
	UVDBinarySymbol *symbolRaw = NULL;
	UVDAnalyzedBinarySymbol *symbol = NULL;
	uv_err_t rc = UV_ERR_GENERAL;

	//Don't print for errors
	rc = findSymbol(name, &symbolRaw);
	if( UV_FAILED(rc) )
	{
		return rc;
	}

	symbol = dynamic_cast<UVDAnalyzedBinarySymbol *>(symbolRaw);
	uv_assert_ret(symbol);

	uv_assert_ret(symbolOut);
	*symbolOut = symbol;

	return UV_ERR_OK;
}

uv_err_t UVDBinarySymbolManager::addSymbol(UVDBinarySymbol *symbol)
{
	uint32_t symbolAddress = 0;
	std::set<std::string> names;

	uv_assert_ret(symbol);
	uv_assert_err_ret(symbol->getSymbolNames(names));
	uv_assert_ret(!names.empty());
	for( std::set<std::string>::iterator iter = names.begin(); iter != names.end(); ++iter )
	{
		std::string name = *iter;
		uv_assert_ret(!name.empty());
		m_symbols[name] = symbol;
	}

	uv_assert_err_ret(symbol->getSymbolAddress(&symbolAddress));
	m_symbolsByAddress[symbolAddress] = symbol;

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

	/*
	For each symbol, we must add all occurences contained in this symbol
	This is O(n**2) which could take a while, it probably can be made O(n) with some work
	*/
	for( std::map<std::string, UVDBinarySymbol *>::iterator iter = m_symbols.begin(); iter != m_symbols.end(); ++iter )
	{
		UVDBinarySymbol *binarySymbol = (*iter).second;
		uv_assert_ret(binarySymbol);
		/*
		uint32_t curSymbolAddress = 0;

		uv_assert_err_ret(binarySymbol->getSymbolAddress(&curSymbolAddress));

		//Is this the matching symbol we are looking for?
		if( functionAddress == curSymbolAddress )
		{
		*/
			uv_assert_err_ret(doCollectRelocations(function, binarySymbol));
		/*
			break;
		}
		*/
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

uv_err_t UVDBinarySymbolManager::addAbsoluteFunctionRelocation(uint32_t functionAddressBytes,
		uint32_t relocatableDataOffset, uint32_t relocatableDataSizeBytes)
{
	return UV_DEBUG(addAbsoluteFunctionRelocationByBits(functionAddressBytes,
			relocatableDataOffset, relocatableDataSizeBytes * 8));
}

uv_err_t UVDBinarySymbolManager::addAbsoluteFunctionRelocationByBits(uint32_t functionAddressBytes,
		uint32_t relocatableDataOffset, uint32_t relocatableDataSizeBits)
{
	UVDAnalyzedBinarySymbol *symbol = NULL;
	UVD *uvd = NULL;

	uv_assert_ret(m_analyzer);
	uvd = m_analyzer->m_uvd;
	uv_assert_ret(uvd);

	//Start by getting the symbol, if it exists
	//Query symbol
	if( UV_FAILED(findAnalyzedSymbolByAddress(functionAddressBytes, &symbol)) )
	{
		std::string symbolName;

		//Get name
		uv_assert_err_ret(analyzedSymbolName(functionAddressBytes, UVD__SYMBOL_TYPE__FUNCTION, symbolName));

		//Create it new then
		symbol = new UVDAnalyzedBinarySymbol();
		uv_assert_ret(symbol);
		uv_assert_err_ret(symbol->init());
		symbol->setSymbolAddress(functionAddressBytes);
		symbol->setSymbolName(symbolName);
		//Register it
		uv_assert_err_ret(addSymbol(symbol));
	}
	/*
	else
	{
		UVDBinaryFunctionInstance *functionSymbol = NULL;

		//printf("Previous symbol hit\n");
		functionSymbol = dynamic_cast<UVDBinaryFunctionInstance *>(symbol);
		if( !functionSymbol )
		{
			printf_warn("previous symbol was not a function symbol as now indicated (0x%.4X)\n", functionAddressBytes);
		}
		//uv_assert_ret(functionSymbol);
	}
	*/

	uv_assert_ret(symbol);
	symbol->registerLabelUsage(functionAddressBytes);
	uv_assert_err_ret(symbol->addSymbolUseByBits(relocatableDataOffset, relocatableDataSizeBits));

	return UV_ERR_OK;
}

uv_err_t UVDBinarySymbolManager::addAbsoluteLabelRelocation(uint32_t labelAddress,
		uint32_t relocatableDataOffset, uint32_t relocatableDataSizeBytes)
{
	return UV_DEBUG(addAbsoluteLabelRelocationByBits(labelAddress,
			relocatableDataOffset, relocatableDataSizeBytes * 8));
}

uv_err_t UVDBinarySymbolManager::addAbsoluteLabelRelocationByBits(uint32_t labelAddress,
		uint32_t relocatableDataOffset, uint32_t relocatableDataSizeBits)
{
	UVDAnalyzedBinarySymbol *symbol = NULL;
	UVD *uvd = NULL;
	std::string symbolName;

	uv_assert_ret(m_analyzer);
	uvd = m_analyzer->m_uvd;
	uv_assert_ret(uvd);

	//Start by getting the symbol, if it exists
	//Get name
	uv_assert_err_ret(analyzedSymbolName(labelAddress, UVD__SYMBOL_TYPE__LABEL, symbolName));
	//Query symbol
	if( UV_FAILED(findAnalyzedSymbol(symbolName, &symbol)) )
	{
		//Create it new then
		symbol = new UVDAnalyzedBinarySymbol();
		uv_assert_ret(symbol);
		uv_assert_err_ret(symbol->init());
		symbol->setSymbolAddress(labelAddress);
		symbol->setSymbolName(symbolName);
		//Register it
		uv_assert_err_ret(addSymbol(symbol));
	}
	/*
	else
	{
		UVDLabelBinarySymbol *labelSymbol = NULL;

		labelSymbol = dynamic_cast<UVDLabelBinarySymbol *>(symbol);
		//uv_assert_ret(labelSymbol);
		if( !labelSymbol )
		{
			printf_warn("previous symbol was not a label symbol as now indicated (0x%.4X)\n", labelAddress);
		}
	}
	*/

	uv_assert_ret(symbol);
	symbol->registerLabelUsage(labelAddress);
	uv_assert_err_ret(symbol->addSymbolUseByBits(relocatableDataOffset, relocatableDataSizeBits));

	return UV_ERR_OK;
}

uv_err_t UVDBinarySymbolManager::findSymbolByAddress(uv_addr_t address, UVDBinarySymbol **symbolOut)
{
	std::map<uint32_t, UVDBinarySymbol *>::iterator iter = m_symbolsByAddress.find(address);

	//This is used for checking for existence
	if( iter == m_symbolsByAddress.end() )
	{
		uv_assert_ret(symbolOut);
		*symbolOut = NULL;
		return UV_ERR_NOTFOUND;
	}
	uv_assert_ret(symbolOut);
	*symbolOut = (*iter).second;

	return UV_ERR_OK; 
}

uv_err_t UVDBinarySymbolManager::analyzedSymbolName(uint32_t symbolAddress, int symbolType, std::string &symbolName)
{
	std::string dataSource;
	
	uv_assert_ret(m_analyzer->m_uvd->m_runtime->m_object->m_data);
	dataSource = uv_basename(m_analyzer->m_uvd->m_runtime->m_object->m_data->getSource());
	uv_assert_err_ret(analyzedSymbolName(dataSource, symbolAddress, symbolType, symbolName));

	return UV_ERR_OK;
}

uv_err_t UVDBinarySymbolManager::analyzedSymbolName(std::string dataSource, uint32_t symbolAddress, int symbolType, std::string &symbolName)
{
	/*
	Might be nice to add on something about these being unknown symbol rather than known
	Ex:
	uvd_unknown__candela_rev_3__3242
	*/
	char buff[512];
	std::string typePrefix;
	std::string mangeledDataSource;
	UVDConfig *config = m_analyzer->m_uvd->m_config;

	uv_assert_err_ret(config->m_symbols.getSymbolTypeNamePrefix(symbolType, typePrefix));
	
	if( config->m_symbols.m_autoNameMangeledDataSource )
	{
		//file + address
		//Do mangling to make sure we don't have dots and such
		mangeledDataSource = UVDBinarySymbol::mangleFileToSymbolName(dataSource) + config->m_symbols.m_autoNameMangeledDataSourceDelim;
	}
	
	snprintf(buff, 512, "%s%s%s%.4X", config->m_symbols.m_autoNameUvudecPrefix.c_str(), mangeledDataSource.c_str(), typePrefix.c_str(), symbolAddress);
	
	symbolName = std::string(buff);

	return UV_ERR_OK;
}

/*
UVDBinarySymbolElement
*/

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

uv_err_t UVDBinarySymbolElement::getName(std::string &s)
{
	if( !m_binarySymbol )
	{
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	if( UV_FAILED(m_binarySymbol->getSymbolName(s)) )
	{
		return UV_DEBUG(UV_ERR_GENERAL);
	}

	return UV_ERR_OK;
}

uv_err_t UVDBinarySymbolElement::setName(const std::string &sName)
{
	if( !m_binarySymbol )
	{
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	m_binarySymbol->setSymbolName(sName);
	return UV_ERR_OK;
}


