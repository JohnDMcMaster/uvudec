/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
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

#include "GUI/analysis_action.h"
#include "GUI/assembly_data.h"
#include "GUI/string_data.h"
#include "GUI/GUI.h"
#include "GUI/lock.h"
#include "GUI/main.h"
#include "GUI/project.h"
#include "GUI/string_data.h"
#include "uvd/core/runtime.h"
#include "uvd/assembly/function.h"
#include "uvd/project/file_extensions.h"
#include "uvd/language/language.h"
#include "uvd/string/engine.h"
#include "uvd/util/debug.h"
#include "uvd/util/io.h"
#include "uvd/util/util.h"
#include "uvqt/dynamic_text_plugin_impl.h"
#include <QtGui>

UVDMainWindow *g_mainWindow = NULL;

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
	g_mainWindow = this;
	m_project = NULL;
	m_assemblyData = NULL;
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
	
	uv_assert_err_ret(m_mainWindow.strings->setDynamicData(new UVDGUIStringData()));
	//This is a core widget type now
	//m_mainWindow.hexdump->setDynamicData(new UVQtDynamicTextDataPluginImpl());
	m_assemblyData = new UVDGUIAssemblyData();
	m_mainWindow.disassembly->setDynamicData(m_assemblyData);

	m_analysisThread = new UVDGUIAnalysisThread();
	m_analysisThread->m_mainWindow = this;
	uv_assert_err_ret(m_analysisThread->init());
	m_analysisThread->m_mainWindow = this;
	
	UVDPrintf("Logging initialized");	

	//uv_assert_err_ret(assemblyDisplayTests());

	/*
	Keep it idly waiting
	Also this will be used to handling printing, so start it early for uniform interface
	*/
	m_analysisThread->start();

	return UV_ERR_OK;
}

/*
uv_err_t UVDMainWindow::assemblyDisplayTests()
{
	appendDisassembledLine("<A href=\"#anchor\">to anchor</A>");
	appendDisassembledLine("some text<BR />on the next line!");
	appendDisassembledLine("some text");
	appendDisassembledLine("some text");
	appendDisassembledLine("some text");
	appendDisassembledLine("some text");
	appendDisassembledLine("some text");
	appendDisassembledLine("");
	appendDisassembledLine("lots of      spacing    ");
	appendDisassembledLine("");
	appendDisassembledLine("1234:          ABC");
	appendDisassembledLine("safd asfd:     123");
	appendDisassembledLine("<A name=\"anchor\">an anchor</A>");
	appendDisassembledLine("some text");
	appendDisassembledLine("          some text");
	appendDisassembledLine("yep       some text");


	//eh BR showed up literally
	appendDisassembledHTML("<BR>");
	appendDisassembledLine("Lots of lines I'm cheating\nFrom a newline<BR>From a BR");


	appendDisassembledHTML("<BR>");
	appendDisassembledHTML("<BR>");
	appendDisassembledHTML("Begin HTML");
	appendDisassembledHTML("<BR>");
	appendDisassembledHTML("some HTML yo");
	appendDisassembledHTML("some HTML yo");
	appendDisassembledHTML("<BR>");
	appendDisassembledHTML("               some HTML yo");
	appendDisassembledHTML("<BR>");
	appendDisassembledHTML("glurb          some HTML yo");

	//Doesn't work
	appendDisassembledHTML("<BR>");
	appendDisassembledHTML("<TT>               some HTML with TT</TT>");
	appendDisassembledHTML("<BR>");
	appendDisassembledHTML("<TT>glurb          some HTML with TT</TT>");

	//Doesn't work
	appendDisassembledHTML("<BR>");
	appendDisassembledHTML("<CODE>               some HTML with CODE</CODE>");
	appendDisassembledHTML("<BR>");
	appendDisassembledHTML("<CODE>glurb          some HTML with CODE</CODE>");

	//Okay this worked but it isn't my selected font
	//What is Qt doing with append that makes it monospaced?
	appendDisassembledHTML("<BR>");
	appendDisassembledHTML("<PRE>               some HTML with PRE</PRE>");
	appendDisassembledHTML("<BR>");
	appendDisassembledHTML("<PRE>glurb          some HTML with PRE</PRE>");

	appendDisassembledHTML("<BR>");
	appendDisassembledHTML("<BR>");
	appendDisassembledHTML("Begin post html append normal");
	appendDisassembledHTML("<BR>");
	appendDisassembledLine("<A href=\"#anchor\">to anchor</A>");
	appendDisassembledLine("some text<BR />on the next line!");
	appendDisassembledLine("some text");
	appendDisassembledLine("some text");
	appendDisassembledLine("some text");
	appendDisassembledLine("some text");
	appendDisassembledLine("some text");
	appendDisassembledLine("");
	appendDisassembledLine("lots of      spacing    ");
	appendDisassembledLine("");
	appendDisassembledLine("1234:          ABC");
	appendDisassembledLine("safd asfd:     123");
	appendDisassembledLine("<A name=\"anchor\">an anchor</A>");
	appendDisassembledLine("some text");
	appendDisassembledLine("          some text");
	appendDisassembledLine("yep       some text");

	return UV_ERR_OK;
}
*/

