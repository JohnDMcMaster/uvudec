/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#include <QtGui>

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
			"",
			m_projectFileNameDialogFilter);
	UV_DEBUG(initializeProject(fileName.toStdString()));
	UV_DEBUG(beginAnalysis());
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

	uv_assert_err_ret(uvd->analyze());
	
	delete data;

	return UV_ERR_OK;
}

uv_err_t UVDMainWindow::initializeProject(const std::string fileName)
{
	m_project = new UVDProject();
	uv_assert_ret(m_project);

	uv_assert_err_ret(m_project->setFileName(fileName));
	uv_assert_err_ret(m_project->init(m_argc, m_argv));

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
	
	delete m_project;
	m_project = NULL;
}

void UVDMainWindow::on_actionAbout_triggered()
{
	printf("%s\n", __FUNCTION__);

	QMessageBox::about(this, tr("About uvudec GUI"),
			tr("The <b>Application</b> example demonstrates how to "
			"write modern GUI applications using Qt, with a menu bar, "
			"toolbars, and a status bar."));
}

