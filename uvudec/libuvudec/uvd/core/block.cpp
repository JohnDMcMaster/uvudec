/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/assembly/cpu_vector.h"
#include "uvd/assembly/function.h"
#include "uvd/core/uvd.h"
#include "uvd/core/analyzer.h"
#include "uvd/core/block.h"
#include "uvd/core/event.h"
#include "uvd/core/runtime.h"
#include "uvd/event/engine.h"
#include "uvd/string/engine.h"
#include "uvd/util/benchmark.h"
#include <algorithm>
#include <stdio.h>
#include "uvd/util/tl/set.h"

using namespace UVDN;

//System this is running on max address
//Needed for getting a pair higher than given (upper_bound()) where first was primary key and second was address
#define UVD_VIRT_ADDR_MAX			((void *)0xFFFFFFFF)


#if 0

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
	UVDDataChunk *dataChunk = NULL;
	
	uv_assert_ret(m_code);
	dataChunk = m_code->m_dataChunk;
	uv_assert_ret(dataChunk);
	
	uv_assert_ret(dataChunkIn);
	*dataChunkIn = dataChunk;

	return UV_ERR_OK;
}

uv_err_t UVDAnalyzedBlock::getMinAddress(uv_addr_t *out)
{
	UVDDataChunk *dataChunk = NULL;
	
	uv_assert_err_ret(getDataChunk(&dataChunk));
	uv_assert_ret(dataChunk);
	
	uv_assert_ret(out);
	*out = dataChunk->m_offset;

	return UV_ERR_OK;
}

uv_err_t UVDAnalyzedBlock::getMaxAddress(uv_addr_t *out)
{
	UVDDataChunk *dataChunk = NULL;
	
	uv_assert_err_ret(getDataChunk(&dataChunk));
	uv_assert_ret(dataChunk);
	
	uv_assert_ret(out);
	*out = dataChunk->m_offset + dataChunk->m_bufferSize;

	return UV_ERR_OK;
}

uv_err_t UVDAnalyzedBlock::getSize(size_t *out)
{
	UVDDataChunk *dataChunk = NULL;
	
	uv_assert_err_ret(getDataChunk(&dataChunk));
	uv_assert_ret(dataChunk);
	
	uv_assert_ret(out);
	*out = dataChunk->m_bufferSize;

	return UV_ERR_OK;
}





//Second pass used to create for DB storage
//Block contains analyzed code
uv_err_t UVD::blockToFunction(UVDAnalyzedBlock *functionBlock, UVDBinaryFunction **functionOut)
{
	UVDBinaryFunction *function = NULL;
	//Since ownership will be xferred to object, we need to map it twice
	UVDDataChunk *functionBlockDataChunk = NULL;

	uv_assert_ret(functionBlock);
	uv_assert_ret(functionOut);

	
	uv_assert_err_ret(functionBlock->getDataChunk(&functionBlockDataChunk));
	uv_assert_ret(functionBlockDataChunk)

	//Create the single known instance of this function
	uv_assert_err_ret(UVDBinaryFunction::getUVDBinaryFunctionInstance(&function));
	uv_assert_ret(function);

	uv_addr_t minAddress = 0;
	uv_assert_err_ret(functionBlock->getMinAddress(&minAddress));
	uv_assert_err_ret(function->setSymbolAddress(minAddress));
	
	//The true name of the function is unknown
	//This name is stored as a way to figure out the real name vs the current symbol name 
	//functionShared->m_name = "";
	//functionShared->m_description = "Automatically generated";	

	function->m_symbolAddress = UVDRelocatableElement(minAddress);
	//Only specific instances get symbol designations
	std::string symbolName;
	uv_assert_err_ret(m_analyzer->m_symbolManager.analyzedSymbolName(minAddress, UVD__SYMBOL_TYPE__FUNCTION, symbolName));
	function->setSymbolName(symbolName);
	
	//This will perform copy
	uv_assert_err_ret(function->setData(functionBlockDataChunk));

	*functionOut = function;

	return UV_ERR_OK;
}

