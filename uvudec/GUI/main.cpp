/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include <QApplication>
#include "GUI/GUI.h"
#include "uvd/core/init.h"

QApplication *g_application = NULL;

uv_err_t uvmain(int argc, char **argv, int *retOut)
{
	UVDMainWindow *mainWindow = NULL;

	g_application = new QApplication(argc, argv);
	uv_assert_ret(g_application);
	mainWindow = new UVDMainWindow();
	uv_assert_ret(mainWindow);
	mainWindow->m_argc = argc;
	mainWindow->m_argv = argv;

	uv_assert_err_ret(UVDInit());
	uv_assert_err_ret(mainWindow->init());
	
	mainWindow->show();
	*retOut = g_application->exec();
	
	delete mainWindow;
	delete g_application;
	
	return UV_ERR_OK;
}

int main(int argc, char **argv)
{
	int ret = 1;

	UV_DEBUG(uvmain(argc, argv, &ret));
	return ret;
}