uv_err_t UVDMainWindow::on_actionNew_triggered()
{
	printf("%s\n", __FUNCTION__);
	
	printf("adding an item\n");
	m_mainWindow.symbolsList->addItem(tr("An item!"));
	return UV_ERR_OK;
}

uv_err_t UVDMainWindow::on_actionOpen_triggered()
{
	/*
	TODO
	Figure out a nice way to distinguish between opening a project file and a binary
	For now, assume if it doesn't end in
	
	Also, we will not do well if we are already initialized
	*/

	QString qFileName;
	std::string fileName;
	std::string projectFileName;
	
	printf("%s\n", __FUNCTION__);

	qFileName = QFileDialog::getOpenFileName(this, tr("Open Project"),
			DEFAULT_DECOMPILE_FILE,
			m_projectFileNameDialogFilter);
	//Blank if user didn't select anything
	if( qFileName.isEmpty() )
	{
		return UV_ERR_OK;
	}

	fileName = qFileName.toStdString();
	if( fileName.find(UVD_EXTENSION_PROJECT) != std::string::npos )
	{
		projectFileName = fileName;
	}
	else
	{
		//Assume a date source then
		uv_assert_ret(g_config);
		g_config->m_targetFileName = fileName;
	}
	
	uv_assert_err_ret(initializeProject(projectFileName));

	return UV_ERR_OK;
}

uv_err_t UVDMainWindow::rebuildFunctionList()
{
	UVDAnalyzer *analyzer = m_project->m_uvd->m_analyzer;

	//Save old selected?
	//uv_assert_err_ret(updateFunctionList());
	m_mainWindow.symbolsList->clear();
	for( std::set<UVDBinaryFunction *>::iterator iter = analyzer->m_functions.begin();
			iter != analyzer->m_functions.end(); ++iter )
	{
		UVDBinaryFunction *binaryFunction = *iter;
		std::string functionName;
		
		uv_assert_ret(binaryFunction);
		uv_assert_err_ret(binaryFunction->getSymbolName(functionName));
		uv_assert_err_ret(newFunction(QString::fromStdString(functionName)));
	}
	return UV_ERR_OK;
}

uv_err_t UVDMainWindow::newFunction(QString functionName)
{
	ASSERT_THREAD();
	//printf("new func\n");
	m_mainWindow.symbolsList->addItem(functionName);
	return UV_ERR_OK;
}

