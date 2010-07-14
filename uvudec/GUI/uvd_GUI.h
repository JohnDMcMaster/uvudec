/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#ifndef UVD_GUI_H
#define UVD_GUI_H

#include "ui_uvudec.h"

class UVDMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	UVDMainWindow(QMainWindow *parent = 0);

private slots:
	void on_actionNew_triggered();
	void on_actionOpen_triggered();
	void on_actionSave_triggered();
	void on_actionSaveAs_triggered();
	void on_actionPrint_triggered();
	void on_actionClose_triggered();

private:
	Ui::UVDMainWindow ui;
};

#endif

