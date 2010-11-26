/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/event/engine.h"
#include "uvd/core/uvd.h"
#include "uvd/core/analyzer.h"
#include "uvd/assembly/function.h"
#include "uvd/core/event.h"
#include "uvd/util/benchmark.h"
#include <algorithm>
#include <stdio.h>

UVDMemoryReference::UVDMemoryReference()
{
	m_types = UVD_MEMORY_REFERENCE_NONE;
	m_from = 0;
}

UVDAnalyzedMemoryRange::UVDAnalyzedMemoryRange()
{
}

UVDAnalyzedMemoryRange::UVDAnalyzedMemoryRange(unsigned int min_addr) 
		: UVDAddressRange(min_addr)
{
}

UVDAnalyzedMemoryRange::UVDAnalyzedMemoryRange(unsigned int min_addr, unsigned int max_addr, UVDAddressSpace *space) 
		: UVDAddressRange(min_addr, max_addr, space)
{
}

UVDAnalyzedMemoryRange::~UVDAnalyzedMemoryRange()
{
	deinit();
}

uv_err_t UVDAnalyzedMemoryRange::deinit()
{
	for( UVDAnalyzedMemoryRangeReferences::iterator iter = m_references.begin(); iter != m_references.end(); ++iter )
	{
		delete (*iter).second;
	}
	m_references.clear();
	
	return UV_ERR_OK;
}

uv_err_t UVDAnalyzedMemoryRange::insertReference(uint32_t from, uint32_t type)
{
	/*
	FIXME: this code has been superseded by some more advanced code
	Consider migrating the advanced code here and/or deleting this code
	*/

	UVDMemoryReference *reference = NULL;
	
	UV_ENTER();
	
	//Increase the calls count to address targetAddress from current address
	if( m_references.find(from) != m_references.end() )
	{
		//Same exact reference as before?
		//Could have new type
		reference = (*m_references.find(from)).second;
		uv_assert_ret(reference);
	}
	else
	{
		reference = new UVDMemoryReference();
		uv_assert_ret(reference);
		reference->m_from = from;
		m_references[from] = reference;
	}
	//Add the (new) type
	reference->m_types |= type;
	
	//Actually each address can only reference once so this is useless after all?
	//++m_referenceCount;
	return UV_ERR_OK;
}

uint32_t UVDAnalyzedMemoryRange::getReferenceCount()
{
	return m_references.size();
}

uint32_t UVDAnalyzedMemoryRange::getReferenceTypes()
{
	uint32_t ret = UVD_MEMORY_REFERENCE_NONE;
	for( UVDAnalyzedMemoryRangeReferences::iterator iter = m_references.begin(); iter != m_references.end(); ++iter )
	{
		//uint32_t key = (*iter).first;
		UVDMemoryReference *value = (*iter).second;
	
		if( !value )
		{
			printf_error("missing value\n");
			exit(1);
		}
		
		//Inherent any sub types
		ret |= value->m_types;
	}
	return ret;
}

uv_err_t UVDAnalyzedMemoryRange::getReferences(UVDAnalyzedMemoryRangeReferences &references, uint32_t type)
{
	for( UVDAnalyzedMemoryRangeReferences::iterator iter = m_references.begin(); iter != m_references.end(); ++iter )
	{
		uint32_t key = (*iter).first;
		UVDMemoryReference *value = (*iter).second;
	
		uv_assert_ret(value);
		
		if( value->m_types & type )
		{
			references[key] = value;
		}
	}
	return UV_ERR_OK;
}

UVDAnalyzedBlock::UVDAnalyzedBlock()
{
	m_code = NULL;
	m_addressSpace = NULL;
}

UVDAnalyzedBlock::~UVDAnalyzedBlock()
{
	deinit();
}

uv_err_t UVDAnalyzedBlock::deinit()
{
	delete m_code;
	m_code = NULL;
	
	for( std::vector<UVDAnalyzedBlock *>::iterator iter = m_blocks.begin(); iter != m_blocks.end(); ++iter )
	{
		delete *iter;
	}

	return UV_ERR_OK;
}

