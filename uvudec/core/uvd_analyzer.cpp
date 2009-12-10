/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#include "uvd.h"
#include "uvd_analysis_db.h"
#include "uvd_analyzer.h"
#include "uvd_binary_function.h"
#include <algorithm>

UVDMemoryReference::UVDMemoryReference()
{
	m_types = UVD_MEMORY_REFERENCE_NONE;
	m_from = 0;
}

UVDAnalyzedMemoryLocation::UVDAnalyzedMemoryLocation()
{
}

UVDAnalyzedMemoryLocation::UVDAnalyzedMemoryLocation(unsigned int min_addr) 
		: UVDMemoryLocation(min_addr)
{
}

UVDAnalyzedMemoryLocation::UVDAnalyzedMemoryLocation(unsigned int min_addr, unsigned int max_addr, UVDMemoryShared *space) 
		: UVDMemoryLocation(min_addr, max_addr, space)
{
}

uv_err_t UVDAnalyzedMemoryLocation::insertReference(uint32_t from, uint32_t type)
{
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

uint32_t UVDAnalyzedMemoryLocation::getReferenceCount()
{
	return m_references.size();
}

uint32_t UVDAnalyzedMemoryLocation::getReferenceTypes()
{
	uint32_t ret = UVD_MEMORY_REFERENCE_NONE;
	for( UVDAnalyzedMemoryLocationReferences::iterator iter = m_references.begin(); iter != m_references.end(); ++iter )
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

uv_err_t UVDAnalyzedMemoryLocation::getReferences(UVDAnalyzedMemoryLocationReferences &references, uint32_t type)
{
	for( UVDAnalyzedMemoryLocationReferences::iterator iter = m_references.begin(); iter != m_references.end(); ++iter )
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
}

uv_err_t UVDAnalyzedBlock::getDataChunk(UVDDataChunk **dataChunkIn)
{
	uv_err_t rc = UV_ERR_GENERAL;
	UVDAnalyzedCodeShared *codeShared = NULL;
	UVDDataChunk *dataChunk = NULL;
	
	uv_assert(m_code);
	
	codeShared = m_code->m_shared;
	uv_assert(codeShared);
	
	dataChunk = codeShared->m_dataChunk;
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

UVDAnalyzedCodeShared::UVDAnalyzedCodeShared()
{
	m_language = 0;
	m_dataChunk = NULL;
}

UVDAnalyzedCode::UVDAnalyzedCode()
{
	m_shared = NULL;
}

UVDAnalyzedFunction::UVDAnalyzedFunction()
{
	m_code = NULL;
	m_shared = NULL;
}

UVDAnalyzedFunctionShared::UVDAnalyzedFunctionShared()
{
	m_callingConvention = 0;
	m_code = NULL;
}

UVDAnalyzer::UVDAnalyzer()
{
	m_block = NULL;
	m_db = NULL;
	m_uvd = NULL;
	//m_symbolManager = NULL;
	m_symbolManager.m_analyzer = this;
}

uv_err_t UVDAnalyzer::init()
{
	m_db = new UVDAnalysisDBConcentrator();
	uv_assert_ret(m_db);
	uv_assert_err_ret(m_db->init());

	//Create a blank archive to fill from current program
	m_curDb = new UVDAnalysisDBArchive();
	uv_assert_ret(m_curDb);
	//uv_assert_err_ret(m_curDb->init());
	
	//Should add to DB concentrator?  Prob not as it might create loops or other anomalies

	return UV_ERR_OK;
}

void UVDAnalyzer::updateCache(uint32_t address, const UVDVariableMap &analysisResult)
{
	printf_debug("Caching analysis of address %d\n", address);
	m_analysisCache[address] = analysisResult;
}

uv_err_t UVDAnalyzer::readCache(uint32_t address, UVDVariableMap &analysisResult)
{
	if( m_analysisCache.find(address) == m_analysisCache.end() )
	{
		return UV_ERR_GENERAL;
	}
	analysisResult = m_analysisCache[address];
	return UV_ERR_OK;
}

uv_err_t UVDAnalyzer::insertReference(uint32_t targetAddress, uint32_t from, uint32_t type)
{
	uv_err_t rc = UV_ERR_GENERAL;
	UVDAnalyzedMemoryLocation *analyzedMemoryLocation = NULL;

	printf_debug("UVDAnalyzer: inserting reference to 0x%.8X from 0x%.8X of type %d\n", targetAddress, from, type);

	//Ensure analyzed location existance
	if( m_referencedAddresses.find(targetAddress) == m_referencedAddresses.end() )
	{
		analyzedMemoryLocation = new UVDAnalyzedMemoryLocation();
		
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
		UVDAnalyzedMemoryLocation *memoryLocation = NULL;
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

bool sortCheck(UVDAnalyzedMemoryLocation *l, UVDAnalyzedMemoryLocation *r)
{
	return UVDMemoryLocation::compareStatic(l, r) >= 0;
} 

void sort(UVDAnalyzedMemoryLocations &locations)
{
	std::sort(locations.begin(), locations.end(), sortCheck);
}

uv_err_t UVDAnalyzer::getAddresses(UVDAnalyzedMemoryLocations &addresses, uint32_t type)
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

uv_err_t UVDAnalyzer::getAnalyzedProgramDB(UVDAnalysisDBArchive **db)
{
	uv_assert_ret(db);
	uv_assert_ret(m_curDb);
	
	//Just always rebuild for now, add dirty field later if needed
	//uv_assert_err_ret(rebuildDb());
	
	*db = m_curDb;
	
	return UV_ERR_OK;	
}

uv_err_t UVDAnalyzer::loadFunction(UVDBinaryFunction *function)
{
	UVDBinaryFunctionShared *functionShared = NULL;

	uv_assert_ret(function);
	//Register it as a found function
	m_functions.insert(function);
	
	//Register the instance to our function analysis database
	functionShared = function->m_shared;
	uv_assert_ret(m_curDb);
	uv_assert_err_ret(m_curDb->loadFunction(functionShared));

	return UV_ERR_OK;
}

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

#define BASIC_SYMBOL_ANALYSIS			

uv_err_t UVDAnalyzer::analyzeCall(UVDInstruction *instruction, uint32_t startPos, const UVDVariableMap &attributes)
{
	std::string sAddr;
	unsigned int targetAddress = 0;

	uv_assert_ret(attributes.find(SCRIPT_KEY_CALL) != attributes.end());
	(*attributes.find(SCRIPT_KEY_CALL)).second.getString(sAddr);
	targetAddress = (unsigned int)strtol(sAddr.c_str(), NULL, 0);
	
	uv_assert_err_ret(insertCallReference(targetAddress, startPos));
	//uv_assert_err(insertReference(targetAddress, startPos, ));
	updateCache(startPos, attributes);

#ifdef BASIC_SYMBOL_ANALYSIS
	/*
	Only simple versions can be parsed for now
	Must have a single immediate as the target value with no calculation required
	*/
	uv_assert_ret(instruction);
	uv_assert_ret(instruction->m_shared);
	if( UV_SUCCEEDED(instruction->m_shared->isImmediateOnlyFunction()) )
	{
		uint32_t relocatableDataSize = 0;
		uv_assert_err_ret(instruction->m_shared->getImmediateOnlyFunctionAttributes(&relocatableDataSize));

		//We know the location of a call symbol relocation
		//uv_assert_ret(m_symbolManager);
		uv_assert_err_ret(m_symbolManager.addAbsoluteFunctionRelocation(targetAddress,
				startPos, relocatableDataSize));
	}
#endif

	return UV_ERR_OK;
}

//#undef BASIC_SYMBOL_ANALYSIS

uv_err_t UVDAnalyzer::analyzeJump(UVDInstruction *instruction, uint32_t startPos, const UVDVariableMap &attributes)
{
	std::string sAddr;
	unsigned int targetAddress = 0;

	uv_assert_ret(attributes.find(SCRIPT_KEY_JUMP) != attributes.end());
	(*attributes.find(SCRIPT_KEY_JUMP)).second.getString(sAddr);
	targetAddress = (unsigned int)strtol(sAddr.c_str(), NULL, 0);
	
	uv_assert_err_ret(insertJumpReference(targetAddress, startPos));
	updateCache(startPos, attributes);

#ifdef BASIC_SYMBOL_ANALYSIS			
	uv_assert_ret(instruction);
	uv_assert_ret(instruction->m_shared);
	if( UV_SUCCEEDED(instruction->m_shared->isImmediateOnlyFunction()) )
	{
		uint32_t relocatableDataSize = 0;
		uv_assert_err_ret(instruction->m_shared->getImmediateOnlyFunctionAttributes(&relocatableDataSize));

		//We know the location of a jump symbol relocation
		//uv_assert_ret(m_symbolManager);
		uv_assert_err_ret(m_symbolManager.addAbsoluteLabelRelocation(targetAddress,
				startPos, relocatableDataSize));
	}
#endif

	return UV_ERR_OK;
}

uv_err_t UVDAnalyzer::mapSymbols()
{
	//uv_assert_ret(m_symbolManager);
	
	//For each function, find its associated symbols
	//This algorithm can be made linear for some extra interaction
	for( std::set<UVDBinaryFunction *>::iterator iter = m_functions.begin(); iter != m_functions.end(); ++iter )
	{
		UVDBinaryFunction *function = *iter;
		uv_assert_ret(function);

		//Get all the relocations for this particular function and register the fixups
		uv_assert_err_ret(m_symbolManager.collectRelocations(function));
	}
	return UV_ERR_OK;
}