uv_err_t UVDMainWindow::deleteFunction(QString functionName)
{
	ASSERT_THREAD();
	//FIXME
	//m_mainWindow.symbolsList->addItem(QString::fromStdString(functionName));
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
	//uv_assert_ret(QObject::connect(m_analysisThread, SIGNAL(lineDisassembledMonospaced(QString)),
	//		this, SLOT(appendDisassembledLine(QString))));
	//uv_assert_ret(QObject::connect(m_analysisThread, SIGNAL(lineDisassembledHTML(QString)),
	//		this, SLOT(appendDisassembledHTML(QString))));
	uv_assert_ret(QObject::connect(m_analysisThread, SIGNAL(newFunction(QString)),
			this, SLOT(newFunction(QString))));
	uv_assert_ret(QObject::connect(m_analysisThread, SIGNAL(deleteFunction(QString)),
			this, SLOT(deleteFunction(QString))));
	uv_assert_ret(QObject::connect(m_analysisThread, SIGNAL(printLog(QString)),
			this, SLOT(appendLogLine(QString))));
	//uv_assert_ret(QObject::connect(m_analysisThread, SIGNAL(setDisassemblyAreaActive(bool)),
	//		m_mainWindow.disassemblyArea, SLOT(setVisible(bool))));

	uv_assert_ret(QObject::connect(m_analysisThread, SIGNAL(binaryStateChanged()),
			this, SLOT(updateBinaryView())));
	uv_assert_ret(QObject::connect(m_analysisThread, SIGNAL(stringsChanged()),
			this, SLOT(updateStringsView())));
	uv_assert_ret(QObject::connect(m_analysisThread, SIGNAL(binaryStateChanged()),
			this, SLOT(updateAssemblyView())));

	m_analysisThread->queueAnalysis(new UVDAnalysisActionBegin());

	return UV_ERR_OK;
}

/*
uv_err_t UVDMainWindow::appendDisassembledLine(QString line)
{
	ASSERT_THREAD();
	//QString qLine = QString::fromStdString(line);
	//m_mainWindow.disassemblyArea->appendPlainText(line);
	//aparantly bold messed up the monospacing
	//line = "<B>" + line + "</B>";
	//printf("appending: %s\n", line.toStdString().c_str());
	m_mainWindow.disassemblyArea->append(line);
	//Test to see if makes GUI more responsive
	//nope, just took longer and less CPU used
	//usleep(1000);
	return UV_ERR_OK;
}
*/

/*
uv_err_t UVDMainWindow::appendDisassembledHTML(QString html)
{
	ASSERT_THREAD();
	//printf("appending HTML of size %d\n", html.size());
	//FIXME: make this actually append and not actually insert at cursor
	m_mainWindow.disassemblyArea->insertHtml(html);
	return UV_ERR_OK;
}
*/

uv_err_t UVDMainWindow::appendLogLine(QString line)
{
	ASSERT_THREAD();
	m_mainWindow.plainTextEdit_log->appendPlainText(line);
	return UV_ERR_OK;
}

UVDData *UVDMainWindow::getObjectData()
{
	//To prevent various crashes
	if( m_project == NULL
			|| m_project->m_uvd == NULL 
			|| m_project->m_uvd->m_runtime == NULL
			|| m_project->m_uvd->m_runtime->m_object == NULL )
	{
		return NULL;
	}

	return m_project->m_uvd->m_runtime->m_object->m_data;
}

uv_err_t UVDMainWindow::updateBinaryView()
{
	return UV_DEBUG(m_mainWindow.hexdump->setData(getObjectData()));
}

uv_err_t UVDMainWindow::updateAssemblyView()
{
printf("\n\nUVDMainWindow::updateAssemblyView()\n");
	uv_assert_err_ret(m_assemblyData->begin(0, 0, &m_mainWindow.disassembly->m_viewportShadow->m_start));
	m_mainWindow.disassembly->refreshDynamicData();
	return UV_ERR_OK;
}