uv_err_t UVDAnalyzedBlock::getDataChunk(UVDDataChunk **dataChunkIn)
{
	uv_err_t rc = UV_ERR_GENERAL;
	UVDDataChunk *dataChunk = NULL;
	
	uv_assert(m_code);
	dataChunk = m_code->m_dataChunk;
	uv_assert(dataChunk);
	
	uv_assert(dataChunkIn);
	*dataChunkIn = dataChunk;

	rc = UV_ERR_OK;

error:
	return UV_DEBUG(rc);
}

uv_err_t UVDAnalyzedBlock::getMinAddress(uint32_t &address)
{
	uv_err_t rc = UV_ERR_GENERAL;
	UVDDataChunk *dataChunk = NULL;
	
	uv_assert_err(getDataChunk(&dataChunk));
	uv_assert(dataChunk);
	
	address = dataChunk->m_offset;

	rc = UV_ERR_OK;

error:
	return UV_DEBUG(rc);
}

uv_err_t UVDAnalyzedBlock::getMaxAddress(uint32_t &address)
{
	uv_err_t rc = UV_ERR_GENERAL;
	UVDDataChunk *dataChunk = NULL;
	
	uv_assert_err(getDataChunk(&dataChunk));
	uv_assert(dataChunk);
	
	address = dataChunk->m_offset + dataChunk->m_bufferSize;

	rc = UV_ERR_OK;

error:
	return UV_DEBUG(rc);
}

uv_err_t UVDAnalyzedBlock::getSize(uint32_t &address)
{
	uv_err_t rc = UV_ERR_GENERAL;
	UVDDataChunk *dataChunk = NULL;
	
	uv_assert_err(getDataChunk(&dataChunk));
	uv_assert(dataChunk);
	
	address = dataChunk->m_bufferSize;

	rc = UV_ERR_OK;

error:
	return UV_DEBUG(rc);
}

UVDAnalyzedCode::UVDAnalyzedCode()
{
	m_language = 0;
	m_dataChunk = NULL;
}

UVDAnalyzedCode::~UVDAnalyzedCode()
{
	deinit();
}

uv_err_t UVDAnalyzedCode::deinit()
{
	delete m_dataChunk;
	m_dataChunk = NULL;

	return UV_ERR_OK;
}

UVDAnalyzedFunction::UVDAnalyzedFunction()
{
	m_code = NULL;
	m_callingConvention = 0;
}

UVDAnalyzedFunction::~UVDAnalyzedFunction()
{
	deinit();
}

uv_err_t UVDAnalyzedFunction::deinit()
{
	delete m_code;
	m_code = NULL;
	
	return UV_ERR_OK;
}

UVDAnalyzer::UVDAnalyzer()
{
	m_block = NULL;
#if USING_PREVIOUS_ANALYSIS
	m_db = NULL;
#endif
	m_uvd = NULL;
	//m_symbolManager = NULL;
	m_symbolManager.m_analyzer = this;
}

UVDAnalyzer::~UVDAnalyzer()
{
	deinit();
}

uv_err_t UVDAnalyzer::init()
{
	return UV_ERR_OK;
}

uv_err_t UVDAnalyzer::deinit()
{
	delete m_block;
	m_block = NULL;

	//delete m_curDb;
	//m_curDb = NULL;
	
	for( std::set<UVDBinaryFunction *>::iterator iter = m_functions.begin(); iter != m_functions.end(); ++iter )
	{
		UVDBinaryFunction *binaryFunction = *iter;
		
		uv_assert_ret(binaryFunction);
		//This is only stored here since its from current analysis and not in a central DB
		//delete binaryFunction->m_shared;
		//binaryFunction->m_shared = NULL;
		
		delete binaryFunction;
	}
	m_functions.clear();

	for( UVDAnalyzedMemorySpace::iterator iter = m_referencedAddresses.begin(); iter != m_referencedAddresses.end(); ++iter )
	{
		delete (*iter).second;
	}
	m_referencedAddresses.clear();

	m_uvd = NULL;

	return UV_ERR_OK;
}

