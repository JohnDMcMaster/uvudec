/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_GUI_H
#define UVD_GUI_H

#include "ui_uvudec.h"
#include "uvd_GUI_analysis_thread.h"
#include "uvd_project.h"
#include "uvd_error.h"
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

public slots:
	uv_err_t newFunction(QString functionName);
	uv_err_t deleteFunction(QString functionName);
	void appendDisassembledLine(QString line);
	void appendLogLine(QString line);

private slots:
	void on_actionNew_triggered();
	void on_actionOpen_triggered();
	void on_actionSave_triggered();
	void on_actionSaveAs_triggered();
	void on_actionPrint_triggered();
	void on_actionClose_triggered();
	void on_actionAbout_triggered();

	void on_symbolsListWidget_itemClicked(QListWidgetItem *item);

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

#endif

