/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd_core_event.h"
#include "event/events.h"

UVDEventFunction::UVDEventFunction()
{
	m_function = NULL;
	m_type = UVD_EVENT_FUNCTION_CHANGED;
}

UVDEventFunction::~UVDEventFunction()
{
}

UVDEventFunctionChanged::UVDEventFunctionChanged()
{
	m_isDefined = 0;
}

UVDEventFunctionChanged::~UVDEventFunctionChanged()
{
}