uv_err_t UVDAnalyzer::insertReference(uint32_t targetAddress, uint32_t from, uint32_t type)
{
	uv_err_t rc = UV_ERR_GENERAL;
	UVDAnalyzedMemoryRange *analyzedMemoryLocation = NULL;

	printf_debug("UVDAnalyzer: inserting reference to 0x%.8X from 0x%.8X of type %d\n", targetAddress, from, type);

	//Ensure analyzed location existance
	if( m_referencedAddresses.find(targetAddress) == m_referencedAddresses.end() )
	{
		analyzedMemoryLocation = new UVDAnalyzedMemoryRange();
		
		uv_assert(analyzedMemoryLocation);
		//These are somewhat fuzzy at this point, we just know it includes this
		analyzedMemoryLocation->m_min_addr = targetAddress;
		analyzedMemoryLocation->m_max_addr = targetAddress;
		m_referencedAddresses[targetAddress] = analyzedMemoryLocation;
	}
	else
	{
		analyzedMemoryLocation = m_referencedAddresses[targetAddress];
	}
	uv_assert(analyzedMemoryLocation);
	
	//Existance ensured, increase count and add types
	analyzedMemoryLocation->insertReference(from, type);
	
	printf_debug("Location now has type: %d\n", analyzedMemoryLocation->getReferenceTypes());

	rc = UV_ERR_OK;

error:
	return UV_DEBUG(rc);
}

uv_err_t UVDAnalyzer::insertCallReference(uint32_t targetAddress, uint32_t from)
{
	//To know there is a branch possible at source
	uv_assert_err_ret(insertReference(from, from, UVD_MEMORY_REFERENCE_CALL_SOURCE));
	//And to keep track of branch destination
	uv_assert_err_ret(insertReference(targetAddress, from, UVD_MEMORY_REFERENCE_CALL_DEST));
	return UV_ERR_OK;
}

uv_err_t UVDAnalyzer::insertJumpReference(uint32_t targetAddress, uint32_t from)
{
	//To know there is a call at source
	uv_assert_err_ret(insertReference(from, from, UVD_MEMORY_REFERENCE_JUMP_SOURCE));
	//And to keep track of functions themselves
	uv_assert_err_ret(insertReference(targetAddress, from, UVD_MEMORY_REFERENCE_JUMP_DEST));
	return UV_ERR_OK;
}

uv_err_t UVDAnalyzer::getAddresses(UVDAnalyzedMemorySpace &addresses, uint32_t type)
{
	addresses.clear();
	
	for( UVDAnalyzedMemorySpace::iterator iter = m_referencedAddresses.begin(); iter != m_referencedAddresses.end(); ++iter )
	{
		UVDAnalyzedMemoryRange *memoryLocation = NULL;
		uint32_t types = 0;
		
		memoryLocation = (*iter).second;
		uv_assert_ret(memoryLocation);
		
		types = memoryLocation->getReferenceTypes();
		//Memory location instance contains desired type?
		if( type == UVD_MEMORY_REFERENCE_NONE || types & type )
		{
			addresses[memoryLocation->m_min_addr] = memoryLocation;
		}
	}
	
	return UV_ERR_OK;
}

bool sortCheck(UVDAnalyzedMemoryRange *l, UVDAnalyzedMemoryRange *r)
{
	return UVDAddressRange::compareStatic(l, r) >= 0;
} 

void sort(UVDAnalyzedMemoryRanges &locations)
{
	std::sort(locations.begin(), locations.end(), sortCheck);
}

