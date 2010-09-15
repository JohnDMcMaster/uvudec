/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the GPL V3 or later, see COPYING for details
*/

#include "uvd_GUI.h"
#include "uvd_GUI_analysis_thread.h"
#include "uvd_analysis_action.h"
#include "util/io.h"
#include "uvd_language.h"
#include "uvd_core_event.h"
#include "event/event.h"
#include "event/events.h"
#include "event/engine.h"
#include <typeinfo>
#include "uvd_debug.h"

static uv_err_t printCallback(const std::string &in, void *data)
{
	/*
	GUI components can only be called from the main thread
	If we are not the main thread, we need to queue it
	Is it worth the effort to send directly if we are in the main thread?
		Or is this taken care for us?
	*/
	UVDGUIAnalysisThread *analysisThread = (UVDGUIAnalysisThread *)data;
	
	//(void)mainWindow;	
	//printf("Trying to print\n");
	//fflush(stdout);
	
	uv_assert_err_ret(analysisThread->printLogEntry(in));

	return UV_ERR_OK;
}

uv_err_t UVDGUIAnalysisThread::printLogEntry(const std::string &in)
{
	emit printLog(QString::fromStdString(in));
	return UV_ERR_OK;
}

static uv_err_t GUIUVDEventHandler(const UVDEvent *event, void *data)
{
	UVDGUIAnalysisThread *analysisThread = (UVDGUIAnalysisThread *)data;

	//printf("event callback, mainwindow: 0x%08X\n", mainWindow);
	//uv_assert_ret(mainWindow);
	//uv_assert_ret(mainWindow->m_analysisThread);
	//printf("event callback, analysis thread: 0x%08X\n", mainWindow->m_analysisThread);
	uv_assert_err_ret(analysisThread->handleUVDEvent(event));

	return UV_ERR_OK;
}

UVDGUIAnalysisThread::UVDGUIAnalysisThread()
{
}

UVDGUIAnalysisThread::~UVDGUIAnalysisThread()
{
}

uv_err_t UVDGUIAnalysisThread::init()
{
	uv_assert_err_ret(UVDRegisterPrintCallback(printCallback, this));

	return UV_ERR_OK;
}

