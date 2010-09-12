/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "event/event.h"
#include "event/events.h"
#include <map>

UVDEvent::UVDEvent()
{
	m_type = UVD_EVENT_UNKNOWN;
}

UVDEvent::~UVDEvent()
{
}


