/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the GPL V3 or later, see COPYING for details
*/

/*
Using threads safely in Qt
http://doc.qt.nokia.com/4.6/threads.html
...cause guess and check wasn't cutting it with QTextEdit


http://doc.qt.nokia.com/4.6/qcoreapplication.html#postEvent
Posting events cross thread

http://doc.qt.nokia.com/4.6/threads-qobject.html
On the other hand, you can safely emit signals from your QThread::run() implementation, because signal emission is thread-safe.

*/

#include <QtGui>

#include "uvd_analysis_action.h"
#include "uvd_GUI.h"
#include "uvd_project.h"
#include "uvd_language.h"
#include "main.h"
#include "util/io.h"
#include "util/uvd_util.h"
#include "uvd_debug.h"

//#define ROLE_FUNCTION_LIST_FUNCTION		(Qt::UserRole + 0)

#define ASSERT_THREAD() \
	if( QThread::currentThread() == m_analysisThread ) \
	{ \
		printf_error("GUI operations in non-main thread\n"); \
		UVD_PRINT_STACK(); \
		UVD_BREAK(); \
		exit(1); \
	}


UVDMainWindow::UVDMainWindow(QMainWindow *parent)
	: QMainWindow(parent)
{
	m_project = NULL;
	m_argc = 0;
	m_argv = NULL;

	m_mainWindow.setupUi(this);
}

UVDMainWindow::~UVDMainWindow()
{
	printf("Shutting down main window\n");
}

uv_err_t UVDMainWindow::init()
{
	//printf("mainwindow init, this: 0x%08X\n", this);

	m_projectFileNameDialogFilter = tr("uvudec oject (*.upj);;All Files (*)");

	//FIXME: this looks wrong, but its the best I could figure out from the error messages
	//Maybe related to multithreading on QTextBlocks
	//qRegisterMetaType("QTextBlock");
	//qRegisterMetaType("QTextCursor");
	
	//FIXME: this is an ugly hack for preventing some crashes until I figure how to fix them properly
	//the original issue of closing properly should be fixed now, I should be able to fix this now
	m_analysisThread = new UVDGUIAnalysisThread();
	m_analysisThread->m_mainWindow = this;
	uv_assert_err_ret(m_analysisThread->init());
	m_analysisThread->m_mainWindow = this;
	
	UVDPrintf("Logging initialized");	

	/*
	Keep it idlying waiting
	Also this will be used to handling printing, so start it early for uniform interface
	*/
	m_analysisThread->start();

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
	if( !fileName.isEmpty() )
	{
		UV_DEBUG(initializeProject(fileName.toStdString()));
	}
}

uv_err_t UVDMainWindow::rebuildFunctionList()
{
	UVDAnalyzer *analyzer = m_project->m_uvd->m_analyzer;

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
		uv_assert_err_ret(newFunction(QString::fromStdString(functionName)));
	}
	return UV_ERR_OK;
}

uv_err_t UVDMainWindow::newFunction(QString functionName)
{
	ASSERT_THREAD();
	//printf("new func\n");
	m_mainWindow.symbolsListWidget->addItem(functionName);
	return UV_ERR_OK;
}

uv_err_t UVDMainWindow::deleteFunction(QString functionName)
{
	ASSERT_THREAD();
	//FIXME
	//m_mainWindow.symbolsListWidget->addItem(QString::fromStdString(functionName));
	return UV_ERR_OK;
}

uv_err_t UVDMainWindow::updateAllViews()
{
	/*
	UVD *uvd = NULL;
	UVDAnalyzer *analyzer = NULL;

	uv_assert_ret(m_project);
	uvd = m_project->m_uvd;
	uv_assert_ret(uvd);
	analyzer = uvd->m_analyzer;
	uv_assert_ret(analyzer);
	*/
	
	//uv_assert_err_ret(rebuildFunctionList());
	//uv_assert_err_ret(updateDisassemblyView());

	return UV_ERR_OK;
}

uv_err_t UVDMainWindow::initializeProject(const std::string fileName)
{
	UVDPrintf("Opening file: %s", fileName.c_str());	

	m_project = new UVDProject();

	uv_assert_ret(m_project);
	uv_assert_err_ret(m_project->setFileName(fileName));
	uv_assert_err_ret(m_project->init(m_argc, m_argv));
	//hmm not working
	uv_assert_ret(QObject::connect(m_analysisThread, SIGNAL(lineDisassembled(QString)),
			this, SLOT(appendDisassembledLine(QString))));
	uv_assert_ret(QObject::connect(m_analysisThread, SIGNAL(newFunction(QString)),
			this, SLOT(newFunction(QString))));
	uv_assert_ret(QObject::connect(m_analysisThread, SIGNAL(deleteFunction(QString)),
			this, SLOT(deleteFunction(QString))));
	uv_assert_ret(QObject::connect(m_analysisThread, SIGNAL(printLog(QString)),
			this, SLOT(appendLogLine(QString))));
	m_analysisThread->queueAnalysis(new UVDAnalysisActionBegin());

	return UV_ERR_OK;
}

void UVDMainWindow::appendDisassembledLine(QString line)
{
//printf("appending line\n");
	ASSERT_THREAD();
	//QString qLine = QString::fromStdString(line);
	m_mainWindow.disassemblyArea->appendPlainText(line);
}

void UVDMainWindow::appendLogLine(QString line)
{
printf_debug("");
	ASSERT_THREAD();
	m_mainWindow.plainTextEdit_log->appendPlainText(line);
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

void UVDMainWindow::on_symbolsListWidget_itemClicked(QListWidgetItem *item)
{
	std::string text;
	
	text = item->text().toStdString();
	
	UVDPrintf("clicked on %s", text.c_str());
}

uv_err_t UVDMainWindow::shutdown()
{
	m_analysisThread->m_active = FALSE;
	for( uint32_t i = 0; i < 100; ++i )
	{
		if( !m_analysisThread->isRunning() )
		{
			break;
		}
		m_analysisThread->wait(10);
	}
	if( m_analysisThread->isRunning() )
	{
		printf_error("forcing termination of analysis thread\n");
		m_analysisThread->terminate();
	}
	
	delete m_project;
	m_project = NULL;

	return UV_ERR_OK;
}

void UVDMainWindow::on_actionClose_triggered()
{
	printf("%s\n", __FUNCTION__);
	
	UV_DEBUG(shutdown());	
	g_application->quit();
	printf("close done\n");
}

void UVDMainWindow::on_actionAbout_triggered()
{
	printf("%s\n", __FUNCTION__);

	QMessageBox::about(this, tr("About uvudec GUI"),
			tr("UVNet Universal Decompiler GUI\n"
			"Copyright 2010 John McMaster\n"
			"Licensed under the terms of the GPL V3+\n"
			"Thanks to Sean O'Sullivan through the Rensselaer Center For Open Source (RCOS) for supporting this project"
			));
}

void UVDMainWindow::closeEvent(QCloseEvent *event)
{
	//Triggered on a window close event, such as the x being hit
	printf("Close event\n");
	UV_DEBUG(shutdown());
}

