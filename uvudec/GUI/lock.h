/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_GUI_LOCK_H
#define UVD_GUI_LOCK_H

#include <QMutex>

//Will unlock when out of scope
#define UVD_AUTOLOCK_ENGINE() UVDGUIAutolock _uvdEngineAutolock(&g_mainWindow->m_analysisThread->m_uvdMutex)
#define UVD_AUTOLOCK_ENGINE_BEGIN() { UVD_AUTOLOCK_ENGINE()
#define UVD_AUTOLOCK_ENGINE_END() }

/*
Locks the mutex when constructed, unlocks when destructed
To take advantage of scoping for (hopefully) clean locking code
*/
class UVDGUIAutolock
{
public:
	UVDGUIAutolock(QMutex *mutex);
	~UVDGUIAutolock();

public:
	QMutex *m_mutex;
};

#endif

