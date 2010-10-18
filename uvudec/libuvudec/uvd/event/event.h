/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_EVENT_H
#define UVD_EVENT_H

#include "uvd/util/types.h"
#include <vector>

/*
Analysis or other type of event
I'd like to keep this class non-virtual so events don't become heavyweight monsters 
	Maybe a virtual event class?
*/
class UVDEvent
{
public:
	UVDEvent();
	~UVDEvent();

public:
	//Type of event
	//Determines polymorphism and such
	uint32_t m_type;
	//Do we want a subsystem identifier?
	//Maybe better to map from type, if applicable
};

/*
Likely for various reasons, some events will need virtual functions
Standard interface for string funcs and such if used
*/
class UVDVirtualEvent : public UVDEvent
{
public:
	UVDVirtualEvent();
	virtual ~UVDVirtualEvent();

	virtual uv_err_t toString(std::string &out);
};

/*
Something that cean respond to engine analysis and related events
Returns
	UV_ERR_OK: stop processing on the chain
	UV_ERR_DONE: do not process event any more
		Indicates we have done something that cancels the validity of the event
		Be careful issuing this, you should have a very good reason to do so
		Most likely this will just lead to horrible hacks and abuse...oh well
*/
typedef uv_err_t (*UVDEventHandler)(const UVDEvent *event, void *data);

//Initialize the events system
//uv_err_t UVDEventInit();

#endif

