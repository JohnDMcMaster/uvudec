/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/util/priority_list.h"

/*
UVDUint32RangePriorityList
*/

UVDUint32RangePriorityList::UVDUint32RangePriorityList()
{
}

UVDUint32RangePriorityList::~UVDUint32RangePriorityList()
{
}

//For the common single address case
uint32_t UVDUint32RangePriorityList::match(uint32_t val)
{
	//Might add a more specialized match routine for this later, but this should work	
	return UVDPriorityList<UVDUint32RangePair, uint32_t>::match(UVDUint32RangePair(val, val));
}

void UVDUint32RangePriorityList::add(uint32_t low, uint32_t high, uint32_t matchState)
{
	UVDUint32RangePair pair = UVDUint32RangePair(low, high);
	UVDPriorityList<UVDUint32RangePair, uint32_t>::add(pair, matchState);
}
