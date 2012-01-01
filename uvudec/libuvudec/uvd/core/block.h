/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_CORE_BLOCK_H
#define UVD_CORE_BLOCK_H

#include "uvd/assembly/address.h"
#include <vector>
#include <set>


/*
A block of code
	May contain nested blocks and/or actual code
Highest level block should be the entire program
	Next level is functions
	The lowest level blocks should be non-branching segments
*/
#if 0
class UVDDataChunk;
class UVDAnalyzedCode;
class UVDAnalyzedBlock
{
public:
	UVDAnalyzedBlock();
	~UVDAnalyzedBlock();
	uv_err_t deinit();
	
	//Get the actual code representation of this block
	uv_err_t getDataChunk(UVDDataChunk **dataChunk);
	
	uv_err_t getMinAddress(uv_addr_t *address);
	uv_err_t getMaxAddress(uv_addr_t *address);
	uv_err_t getSize(size_t *size);
	
public:
	//Both of following can be set
	//m_code should always indicate the range and subblocks, if present, also indicated
	//If it contains code
	//We own this
	UVDAnalyzedCode *m_code;
	//If it contains blocks, usually should be more than one
	std::vector<UVDAnalyzedBlock *> m_blocks;
	UVDAddressSpace *m_addressSpace;
};
#endif

/*
Original is mediocre, spend some time thinking rev 2 out...

Requirements
-Blocks must be nestable
-Must be able to represent as blocks
	-Function
	-Branching
		-Provide information on the branch point
			Pointer to instruction?  Should be the last instruction in the block
		-if clause, else clause
		-while loop
-Must be able to get all blocks associated with address
-Must be ale to iterate over blocks
-Must be able to iterativly add and remove blocks
	Initial analysis should use this method instead of bulk create
-Must be able to get all sub-blocks associated with a block
-No memory / performance requirements, solve those problems as they come up


Binary executable format construction algorithm
Do either linear sweep or recursive descent and note function entry points / conditional branching
	Should these be moved into helper objects?
Each time a block is found it should be added to the working set, not put in some special store
	Can be optimized later if possible / necessary
Blocks must reference UVDAddress's, not pointers to data
	A utility function can be provided to get the data if needed, but more important is following
It must be easy to create an instruction iterator from a block

Represent as self-identifying UVDAddressRange objects for now
	If memory consumption or other issues come up can cut out the address space object but don't track for now

Don't want blocks to support newsting at the lowest level
	This seems more of a high level analysis construct
	For example, given some condition branches one would have to discover a loop first and then realize the if 


Scenarios

Display a function with control flow
User will click on function in GUI
GUI will render disassembly by:
	-fetching the start and end of the block associated with the function
	-creating a disassembly iterator with this start and end
	-iterating
GUI will then draw control flow by iterating over all control flow within the function and recursing as necessary
A block should not be aware wether or not its in a function
	Analysis should be left to the function and not in the UVDBasicBlockblock
Blocks will be owned by the address space, not other blocks
Blocks can only be referenced by blocks within the same address space
Block creation and deletion must be through an address space
Deleting a block at the address space level must be able to remove all references to that block
	 Functions and other blocks at a minimum
	 Would prefer that an address space is not aware of UVD
	 Also, would like plugins to be able to easily deal with block management
	 Block creation and deletion will have a per address space callback mechanism
	 
Plan on making these pointers since we'll eventually want to start attaching more analysis info
Block address can become primary key if nothing else
*/
class UVDBasicBlock {
public:
	UVDBasicBlock();
	UVDBasicBlock(UVDAddressRange addressRange);
	~UVDBasicBlock();
	
public:
	UVDAddressRange m_addressRange;
};

typedef std::vector<UVDBasicBlock *> UVDBasicBlockList;
typedef std::set<UVDBasicBlock *> UVDBasicBlockSet;

//TODO: consider using a pointer to object if we start needing arguments
class UVDBlockEvent {
};

