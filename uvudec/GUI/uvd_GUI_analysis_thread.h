/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_GUI_ANALYSIS_THREAD_H
#define UVD_GUI_ANALYSIS_THREAD_H

#include <QThread>
#include <QMutex>
#include "uvd_types.h"

class UVDMainWindow;
class UVDAnalysisAction;
class UVDGUIAnalysisThread : public QThread
{
public:
	UVDGUIAnalysisThread();
	~UVDGUIAnalysisThread();
	uv_err_t init();

	void run();
	uv_err_t runLoop();
	//Will be free'd internally
	void queueAnalysis(UVDAnalysisAction *action);

protected:
	uv_err_t getNextAction(UVDAnalysisAction **action);

public:
	UVDMainWindow *m_mainWindow;
	//
	std::list<UVDAnalysisAction *> m_analyisActionsQueue;
	QMutex m_analyisActionsQueueMutex;
	//Add some sort of queue here to queue up analsis events
	uint32_t m_active;
};

#endif

