/*
Copyright 2008 John McMaster
JohnDMcMaster@gmail.com
*/

#include "uvd.h"
#include "uvd_analysis_db.h"
#include "uvd_analyzer.h"
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
			printf_error("ERROR: missing value\n");
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

uv_err_t UVDAnalyzer::getAnalyzedProgramDB(UVDAnalysisDBArchive **db)
{
	uv_assert_ret(db);
	uv_assert_ret(m_curDb);
	
	*db = m_curDb;
	
	return UV_ERR_OK;	
}