uv_err_t UVDAnalyzer::getAddresses(UVDAnalyzedMemoryRanges &addresses, uint32_t type)
{
	UVDAnalyzedMemorySpace space;
	addresses.clear();
	
	uv_assert_err_ret(getAddresses(space, type));
	
	for( UVDAnalyzedMemorySpace::iterator iter = space.begin(); iter != space.end(); ++iter )
	{
		addresses.push_back((*iter).second);
	}
	
	sort(addresses);
	
	printf_debug("Sorted addresses: %d\n", addresses.size());
	for( unsigned int i = 0; i < addresses.size(); ++i )
	{
		printf_debug("address[%d] = 0x%.8X, type: 0x%.4X\n", i, addresses[i]->m_min_addr, addresses[i]->getReferenceTypes());
	}	

	return UV_ERR_OK;
}

uv_err_t UVDAnalyzer::getCalledAddresses(UVDAnalyzedMemorySpace &calledAddresses)
{
	return UV_DEBUG(getAddresses(calledAddresses, UVD_MEMORY_REFERENCE_CALL_DEST));
}

uv_err_t UVDAnalyzer::getJumpedAddresses(UVDAnalyzedMemorySpace &jumpedAddresses)
{
	return UV_DEBUG(getAddresses(jumpedAddresses, UVD_MEMORY_REFERENCE_JUMP_DEST));
}

/*
uv_err_t UVDAnalyzer::rebuildDb()
{
	printf_debug_level(UVD_DEBUG_SUMMARY, "Rebuilding analyzer database\n");
	//Assumes all functions are already here and just need to be individually picked apart
	
	//Start clean
	uv_assert_ret(m_curDb);
	m_curDb->clear();

	//Rebuild database for each function
	//Since each UVDBinaryFunction had the UVDBinaryFunctionShared created with it,
	//we simply strip out the local analysis specific data
	for( std::vector<UVDBinaryFunction *>::iterator iter = m_functions.begin(); iter != m_functions.end(); ++iter )
	{
		UVDBinaryFunction *binaryFunction = *iter;
		//The metadata and such for this function
		UVDBinaryFunctionShared *binaryFunctionShared = NULL;
	
		uv_assert_ret(binaryFunction);
		//Get the shared section
		binaryFunctionShared = binaryFunction->m_shared;
		uv_assert_ret(binaryFunctionShared);
		//And register it to the DB
		uv_assert_err_ret(m_curDb->loadFunction(binaryFunctionShared));
	}

	return UV_ERR_OK;
}
*/

uv_err_t UVDAnalyzer::loadFunction(UVDBinaryFunction *function)
{
	uv_assert_ret(function);

	//Register it as a found function
	m_functions.insert(function);
	
	//Register the instance to our function analysis database
	//uv_assert_ret(m_curDb);
	//uv_assert_err_ret(m_curDb->loadFunction(function));

	//Tell the world
	UVDEventFunctionChanged functionChangedEvent;
	functionChangedEvent.m_function = function;
	functionChangedEvent.m_isDefined = true;
	uv_assert_err_ret(m_uvd->m_eventEngine->emitEvent(&functionChangedEvent));

	return UV_ERR_OK;
}

/*
uv_err_t UVDAnalyzer::functionSharedToFunction(UVDBinaryFunctionShared *functionShared, UVDBinaryFunction **functionOut)
{
	UVDBinaryFunctionInstance *functionInstance = NULL;
	
	uv_assert_ret(functionShared);
	//A newly analyzed function should only know of the one occurance
	uv_assert_ret(functionShared->m_representations.size() == 1);
	//Get that occurance
	functionInstance = functionShared->m_representations[0];
	
	return UV_DEBUG(functionInstanceToFunction(functionInstance, functionOut));
}

uv_err_t UVDAnalyzer::functionInstanceToFunction(UVDBinaryFunctionInstance *targetFunctionInstance, UVDBinaryFunction **functionOut)
{
	UVDBinaryFunction *functionToOutput = NULL;

	uv_assert_ret(targetFunctionInstance);
	for( std::set<UVDBinaryFunction *>::iterator iter = m_functions.begin();
			iter != m_functions.end(); ++iter )
	{
		UVDBinaryFunction *function = *iter;
		UVDBinaryFunctionShared *functionShared = NULL;
		UVDBinaryFunctionInstance *functionInstance = NULL;

		//Get the function instance generated by this raw function object
		uv_assert_ret(function);
		functionShared = function->m_shared;
		uv_assert_ret(functionShared->m_representations.size() == 1);
		functionInstance = functionShared->m_representations[0];
		uv_assert_ret(functionInstance);	
			
		//See if the associated function instance is the one we are looking for
		if( functionInstance == targetFunctionInstance )
		{
			functionToOutput = function;
			break;
		}
	}
	uv_assert_ret(functionToOutput);

	//We found a match, good to go
	uv_assert_ret(functionOut);
	*functionOut = functionToOutput;
	return UV_ERR_OK;
}
*/

