/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#ifndef UVD_PRIORITY_LIST_H
#define UVD_PRIORITY_LIST_H

#include <stdint.h>
#include <vector>
#include "uvd_types.h"

/*
A priority based list for determining complex priorities
Originally written for determining address space intersections and validity
Also an excersize in templating
Each list item should have a lower and upper bound

WARNING: this is templated, but I couldn't ge things to work right, so it will only work for the class I needed originally

T: priority key
U: match value
Should add V to seperate priority query value and the keys stored?
	This is an issue for the range compares in a way
	Currently solving this by creating a single value range
	I'll probably want such comparisons in the future on the same data structure as well, so don't worry about for now
*/

/*
UVDPriorityListItem
*/

template <typename T, typename U>
class UVDPriorityListItem
{
public:
	UVDPriorityListItem(T t, U matchState);
	~UVDPriorityListItem();
	
	//A strict does this fall in the requested range check
	bool matches(T t);
	//XXX FIXME
	bool matches(uint32_t t);
	//int compare();
	
public:
	//The key to compare against
	T m_t;
	//The value to fetch if a match occurs
	U m_matchState;
};

template <typename T, typename U>
UVDPriorityListItem<T, U>::UVDPriorityListItem(T t, U matchState)
{
	m_t = t;
	m_matchState = matchState;
}

template <typename T, typename U>
UVDPriorityListItem<T, U>::~UVDPriorityListItem<T, U>()
{
}

template <typename T, typename U>
//XXX fixme
//Until I fix this...
bool UVDPriorityListItem<T, U>::matches(T t)
{
/*
	return t == m_t;
}

template <typename T, typename U>
bool UVDPriorityListItem<UVDUint32RangePair, U>::matches(UVDUint32RangePair t)
{
*/
	//Queried range must fall completly within this range
	return t.m_min >= m_t.m_min && t.m_max <= m_t.m_max;
}

template <typename T, typename U>
bool UVDPriorityListItem<T, U>::matches(uint32_t t)
{
	return t >= m_t.m_min && t <= m_t.m_max;
}

/*
UVDPriorityList
This should have actually extended the templated list
*/
template <typename T, typename U>
//class UVDPriorityList
class UVDPriorityList : public std::vector< UVDPriorityListItem<T, U> >
{
public:
	//typedef typename std::vector< UVDPriorityListItem<T, U> >::iterator iterator;
	//typedef typename std::vector< UVDPriorityListItem<T, U> >::const_iterator const_iterator;

public:
	UVDPriorityList();
	~UVDPriorityList();

	void add(T t, U matchState);
	U match(T t);
	
	//iterator begin();
	//iterator end();
	
	//size_t size() const;
	//bool empty() const;
	
public:
	//std::vector< UVDPriorityListItem<T, U> > m_list;
	//State if no match is found
	U m_default;
	
};

template <typename T, typename U>
UVDPriorityList<T, U>::UVDPriorityList()
{
}

/*
XXX fixme
template <typename T, typename uint32_t>
UVDPriorityList<T, U>::UVDPriorityList()
{
	m_default = 0;
}
*/

template <typename T, typename U>
UVDPriorityList<T, U>::~UVDPriorityList()
{
}

template <typename T, typename U>
void UVDPriorityList<T, U>::add(T t, U matchState)
{
	push_back(UVDPriorityListItem<T, U>(t, matchState));
}

template <typename T, typename U>
U UVDPriorityList<T, U>::match(T t)
{
	for( typename std::vector< UVDPriorityListItem<T, U> >::iterator iter = this->begin();
		iter != this->end(); ++iter )
	{
		//Do they match?
		if( (*iter).matches(t) )
		{
			return (*iter).m_matchState;
		}
	}
	//No items match, default it
	return m_default;
}

/*
template <typename T, typename U>
typename UVDPriorityList<T, U>::iterator UVDPriorityList<T, U>::begin()
{
	return m_list.begin();
}

template <typename T, typename U>
typename UVDPriorityList<T, U>::iterator UVDPriorityList<T, U>::end()
{
	return m_list.end();
}

template <typename T, typename U>
size_t UVDPriorityList<T, U>::size()
{
	return m_list.size();
}

template <typename T, typename U>
bool UVDPriorityList<T, U>::empty() const
{
	return m_list.empty();
}
*/

/*
UVDUint32RangePriorityList
*/
class UVDUint32RangePriorityList : public UVDPriorityList<UVDUint32RangePair, uint32_t>
{
public:
	UVDUint32RangePriorityList();
	~UVDUint32RangePriorityList();
	
	uint32_t match(uint32_t val);
	void add(uint32_t low, uint32_t high, uint32_t matchState);
};

#endif
