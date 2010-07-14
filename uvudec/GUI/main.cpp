/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under terms of the three clause BSD license, see LICENSE for details
*/

#include <QApplication>
#include "uvd_GUI.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	UVDMainWindow win;
	win.show();
	return app.exec();
}

