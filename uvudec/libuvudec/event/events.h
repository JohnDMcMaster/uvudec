/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_EVENTS_H
#define UVD_EVENTS_H

#define UVD_EVENT_UNKNOWN				0x00000
//Staticly generated event class numbers
#define UVD_EVENT_STATIC_BASE			0x00001
//Dynamically generated event class numbers
#define UVD_EVENT_DYNAMIC_BASE			0x10000
#define UVD_EVENT_STATIC(offset)		(UVD_EVENT_STATIC_BASE + offset)

//Static event type table definitly needs to be here
//Might scatter class types across code though
//#define UVD_EVENT_FUNCTION_NEW			UVD_EVENT_STATIC(0x0001)
//Function no longer exists
//#define UVD_EVENT_FUNCTION_DELETED		UVD_EVENT_STATIC(0x0002)
#define UVD_EVENT_FUNCTION_CHANGED			UVD_EVENT_STATIC(0x0003)

#endif

