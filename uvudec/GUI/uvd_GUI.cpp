/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#include <QtGui>

#include "uvd_GUI.h"

UVDMainWindow::UVDMainWindow(QMainWindow *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
}

void UVDMainWindow::on_actionNew_triggered()
{
	printf("%s\n", __FUNCTION__);
}

void UVDMainWindow::on_actionOpen_triggered()
{
	printf("%s\n", __FUNCTION__);
}

void UVDMainWindow::on_actionSave_triggered()
{
	printf("%s\n", __FUNCTION__);

	//file:///opt/qtsdk-2010.04/qt/doc/html/tutorials-addressbook-part6.html
	QString fileName = QFileDialog::getSaveFileName(this,
			tr("Save project file"), "",
			tr("uvudec oject (*.upj);;All Files (*)"));
	printf("Save fileName: %s\n", fileName.toStdString().c_str());
}

void UVDMainWindow::on_actionSaveAs_triggered()
{
	printf("%s\n", __FUNCTION__);
}

void UVDMainWindow::on_actionPrint_triggered()
{
	printf("%s\n", __FUNCTION__);
}

void UVDMainWindow::on_actionClose_triggered()
{
	printf("%s\n", __FUNCTION__);
}



