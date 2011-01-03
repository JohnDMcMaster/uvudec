/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_GUI_H
#define UVD_GUI_H

#include "uvd/util/error.h"
#include "GUI/uvudec.ui.h"
#include "GUI/analysis_thread.h"
#include "GUI/project.h"
#include <string>

class UVDEvent;
class UVDMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	UVDMainWindow(QMainWindow *parent = 0);
	~UVDMainWindow();
	uv_err_t init();

	uv_err_t initializeProject(const std::string fileName);
	uv_err_t updateAllViews();

	void closeEvent(QCloseEvent *event);

	uv_err_t shutdown();

protected:
	uv_err_t rebuildFunctionList();
	uv_err_t assemblyDisplayTests();
	UVDData *getObjectData();

public slots:
	uv_err_t newFunction(QString functionName);
	uv_err_t deleteFunction(QString functionName);
	//Inserts a newline before the current text if the text area is not empty
	//uv_err_t appendDisassembledLine(QString line);
	//Don't think this inserts a newline
	//uv_err_t appendDisassembledHTML(QString html);
	uv_err_t appendLogLine(QString line);
	/*
	Indicates we have changed the status of (a?) binary
	May be now present, values changes, or no longer present
	*/
	uv_err_t updateBinaryView();

private slots:
	uv_err_t on_actionNew_triggered();
	uv_err_t on_actionOpen_triggered();
	uv_err_t on_actionSave_triggered();
	uv_err_t on_actionSaveAs_triggered();
	uv_err_t on_actionPrint_triggered();
	uv_err_t on_actionClose_triggered();
	uv_err_t on_actionAbout_triggered();

	uv_err_t on_symbolsList_itemClicked(QListWidgetItem *item);

public:
	Ui::UVDMainWindow m_mainWindow;
	UVDProject *m_project;
	QString m_projectFileNameDialogFilter;
	
	//Need to add some sort of thread safe queue object
	UVDGUIAnalysisThread *m_analysisThread;
	
	//So we can pass options off to children later
	int m_argc;
	char **m_argv;
};

extern UVDMainWindow *g_mainWindow;

#endif