uv_err_t UVD::constructBlock(UVDAddressRange addressRange, UVDAnalyzedBlock **blockOut)
{
	/*
	TODO: move this to UVDAnalyzedBlock->init()
	*/
	
	UVDAnalyzedBlock *block = NULL;
	UVDAnalyzedCode *analyzedCode = NULL;
	UVDDataChunk *dataChunk = NULL;
	UVDData *data = NULL;
	//uint32_t dataSize = 0;
	
	uv_assert_ret(addressRange.m_space);
	data = addressRange.m_space->m_data;
	uv_addr_t minAddr = addressRange.m_min_addr;
	uv_addr_t maxAddr = addressRange.m_max_addr;
	
	uv_assert_ret(m_config);
	uv_assert_ret(blockOut);
	
	printf_debug("Constructing block 0x%.8X:0x%.8X\n", minAddr, maxAddr);
	uv_assert_ret(data);

	uv_assert_ret(minAddr <= maxAddr);
	//uv_assert_ret(maxAddr <= m_config->m_addr_max);
	//dataSize = maxAddr - minAddr;

	block = new UVDAnalyzedBlock();
	uv_assert_ret(block);
	block->m_addressSpace = addressRange.m_space;
	
	analyzedCode = new UVDAnalyzedCode();
	uv_assert_ret(analyzedCode);
	block->m_code = analyzedCode;

	dataChunk = new UVDDataChunk();
	uv_assert_ret(dataChunk);
	uv_assert_ret(data);
	uv_assert_err_ret(dataChunk->init(data, minAddr, maxAddr));
	uv_assert_ret(dataChunk->m_data);
	analyzedCode->m_dataChunk = dataChunk;

	*blockOut = block;

	return UV_ERR_OK;
}


uv_err_t UVD::constructJumpBlocks(UVDAnalyzedBlock *superblock, UVDAnalyzedMemoryRanges &superblockLocations, UVDAnalyzedMemoryRanges::iterator &iterSuperblock)
{
	//Commented out because its not being used?  What about labeling?
	return UV_ERR_OK;
/*
	uv_err_t rc = UV_ERR_GENERAL;
	uint32_t lastSuperblockLocation = 0;
	uint32_t superblockMinAddress = 0;
	uint32_t superblockMaxAddress = 0;
	UVDAnalyzedBlock *jumpedBlock = NULL;

	printf_debug("\n");
	UV_ENTER();

	uv_assert(superblock);

	uv_assert_err(superblock->getMinAddress(superblockMinAddress));
	uv_assert_err(superblock->getMaxAddress(superblockMaxAddress));
	
	//Skip ahead to where the superblock is
	while( iterSuperblock != superblockLocations.end() )
	{
		UVDAnalyzedMemoryRange *memoryCalled = *iterSuperblock;
		uv_assert(memoryCalled);

		//Do we have at least func start address?
		if( memoryCalled->m_min_addr >= superblockMinAddress )
		{
			//If so, we are done seeking
			break;
		}
		++iterSuperblock;
	}
	//Loop for all called locations in this block
	for( ; iterSuperblock != superblockLocations.end(); ++iterSuperblock )
	{
		UVDAnalyzedMemoryRange *memoryJumped = *iterSuperblock;		
		uint32_t jumpedBlockEnd = 0;
		
		uv_assert(memoryJumped);
		
		//Are we past superblock?
		if( memoryJumped->m_min_addr > superblockMaxAddress )
		{
			break;
		}
		
		//Nothing precedes superblock start, skip block
		if( memoryJumped->m_min_addr == superblockMinAddress )
		{
			continue;
		}

		//Add the block
		jumpedBlockEnd = memoryJumped->m_min_addr - 1;
		uv_assert_err(constructBlock(lastSuperblockLocation, jumpedBlockEnd, &jumpedBlock));
		uv_assert(jumpedBlock);
		superblock->m_blocks.push_back(jumpedBlock);
		lastSuperblockLocation = memoryJumped->m_min_addr;
	}
	
	//Finally, end of function block
	//We want location after last block
	uv_assert_err(constructBlock(lastSuperblockLocation, superblockMaxAddress, &jumpedBlock));
	uv_assert(jumpedBlock);
	superblock->m_blocks.push_back(jumpedBlock);
	
	rc = UV_ERR_OK;
	
error:
	return UV_DEBUG(rc);
*/
}


#endif




UVDBasicBlock::UVDBasicBlock() {
	//m_addressRange
}

UVDBasicBlock::UVDBasicBlock(UVDAddressRange addressRange) {
	m_addressRange = addressRange;
}

UVDBasicBlock::~UVDBasicBlock() {
}

uv_addr_t UVDBasicBlock::min() {
	return m_addressRange.min();
}

