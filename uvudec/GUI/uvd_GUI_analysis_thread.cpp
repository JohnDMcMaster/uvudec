/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd_GUI.h"
#include "uvd_GUI_analysis_thread.h"
#include "uvd_analysis_action.h"
#include <typeinfo>

UVDGUIAnalysisThread::UVDGUIAnalysisThread()
{
}

UVDGUIAnalysisThread::~UVDGUIAnalysisThread()
{
}

uv_err_t UVDGUIAnalysisThread::init()
{
	return UV_ERR_OK;
}

void UVDGUIAnalysisThread::run()
{
	//int pthread_mutex_trylock (pthread_mutex_t *__mutex)
	//int pthread_mutex_lock (pthread_mutex_t *__mutex)
	m_active = TRUE;

	while( m_active )
	{
		UV_DEBUG(runLoop());
	}
	printf("Analysis thread exiting\n");
}

uv_err_t UVDGUIAnalysisThread::runLoop()
{
	UVDAnalysisAction *action = NULL;
	
	uv_assert_err_ret(getNextAction(&action));
	if( !action )
	{
		//printf("S");
		fflush(stdout);
		usleep(10000);
		return UV_ERR_OK;
	}
	if( typeid(*action) == typeid(UVDAnalysisActionBegin) )
	{
		uv_assert_err_ret(m_mainWindow->beginAnalysis());
	}
	else
	{
		printf_error("Unrecognized analysis action\n");
		delete action;	
		return UV_DEBUG(UV_ERR_GENERAL);
	}

	delete action;	
	return UV_ERR_OK;
}

void UVDGUIAnalysisThread::queueAnalysis(UVDAnalysisAction *action)
{
	m_analyisActionsQueueMutex.lock();
	m_analyisActionsQueue.push_back(action);	
	m_analyisActionsQueueMutex.unlock();
}

uv_err_t UVDGUIAnalysisThread::getNextAction(UVDAnalysisAction **action)
{
	uv_assert_ret(action);
	m_analyisActionsQueueMutex.lock();

	if( m_analyisActionsQueue.empty() )
	{
		*action = NULL;
		m_analyisActionsQueueMutex.unlock();
		return UV_ERR_BLANK;
	}
	
	*action = *m_analyisActionsQueue.begin();
	m_analyisActionsQueue.erase(m_analyisActionsQueue.begin());

	m_analyisActionsQueueMutex.unlock();
	return UV_ERR_OK;
}

