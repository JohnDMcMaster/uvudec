/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the GPL V3 or later, see COPYING for details
*/

#ifndef UVD_GUI_ANALYSIS_THREAD_H
#define UVD_GUI_ANALYSIS_THREAD_H

#include "GUI/uvudec.ui.h"
#include <QThread>
#include <QMutex>
#include <QObject>
#include "uvd/util/types.h"
#include "uvd/core/uvd.h"

class UVDMainWindow;
class UVDAnalysisAction;
class UVDEvent;
class UVDGUIAnalysisThread : public QThread
{
	Q_OBJECT

public:
	UVDGUIAnalysisThread();
	~UVDGUIAnalysisThread();
	uv_err_t init();

	void run();
	uv_err_t runLoop();
	//Will be free'd internally
	void queueAnalysis(UVDAnalysisAction *action);

	uv_err_t beginAnalysis();
	uv_err_t disassembleRange(UVDIterator iterBegin, UVDIterator iterEnd);

	uv_err_t initializeUVDCallbacks();
	uv_err_t handleUVDEvent(const UVDEvent *event);

	uv_err_t printLogEntry(const std::string &line);

signals:
	//We finished disassembling the next line, line is the result
	void lineDisassembledMonospaced(QString name);
	void lineDisassembledHTML(QString name);
	void newFunction(QString functionName);
	void deleteFunction(QString functionName);
	void printLog(QString line);
	void setDisassemblyAreaActive(bool);
	void binaryStateChanged();

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