uv_addr_t UVDBasicBlock::max() {
	return m_addressRange.max();
}



/*
UVDBlockGroup
*/
UVDBlockGroup::UVDBlockGroup() {
	m_addressSpace = NULL;
}

UVDBlockGroup::~UVDBlockGroup() {
}

uv_err_t UVDBlockGroup::init(UVDAddressSpace *addressSpace) {
	uv_assert_ret(addressSpace);
	m_addressSpace = addressSpace;
	return UV_ERR_OK;
}

uv_err_t UVDBlockGroup::add(UVDBasicBlock *block) {
	//printf("1\n");
	uv_assert_ret(block);
	//printf("2\n");
	//Don't require it to have a space but if it does it must match
	if (block->m_addressRange.m_space) {
		uv_assert_ret(block->m_addressRange.m_space == m_addressSpace);
	}
	if (m_unique.find(block) != m_unique.end()) {
		return UV_ERR_DUPLICATE;
	}
	m_unique.insert(block);
	{
		interval_t inter = interval(block);
		BBS s = singleton_set(block);
		std::pair<interval_t, BBS> p = std::make_pair(inter, s);
		
		//m_map += std::make_pair(inter, s);
		m_map += p;
		//m_map.add(p);
	}
	
	return UV_ERR_OK;
}

#if 0
UVDBlockGroup::interval_t UVDBlockGroup::interval(uv_addr_t startend) {
	return interval(startend, startend);
}

UVDBlockGroup::interval_t UVDBlockGroup::interval(UVDBasicBlock *block) {
	return interval(block->min(), block->max()), singleton_set(block));
}

UVDBlockGroup::interval_t UVDBlockGroup::interval(uv_addr_t start, uv_addr_t end) {
	return interval_t::closed(start, end);
}

uv_err_t UVDBlockGroup::removeCore(UVDBasicBlock *block, bool del) {
	BBS::iterator iterUnique = m_unique.find(block);
	
	if (iterUnique == m_unique.end()) {
		return UV_ERR_NOTFOUND;
	}
	m_unique.erase(iterUnique);
	//Not that it should matter, but it should be safe if already erased
	//The biggest danger for corruption is if its range had changed meaning we didn't
	//erase it and could return a bad pointer
	m_map.erase(interval(block));
	
	if (del) {
		delete block;
	}
	return UV_ERR_OK;
}

uv_err_t UVDBlockGroup::notify(UVDBasicBlock *block, uvd_block_event_t event) {
	//TODO: implement this so that we can respond to blocks being moved around
	//We need to register with the address space
	return UV_ERR_OK;
}

uv_err_t UVDBlockGroup::remove(UVDBasicBlock *block) {
	return removeCore(block, false);
}

uv_err_t UVDBlockGroup::del(UVDBasicBlock *block) {
	return removeCore(block, true);
}

uv_err_t UVDBlockGroup::getAtAddress( uv_addr_t address, UVDBasicBlockSet *out ) {
	return UV_DEBUG(getAtAddresses(UVDAddressRange(address, address), out));
}

uv_err_t UVDBlockGroup::getAtAddresses( UVDAddressRange addressRange, UVDBasicBlockSet *out ) {
    for (Map::iterator iter = m_map.lower_bound(interval(addressRange.min()));
    		iter != m_map.upper_bound(interval(addressRange.max())); 
    		++iter) {
		//This doesn't care about the who, only the what
        BBS what = iter->second;
        for (BBS::iterator what_iter = what.begin(); what_iter != what.end(); ++what_iter) {
        	//Note that its safe to add duplicates
        	out->insert(*what_iter);
        }
    }
	
	return UV_ERR_OK;
}

uv_err_t UVDBlockGroup::addNotifier(Notifier notifier, void *user) {
	uv_assert_ret(notifier);
	
	m_notifications.push_back(Notification(notifier, user));
	return UV_ERR_OK;
}

uv_err_t UVDBlockGroup::removeNotifier(Notifier notifier, void *user) {
	uv_assert_ret(notifier);
	std::vector<Notification>::iterator iter = std::find(m_notifications.begin(), m_notifications.end(), Notification(notifier, user));
	if (iter == m_notifications.end()) {
		return UV_ERR_NOTFOUND;
	} else {
		m_notifications.erase(iter);
		return UV_ERR_OK;
	}
}
#endif

