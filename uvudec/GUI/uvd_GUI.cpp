/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include <QtGui>

#include "uvd_analysis_action.h"
#include "uvd_GUI.h"
#include "uvd_project.h"

UVDMainWindow::UVDMainWindow(QMainWindow *parent)
	: QMainWindow(parent)
{
	m_project = NULL;
	m_argc = 0;
	m_argv = NULL;

	m_mainWindow.setupUi(this);
}

uv_err_t UVDMainWindow::init()
{
	m_projectFileNameDialogFilter = tr("uvudec oject (*.upj);;All Files (*)");
	
	m_analysisThread.m_mainWindow = this;
	uv_assert_err_ret(m_analysisThread.init());
	
	return UV_ERR_OK;
}

void UVDMainWindow::on_actionNew_triggered()
{
	printf("%s\n", __FUNCTION__);
	
	printf("adding an item\n");
	m_mainWindow.symbolsListWidget->addItem(tr("An item!"));
}

void UVDMainWindow::on_actionOpen_triggered()
{
	QString fileName;
	
	printf("%s\n", __FUNCTION__);

	fileName = QFileDialog::getOpenFileName(this, tr("Open Project"),
			DEFAULT_DECOMPILE_FILE,
			m_projectFileNameDialogFilter);
	UV_DEBUG(initializeProject(fileName.toStdString()));
}

uv_err_t UVDMainWindow::beginAnalysis()
{
	std::string output;
	UVD *uvd = NULL;
	UVDData *data = NULL;

	printf_debug_level(UVD_DEBUG_PASSES, "main: initializing data streams\n");

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
	m_project->m_uvd = uvd;

	uv_assert_err_ret(uvd->analyze());
	uv_assert_err_ret(updateAllViews());	
	
	delete data;

	return UV_ERR_OK;
}

uv_err_t UVDMainWindow::updateAllViews()
{
	UVD *uvd = NULL;
	UVDAnalyzer *analyzer = NULL;

	uv_assert_ret(m_project);
	uvd = m_project->m_uvd;
	uv_assert_ret(uvd);
	analyzer = uvd->m_analyzer;
	uv_assert_ret(analyzer);

	//Save old selected?
	//uv_assert_err_ret(updateFunctionList());
	m_mainWindow.symbolsListWidget->clear();
	for( std::set<UVDBinaryFunction *>::iterator iter = analyzer->m_functions.begin();
			iter != analyzer->m_functions.end(); ++iter )
	{
		UVDBinaryFunction *binaryFunction = *iter;
		std::string functionName;
		
		uv_assert_ret(binaryFunction);
		uv_assert_err_ret(binaryFunction->getFunctionInstance()->getSymbolName(functionName));
		m_mainWindow.symbolsListWidget->addItem(QString::fromStdString(functionName));
	}
	
	//uv_assert_err_ret(updateDisassemblyView());

	return UV_ERR_OK;
}

uv_err_t UVDMainWindow::initializeProject(const std::string fileName)
{
	m_project = new UVDProject();
	uv_assert_ret(m_project);

	uv_assert_err_ret(m_project->setFileName(fileName));
	uv_assert_err_ret(m_project->init(m_argc, m_argv));
	m_analysisThread.start();
	m_analysisThread.queueAnalysis(new UVDAnalysisActionBegin());

	return UV_ERR_OK;
}

void UVDMainWindow::on_actionSave_triggered()
{
	printf("%s\n", __FUNCTION__);

	//TODO: gray out so we can't do this
	if( !m_project )
	{
		return;
	}

	if( m_project->m_canonicalProjectFileName.empty() )
	{
		on_actionSaveAs_triggered();
	}
	else
	{
		UV_DEBUG(m_project->doSave());
	}
}

void UVDMainWindow::on_actionSaveAs_triggered()
{
	QString fileName;

	printf("%s\n", __FUNCTION__);

	if( !m_project )
	{
		return;
	}

	//file:///opt/qtsdk-2010.04/qt/doc/html/tutorials-addressbook-part6.html
	fileName = QFileDialog::getSaveFileName(this,
			tr("Save project file"), "",
			m_projectFileNameDialogFilter);
	printf("Save fileName: %s\n", fileName.toStdString().c_str());
	
	m_project->setFileName(fileName.toStdString());

	UV_DEBUG(m_project->doSave());
}

void UVDMainWindow::on_actionPrint_triggered()
{
	printf("%s\n", __FUNCTION__);
}

void UVDMainWindow::on_actionClose_triggered()
{
	printf("%s\n", __FUNCTION__);
	
	m_analysisThread.m_active = FALSE;
	m_analysisThread.wait(100);
	if( !m_analysisThread.isFinished() )
	{
		printf_error("Analysis thread is still running, getting it before it gets away\n");
		m_analysisThread.terminate();
	}
	
	delete m_project;
	m_project = NULL;
}

void UVDMainWindow::on_actionAbout_triggered()
{
	printf("%s\n", __FUNCTION__);

	QMessageBox::about(this, tr("About uvudec GUI"),
			tr("UVNet Universal Decompiler GUI\n"
			"Copyright 2010 John McMaster\n"
			"Licensed under the terms of the GPL V3+ or, at your option, a later version"
			));
}