uv_err_t UVDAnalyzer::mapSymbols()
{
	//For each function, find its associated symbols
	//This algorithm can be made linear for some extra interaction
	for( std::set<UVDBinaryFunction *>::iterator iter = m_functions.begin(); iter != m_functions.end(); ++iter )
	{
		UVDBinaryFunction *function = *iter;
		uv_assert_ret(function);

		//Get all the relocations for this particular function and register the fixups
		uv_assert_err_ret(m_symbolManager.collectRelocations(function));
	}

	//Assign default symbol names
	uv_assert_err_ret(assignDefaultSymbolNames());
		
	//Use object file database to identify previously known functions
	identifyKnownFunctions();
	
	return UV_ERR_OK;
}

uv_err_t UVDAnalyzer::assignDefaultSymbolNames()
{
	/*
	Although the UVDBinarySymbol objects already have names,
	we must link them to the UVDBinarySymbolElement UVDRelocatableElement
	so that they can get the core symbol attributes as needed
	
	This is done by doing address lookups
	*/
	
	//printf("Default sym names\n");

	/*
	Loop through all functions
	*/
	//printf("Functions: %d\n", m_functions.size());
	for( std::set<UVDBinaryFunction *>::iterator iterFunctions = m_functions.begin();
			iterFunctions != m_functions.end(); ++iterFunctions )
	{
		UVDBinaryFunction *function = *iterFunctions;
		UVDBinaryFunction *functionInstance = NULL;
		
		uv_assert_ret(function);			
		functionInstance = function;
		uv_assert_ret(functionInstance);
		/*
		{
			std::string name;
			uint32_t size = 0;
			uv_assert_err_ret(functionInstance->getSymbolName(name));
			uv_assert_err_ret(functionInstance->getSymbolSize(&size));
			printf("Function: %s, size: 0x%.4X\n", name.c_str(), size);
		}
		*/
		/*
		For all of the places we have to patch this symbol, 
		make sure each of those symbols we must resolve will have a name associated with them
		*/
		for( std::set<UVDRelocationFixup *>::iterator iter = functionInstance->m_relocatableData->m_fixups.begin();
				iter != functionInstance->m_relocatableData->m_fixups.end(); ++iter )
		{
			UVDRelocationFixup *fixup = *iter;
			UVDRelocatableElement *relocatableElement = NULL;
			UVDBinarySymbolElement *binarySymbolElement = NULL;
			UVDBinarySymbol *relocationsSymbol = NULL;
			uint32_t symbolAddress = 0;
			
			uv_assert_ret(fixup);		
			relocatableElement = fixup->m_symbol;
			uv_assert_ret(relocatableElement);
			
			//Relocations from analysis should be of this type
			binarySymbolElement = dynamic_cast<UVDBinarySymbolElement *>(relocatableElement);
			uv_assert_ret(binarySymbolElement);
			
			//What was the recorded address of this symbol?
			uv_assert_err_ret(binarySymbolElement->getDynamicValue(&symbolAddress));
			//Fetch the associated UVDBinarySYmbol
			uv_assert_err_ret(m_symbolManager.findSymbolByAddress(symbolAddress, &relocationsSymbol));
			uv_assert_ret(relocationsSymbol);
			{
				std::string s;
				//uint32_t size = 0;
				uv_assert_err_ret(relocationsSymbol->getSymbolName(s));
				//uv_assert_err_ret(relocationsSymbol->getSymbolSize(size));
				uv_assert_ret(!s.empty());
				//printf("\tfixup %s @ 0x%.4X\n", s.c_str(), fixup->m_offset);
			}
			//early sanity check
			{
				uint32_t size = 0;
				uv_assert_err_ret(functionInstance->getSymbolSize(&size));
				uv_assert_ret(fixup->m_offset < size);
			}
			//And link them
			binarySymbolElement->m_binarySymbol = relocationsSymbol;
		}

		/*
		These are all of the locations this symbol is used
		This is not really used here it seems
		*/
		for( std::set<UVDRelocationFixup *>::iterator iterUsage = functionInstance->m_symbolUsageLocations.begin();
				iterUsage != functionInstance->m_symbolUsageLocations.end(); ++iterUsage )
		{
			UVDRelocationFixup *fixup = *iterUsage;
			UVDRelocatableElement *relocatableElement = NULL;
			UVDBinarySymbolElement *binarySymbolElement = NULL;
			UVDBinarySymbol *relocationsSymbol = NULL;
			uint32_t symbolAddress = 0;

			uv_assert_ret(fixup);
			relocatableElement = fixup->m_symbol;
			uv_assert_ret(relocatableElement);
			
			//Relocations from analysis should be of this type
			binarySymbolElement = dynamic_cast<UVDBinarySymbolElement *>(relocatableElement);
			uv_assert_ret(binarySymbolElement);
			
			//What was the recorded address of this symbol?
			uv_assert_err_ret(binarySymbolElement->getDynamicValue(&symbolAddress));
			//Fetch the associated UVDBinarySYmbol
			uv_assert_err_ret(m_symbolManager.findSymbolByAddress(symbolAddress, &relocationsSymbol));
			uv_assert_ret(relocationsSymbol);
			//And link them
			binarySymbolElement->m_binarySymbol = relocationsSymbol;
		}
	}

	return UV_ERR_OK;
}
	