uv_err_t UVDMainWindow::updateStringsView()
{
printf("\n\nUVDMainWindow::updateStringsView()\n");
	m_mainWindow.strings->refreshDynamicData();
	return UV_ERR_OK;

#if 0
	UVDStringEngine *stringEngine = NULL;
	
	UVD_AUTOLOCK_ENGINE_BEGIN();
	
	stringEngine = m_project->m_uvd->m_analyzer->m_stringEngine;

	for( std::vector<UVDString>::iterator iter = stringEngine->m_strings.begin();
			iter != stringEngine->m_strings.end(); ++iter )
	{
		UVDString uvdString = *iter;
		std::string string;
		
		//Read a string
		uv_assert_err_ret(uvdString.readString(string));

		m_indexBuffer.push_back(UVDSprintf("# 0x%.8X: %s", uvdString.m_addressRange.m_min_addr, stringTableStringFormat(lines[0]).c_str()));				
	}
	
	UVD_AUTOLOCK_ENGINE_END();
#endif
	return UV_ERR_OK;
}

uv_err_t UVDMainWindow::on_actionSave_triggered()
{
	printf("%s\n", __FUNCTION__);

	//TODO: gray out so we can't do this
	uv_assert_ret(m_project);

	if( m_project->m_canonicalProjectFileName.empty() )
	{
		on_actionSaveAs_triggered();
	}
	else
	{
		UV_DEBUG(m_project->doSave());
	}
	return UV_ERR_OK;
}

uv_err_t UVDMainWindow::on_actionSaveAs_triggered()
{
	QString fileName;

	printf("%s\n", __FUNCTION__);

	uv_assert_ret(m_project);

	//file:///opt/qtsdk-2010.04/qt/doc/html/tutorials-addressbook-part6.html
	fileName = QFileDialog::getSaveFileName(this,
			tr("Save project file"), "",
			m_projectFileNameDialogFilter);
	printf("Save fileName: %s\n", fileName.toStdString().c_str());
	
	m_project->setFileName(fileName.toStdString());

	UV_DEBUG(m_project->doSave());
	return UV_ERR_OK;
}

uv_err_t UVDMainWindow::on_actionPrint_triggered()
{
	printf("%s\n", __FUNCTION__);
	return UV_ERR_OK;
}

uv_err_t UVDMainWindow::on_symbolsList_itemClicked(QListWidgetItem *item)
{
	std::string text;
	uint32_t address;
	UVDBinarySymbol *symbol = NULL;
	uv_err_t rc_tmp;
	
	text = item->text().toStdString();
	
	//Text holds symbol name
	UVDPrintf("clicked on %s", text.c_str());
	rc_tmp = m_project->m_uvd->m_analyzer->m_symbolManager.findSymbol(text.c_str(), &symbol);
	printf("rc: %d\n", rc_tmp);
	uv_assert_err_ret(rc_tmp);
	uv_assert_err_ret(symbol->m_symbolAddress.getDynamicValue(&address));
	printf("address: 0x%04X, pointer: %p\n",  address, symbol);
	uv_assert_err_ret(m_mainWindow.disassembly->setPosition(address, 0));
	
	return UV_ERR_OK;
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

uv_err_t UVDMainWindow::on_actionClose_triggered()
{
	printf("%s\n", __FUNCTION__);
	
	UV_DEBUG(shutdown());	
	g_application->quit();
	printf("close done\n");
	return UV_ERR_OK;
}

uv_err_t UVDMainWindow::on_actionAbout_triggered()
{
	printf("%s\n", __FUNCTION__);

	QMessageBox::about(this, tr("About uvudec GUI"),
			tr("UVNet Universal Decompiler GUI\n"
			"Copyright 2010 John McMaster\n"
			"Licensed under the terms of the GPL V3+\n"
			"Thanks to Sean O'Sullivan through the Rensselaer Center For Open Source (RCOS) for supporting this project"
			));
	return UV_ERR_OK;
}

void UVDMainWindow::closeEvent(QCloseEvent *event)
{
	//Triggered on a window close event, such as the x being hit
	printf("Close event\n");
	UV_DEBUG(shutdown());
}