typedef enum {
	UVD_BLOCK_EVENT_INVALID = 0,
	/*
	The block has just been created
	*/
	UVD_BLOCK_EVENT_NEW = 1,
	/*
	The block will be deleted soon
	After the callback returns there is no garauntee its pointer will be valid
	*/
	UVD_BLOCK_EVENT_DELETE = 2,
	/*
	Reserved for future use
	*/
	UVD_BLOCK_EVENT_CHANGED = 3,
} uvd_block_event_t;

/*
Vectored multimap
Has an ordered list of items on each key
*/
/*
namespace uvdstd {
	template class <_T, _U>
	class multimap : public std::multimap {
	public:
		multimap();
	};
}
*/

/*
A registered block may not change address while added
If it changes address this is a new block and must be re-added
	Should provide a rotate block call?
	Might help some things that need to deal with the switch out


maybe instead of bucketed map I should create a comparator instead of using just the address
The only issue this creates is how to specify up to and including an address
	but NULL might work just as easily
*/

/*
Do not use the internal data members, this class will be turbulent
*/
class UVDBlockGroup {
public:
	//typedef std::map<uv_addr_t, UVDBasicBlockSet *> BBMS;
	/*
	In order to query a range we need to query data in both the forward and reverse	ranges
	We can then construct the union of these to figure out whats actually availible
	Otherwise we would have to iterate backward an indeterminate amount of time finding
	old overlapping blocks
	*/
	//typedef std::multimap<uv_addr_t, UVDBasicBlockSet *> BBSMM;
	/*
	We don't just form a set because we want multiple items at each address
	*/
	typedef std::pair<uv_addr_t, UVDBasicBlock *> K;
	//static int BBSMC(BBSMK l, BBSMK r);
	typedef std::set<K> BBPS;
	typedef std::set<UVDBasicBlock *> BBS;
	//Internal use for constructing keys
	//const UVDBasicBlock * BlockMax = (const UVDBasicBlock *)UINT_MAX;
	
	//typedef uv_err_t (*Notifier)(UVDBasicBlock *block, const UVDBlockEvent *event, void *user);
	typedef uv_err_t (*Notifier)(UVDBasicBlock *block, uvd_block_event_t event, void *user);
	//Stores the user defined parameter with the notifier
	typedef std::pair<Notifier, void *> Notification;
	
public:
	UVDBlockGroup();
	~UVDBlockGroup();
	uv_err_t init(UVDAddressSpace *addressSpace);
	
	//Add
	//We now own this and it will be deleted at object destruction
	uv_err_t add(UVDBasicBlock *block);
	//Remove but don't delete
	uv_err_t remove(UVDBasicBlock *block);
	//Remove and delete
	uv_err_t del(UVDBasicBlock *block);
	
	uv_err_t getAtAddress( uv_addr_t address, UVDBasicBlockSet *out );
	//Intended primarily to get all of the blocks associated with a function but many other uses possible4444dw
	uv_err_t getAtAddresses( UVDAddressRange addressRange, UVDBasicBlockSet *out );
	
	//Notifiers are added such that newest is last called
	//This allows users to have all internal data structures setup first
	//but might not be flexible enough, see how it goes
	//Get notified when a block changes status
	uv_err_t addNotifier(Notifier notifier, void *user);
	//if a notification was added twice the first instance will be removed
	uv_err_t removeNotifier(Notifier notifier, void *user);

protected:
	uv_err_t removeCore(UVDBasicBlock *block, bool del);
	BBPS::iterator findForward(UVDBasicBlock *block);
	BBPS::iterator findReverse(UVDBasicBlock *block);
	K forwardK(UVDBasicBlock *block);
	K reverseK(UVDBasicBlock *block);
	uv_err_t notify(UVDBasicBlock *block, uvd_block_event_t event);

public:
	UVDAddressSpace *m_addressSpace;
	//Address based index
	//BBMS m_byAddress;
	//The actual blocks we own
	//std::set<UVDBasicBlock *> m_blocks;
	//Prepare vector over set for deterministic behavior
	std::vector<Notification> m_notifications;
	BBPS m_forward;
	BBPS m_reverse;
	
	//This is not strictly necessary and is really more for debugging
	//With this architecture it is difficult to
	//determine if we added something twice by accident
	BBS m_unique;
};

#endif