void UVDGUIAnalysisThread::run()
{
	//int pthread_mutex_trylock (pthread_mutex_t *__mutex)
	//int pthread_mutex_lock (pthread_mutex_t *__mutex)
	m_active = TRUE;

	printf("Analysis thread started\n");
	fflush(stdout);
	while( m_active )
	{
		UV_DEBUG(runLoop());
	}
	printf("Analysis thread exiting\n");
	fflush(stdout);
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
	printf("GUI analysis thread request recieved\n");
	fflush(stdout);
	if( typeid(*action) == typeid(UVDAnalysisActionBegin) )
	{
		uv_assert_err_ret(beginAnalysis());
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

uv_err_t UVDGUIAnalysisThread::beginAnalysis()
{
	/*
	This is where the magic starts
	*/

	std::string output;
	UVD *uvd = NULL;
	UVDData *data = NULL;

	printf_debug_level(UVD_DEBUG_PASSES, "main: initializing data streams\n");
	UVDPrintf("Beginning analysis");

	uv_assert_ret(g_config);
	uv_assert_ret(!g_config->m_targetFileName.empty());

	//Select input
	printf_debug_level(UVD_DEBUG_SUMMARY, "Initializing data stream on %s...\n", g_config->m_targetFileName.c_str());
	uv_assert_err_ret(UVDDataFile::getUVDDataFile(&data, g_config->m_targetFileName));
	uv_assert_ret(data);
	
	//Create a runTasksr engine active on that input
	printf_debug_level(UVD_DEBUG_SUMMARY, "runTasks: initializing engine...\n");
	uv_assert_err_ret(UVD::getUVD(&uvd, data));
	uv_assert_ret(uvd);
	uv_assert_ret(g_uvd);
	m_mainWindow->m_project->m_uvd = uvd;

	//Get our callbacks ready...
	printf("initializing callbacks\n");
	fflush(stdout);
	uv_assert_err_ret(initializeUVDCallbacks());
	//Fire at will
	printf("analyzing\n");
	fflush(stdout);
	UVDPrintf("Analyzing");
	uv_assert_err_ret(uvd->analyze());

	printf("setting lang\n");
	fflush(stdout);
	uv_assert_err_ret(uvd->setDestinationLanguage(UVD_LANGUAGE_ASSEMBLY));

	UVDPrintf("Disassembling");
#if 0
	std::string deadListing;
	//FIXME: use the iterator directly so we don't waste time with the whole line
	uv_assert_err_ret(uvd->printRangeCore(uvd->begin(), uvd->end(), deadListing));
	QString deadListingQ = QString::fromStdString(deadListing);
	//m_mainWindow.disassemblyArea->appendPlainText(deadListingQ);
	//m_mainWindow.disassemblyArea->setPlainText(deadListingQ);
	//Is this the correct way to do it?
	//emit m_mainWindow.disassemblyArea->setPlainText(deadListingQ);
	//std::vector<std::string> lines = split(const std::string &s, char delim, bool ret_blanks = true);

	//This should become less necessary as event system pushes events instead of rebuilding each time
	//uv_assert_err_ret(updateAllViews());	
#endif

	uv_assert_err_ret(disassembleRange(uvd->begin(), uvd->end()));
	
	UVDPrintf("Initial analysis completed");

	delete data;

	return UV_ERR_OK;
}

uv_err_t UVDGUIAnalysisThread::disassembleRange(UVDIterator iterBegin, UVDIterator iterEnd)
{
	UVDIterator iter;
	//UVDIterator iterEnd;

	iter = iterBegin;

	//FIXME: what if we misalign by accident and surpass?
	//need to add some check for that
	//maybe we should do <
	while( iter != iterEnd )
	{
		char buff[256];		
		std::string lineRaw;
		std::string lineDone;
		uint32_t maxOpcodeBytes = 4;

		uv_assert_err_ret(iter.getCurrent(lineRaw));
		uv_assert_ret(iter.m_data);

		//snprintf(buff, sizeof(buff), "%04X: %s", , lineRaw.c_str());

		//opcode bytes
		//printf("iter.m_instruction.m_inst_size: %d\n", iter.m_instruction.m_inst_size);
		fflush(stdout);
		lineDone += m_mainWindow->m_project->m_uvd->m_format->formatAddress(iter.getPosition());
		//printf("line done w/ address: %s\n", lineDone.c_str());
		lineDone += "  ";
		for( uint32_t i = 0; i < iter.m_instruction.m_inst_size; ++i )
		{
			snprintf(buff, sizeof(buff), "%02X", (unsigned int)(unsigned char)iter.m_instruction.m_inst[i]);
			lineDone += buff;
			--maxOpcodeBytes;
		}
		while( maxOpcodeBytes )
		{
			lineDone += "  ";
			--maxOpcodeBytes;
		}
		
		//lineDone += ": ";
		lineDone += "  ";
		lineDone += lineRaw;
		
		
		emit lineDisassembled(QString::fromStdString(lineDone));

		if( UV_FAILED(iter.next()) )
		{
			printf_debug("Failed to get next\n");
			return UV_DEBUG(UV_ERR_GENERAL);
		}
	}
	return UV_ERR_OK;
}

uv_err_t UVDGUIAnalysisThread::initializeUVDCallbacks()
{
	UVDEventEngine *eventEngine = m_mainWindow->m_project->m_uvd->m_eventEngine;
	
	uv_assert_ret(eventEngine);
	uv_assert_err_ret(eventEngine->registerHandler(GUIUVDEventHandler, this, UVD_EVENT_HANDLER_PRIORITY_NORMAL));

	return UV_ERR_OK;
}

uv_err_t UVDGUIAnalysisThread::handleUVDEvent(const UVDEvent *event)
{
	/*
	blah
	*/
	//printf("GUI got an event\n");
	if( event->m_type == UVD_EVENT_FUNCTION_CHANGED )
	{
		const UVDEventFunctionChanged *functionChanged = (const UVDEventFunctionChanged *)event;
		std::string functionName;
	
		uv_assert_ret(functionChanged);
		uv_assert_err_ret(functionChanged->m_function->getFunctionInstance()->getSymbolName(functionName));	

		uv_assert_ret(m_mainWindow);
		if( functionChanged->m_isDefined )
		{
			/*
			eh I think I get it
			emit is thread safe in that you can call funcs with it from other threads
			however, since GUI operations aren't, this doesn't work
				they aren't even reentrant, mutex wouldn't help
			how to queue events?  thought emit would do that for you if in diff thread
			tried emitting local event and connecting signal
			didn't seem signal ever arrived
			*/
			//emit m_mainWindow->newFunction(functionName);
			emit newFunction(QString::fromStdString(functionName));
		}
		else
		{
			//emit m_mainWindow->deleteFunction(functionName);
			emit deleteFunction(QString::fromStdString(functionName));
		}
	}
	return UV_ERR_OK;
}