uv_err_t UVDAnalyzer::identifyKnownFunctions()
{
	//TODO: compare symbols with database
	return UV_ERR_OK;
}


/*
Saved in case they might be useful as ref or other
Other method of interest might have been save function to binary,
but it was just simple fetch the UVDData and save it 

uv_err_t UVDAnalysisDBArchive::shouldSaveFunction(UVDBinaryFunction *functionShared)
{
	UVDConfig *config = m_analyzer->m_uvd->m_config;
	UVDBinaryFunction *function = functionShared;
	uint32_t iFunctionAddress = 0;
		
	uv_assert_ret(config);	
	if( config->m_analysisOutputAddresses.empty() )
	{
		return UV_ERR_OK;
	}
	
	uv_assert_ret(function);
	iFunctionAddress = function->m_offset;

	if( config->m_analysisOutputAddresses.find(iFunctionAddress) != config->m_analysisOutputAddresses.end() )
	{
		return UV_ERR_OK;
	}
	else
	{
		return UV_ERR_GENERAL;
	}
}

uv_err_t UVDAnalysisDBArchive::queryFunctionByBinary(UVDDataChunk *dataChunk, std::vector<UVDBinaryFunction *> &funcs, bool bClear)
{
	if( bClear )
	{
		funcs.clear();
	}
	for( std::vector<UVDBinaryFunction *>::size_type i = 0; i < m_functions.size(); ++i )
	{
		UVDBinaryFunction *function = m_functions[i];
		
		uv_assert_ret(function);
		
		UVDBinaryFunction *functionShared = function;
		uv_assert_ret(functionShared);
		
		//Match?
		if( dataChunk == functionShared->getData() )
		{
			//Yipee!
			funcs.push_back(function);
			//We already have a match for this function
			break;
		}
	}
	
	return UV_ERR_OK;
}
*/

