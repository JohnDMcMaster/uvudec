/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_CORE_EVENT_H
#define UVD_CORE_EVENT_H

#include "uvd/event/event.h"

/*
Function related events
*/
class UVDBinaryFunction;
class UVDEventFunction : public UVDEvent
{
public:
	UVDEventFunction();
	~UVDEventFunction();
public:
	//Don't own this
	UVDBinaryFunction *m_function;
};

//UVD_EVENT_FUNCTION_CHANGED
class UVDEventFunctionChanged : public UVDEventFunction
{
public:
	UVDEventFunctionChanged();
	~UVDEventFunctionChanged();
public:
	//Set if we just defined this function
	//Cleared if we are deleting it
	uint32_t m_isDefined;
};

#if 0
/*
UVD_EVENT_FUNCTION_NEW
Issued when a new function is discovered
*/
class UVDEventFunctionNew : public UVDEventFunction
{
public:
	
};

/*
UVD_EVENT_FUNCTION_DELETED
*/
class UVDEventFunctionDeleted : public UVDEventFunction
{
public:
	
};
#endif

#if 0
/*
Issued when an existing function is changed
Includes
*/
class UVDEventFunctionModified : public UVDEventFunction
{
public:
	
};
#endif


//FIXME: I think we define something like this elsewhere
typedef uint32_t uv_code_t;
//Hasn't been analyzed yet, no data was previously availible
#define UVD_BINARY_DATA_UNKNOWN		0
//Is executable code
#define UVD_BINARY_DATA_CODE		1
//Is some data
#define UVD_BINARY_DATA_DATA		2
//Should we add a special type for code like data? (eg: switch table)
//Or should this just be data
//Probably just be data

/*
The binary data type has been updated in some way
*/
class UVDEventBinaryChanged
{
public:
	uv_addr_t m_startAddress;
	uv_addr_t m_endAddress;

	uv_code_t m_startType;
	uv_code_t m_endType;
};

#if 0
/*
New data became availible
Causes:
-Initial analysis of program found a string table or something
-User manually converted code to data
*/
class UVDEventNewData : public UVDEventBinaryChanged
{
public:
};

class UVDEventDeletedData : public UVDEventBinaryChanged
{
public:
};

/*
New code became availible
Similar conditions to above
*/
class UVDEventNewCode : public UVDEventBinaryChanged
{
public:	
};

class UVDEventDeletedCode : public UVDEventBinaryChanged
{
public:	
};

#endif

#endif

