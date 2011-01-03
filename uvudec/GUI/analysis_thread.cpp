/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/assembly/function.h"
#include "uvd/util/benchmark.h"
#include "uvd/util/io.h"
#include "uvd/language/language.h"
#include "uvd/core/event.h"
#include "uvd/event/event.h"
#include "uvd/event/events.h"
#include "uvd/event/engine.h"
#include "uvd/util/debug.h"
#include "GUI/GUI.h"
#include "GUI/analysis_thread.h"
#include "GUI/analysis_action.h"
#include "GUI/format.h"
#include "GUI/lock.h"
#include <typeinfo>

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
	printf("GUI analysis thread request recieved, aquiring lock\n");
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
	
	//Begin engine operations, lock engine
	UVD_AUTOLOCK_ENGINE_BEGIN();

	//Create a runTasksr engine active on that input
	printf_debug_level(UVD_DEBUG_SUMMARY, "runTasks: initializing engine...\n");
	uv_assert_err_ret(UVD::getUVDFromData(&uvd, data));
	uv_assert_ret(uvd);
	uv_assert_ret(g_uvd);
	m_mainWindow->m_project->m_uvd = uvd;
	emit binaryStateChanged();

	//Get our callbacks ready...
	uv_assert_err_ret(initializeUVDCallbacks());
	UVDFormat *format = new UVDGUIFormat();
	uv_assert_ret(format);
	uv_assert_err_ret(uvd->setOutputFormatting(format));

	//Fire at will
	UVDPrintf("Analyzing");
	uv_assert_err_ret(uvd->analyze());

	uv_assert_err_ret(uvd->setDestinationLanguage(UVD_LANGUAGE_ASSEMBLY));

	UVDPrintf("Disassembling");
	uv_assert_err_ret(disassembleRange(uvd->begin(), uvd->end()));

	UVDPrintf("Initial analysis completed");
	
	UVD_AUTOLOCK_ENGINE_END();

	return UV_ERR_OK;
}

uv_err_t UVDGUIAnalysisThread::disassembleRange(UVDIterator iterBegin, UVDIterator iterEnd)
{
	UVDIterator iter;
	//UVDIterator iterEnd;
	UVDBenchmark disassemblyTime;
	std::string everything;
	
	disassemblyTime.start();

	iter = iterBegin;

	//FIXME: what if we misalign by accident and surpass?
	//need to add some check for that
	//maybe we should implement <
	//emit setDisassemblyAreaActive(false);
	//printf("iterBegin: 0x%08X, iterEnd: 0x%08X\n", iterBegin.m_nextPosition, iterEnd.m_nextPosition);
	while( iter != iterEnd )
	{
		char buff[256];
		std::string lineRaw;
		std::string lineDone;
		uint32_t maxOpcodeBytes = 4;
		std::string anchorName;
		std::string lineAnchor;

		/*
		//For debugging
		static int count = 0;
		++count;
		if( count >= 10 )
		{
		break;
		}
		*/

		uv_assert_err_ret(iter.getCurrent(lineRaw));
		
		uv_assert_err_ret(m_mainWindow->m_project->getFormat()->addressToAnchorName(iter.getPosition(), anchorName));
		lineAnchor = "<A name=\"" + anchorName + "\" />\n";
		everything += lineAnchor;

		//opcode bytes
		//printf("iter.m_instruction.m_inst_size: %d\n", iter.m_instruction.m_inst_size);
		//fflush(stdout);
		std::string formattedAddress;
		//We should probably not format this as having a link to the address at the address isn't terribly useful
		uv_assert_err_ret(m_mainWindow->m_project->m_uvd->m_format->formatAddress(iter.getPosition(), formattedAddress));
		lineDone += formattedAddress;
		//printf("line done w/ address: %s\n", lineDone.c_str());
		lineDone += "  ";
		for( uint32_t i = 0; i < iter.m_instruction->m_inst_size; ++i )
		{
			snprintf(buff, sizeof(buff), "%02X", (unsigned int)(unsigned char)iter.m_instruction->m_inst[i]);
			lineDone += buff;
			--maxOpcodeBytes;
		}
		while( maxOpcodeBytes )
		{
			//lineDone += "  ";
			lineDone += "&nbsp;&nbsp;";
			--maxOpcodeBytes;
		}
		
		//lineDone += ": ";
		//lineDone += "  ";
		lineDone += "&nbsp;&nbsp;";
		lineDone += lineRaw;
		//Monospaced
		//See how much this helps w/ and w/o monospaced font
		//lineDone = std::string("<TT>") + lineDone + "</TT>";
		
		//emit lineDisassembled(QString::fromStdString(lineDone));
		everything += lineDone + "\n<BR />\n";

		if( UV_FAILED(iter.next()) )
		{
			printf_debug("Failed to get next\n");
			return UV_DEBUG(UV_ERR_GENERAL);
		}
	}
	//printf("emitting HTML of size %d\n%s\n", everything.size(), everything.c_str());
	//emit lineDisassembledMonospaced(QString::fromStdString(everything));
	emit lineDisassembledHTML(QString::fromStdString(everything));
	disassemblyTime.stop();
	printf_debug_level(UVD_DEBUG_PASSES, "disassembly time: %s\n", disassemblyTime.toString().c_str());
	//emit setDisassemblyAreaActive(true);
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
		uv_assert_err_ret(functionChanged->m_function->getSymbolName(functionName));	

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

