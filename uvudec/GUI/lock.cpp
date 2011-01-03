/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "GUI/lock.h"

UVDGUIAutolock::UVDGUIAutolock(QMutex *mutex)
{
	m_mutex = mutex;
	
	if( m_mutex )
	{
		m_mutex->lock();
	}
}

UVDGUIAutolock::~UVDGUIAutolock()
{
	if( m_mutex )
	{
		m_mutex->unlock();
	}
}

