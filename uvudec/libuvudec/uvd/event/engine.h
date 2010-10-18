/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_EVENT_ENGINE_H
#define UVD_EVENT_ENGINE_H

#include "uvd/event/event.h"
#include "uvd/util/types.h"
#include <map>

/*
Don't be afraid to register at multiple levels instead of trying to hack everything in one
*/
#define UVD_EVENT_HANDLER_PRIORITY_FIRST					0x00000000
//Works at the binary level
//For identifying areas as code or data
#define UVD_EVENT_HANDLER_PRIORITY_BINARY_RECOGNITION		0x00010000
//Has been identified as code and data
//This layer tries to do pre-processing on functions based on 
#define UVD_EVENT_HANDLER_PRIORITY_FUNCTION_RECOGNITION		0x00010000
//We now want to generate IR for the functions
#define UVD_EVENT_HANDLER_PRIORITY_DISASSEMBLY				0x00020000
//Analysis of disassembly, such as figuring out stack frames, data structures, and such
#define UVD_EVENT_HANDLER_PRIORITY_ANALYSIS					0x00030000
//After engine does all of its normal stuff
//Most plugins should register at this level
#define UVD_EVENT_HANDLER_PRIORITY_NORMAL					0x00100000
//really really make sure we wait until other processing is done
//A plugin in this category should not modify the engine state
#define UVD_EVENT_HANDLER_PRIORITY_LAST						0xFFFFFFFF

class UVDRegisteredHandler
{
public:
	bool operator==(const UVDRegisteredHandler &r) const;

public:
	UVDEventHandler m_handler;
	void *m_data;
};


/*
Event handler engine
Think we need priority for handling instead of arbitrary registration order
Ex:
	FLIRT and dissassembly analysis plugin loaded
	FLIRT should go first as if anything is discvered, other plugin is just wasting time
*/
class UVDEvent;
class UVDEventEngine
{
public:
	typedef std::vector<UVDRegisteredHandler> HandlerBucket;
	typedef std::map<uint32_t, HandlerBucket> Handlers;

public:
	UVDEventEngine();
	~UVDEventEngine();
	uv_err_t init();
	uv_err_t deinit();
	
	/*
	Callback for analysis events
	Processed in the order registered
	Improve this later if this is to simplistic
	data: will be saved and called on the handler
	*/
	uv_err_t registerHandler(UVDEventHandler handler, void *data, uint32_t priority);
	//Unregister the first instance found
	//If not found, returns error
	uv_err_t unregisterHandler(UVDEventHandler handler, void *data);
	//Unregister all handlers matching handler
	//Will not return error upon not found
	uv_err_t unregisterHandlerAll(UVDEventHandler handler);
	
	/*
	Throw the event through the engine
	Will do a linear search on handlers
	WARNING: originally this was called "emit"
		But I had issues with q Qt preprocess or something
		rather than fixing it Qt side, I renamed this
	*/
	uv_err_t emitEvent(const UVDEvent *event);
	
	/*
	Register an event type
	Type out is garaunteed to be unique
	*/
	uv_err_t registerEventType(const std::string &name, uint32_t *out);
	uv_err_t registerEventTypeCore(const std::string &name, uint32_t type);

public:
	//Map key is priority
	Handlers m_handlers;
	//Registered event types
	std::map<uint32_t, std::string> m_eventTypes;
	//Current number is availible
	uint32_t m_nextEventKey;
};

#endif

