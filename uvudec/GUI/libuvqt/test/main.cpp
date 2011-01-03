/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include <QtGui>
#include "uvqt/dynamic_text_plugin_impl.h"
#include <stdio.h>
#include "uvd/core/init.h"
#include "uvd/util/debug.h"
#include "uvqt/plain_text_edit.h"
#include "uvqt/hexdump.h"
#include "uvqt/hexdump_plugin.h"

int UVQtScrollableDynamicTextMain(int argc, char **argv)
{
	QApplication app(argc, argv);
	int ret = 0;
	UVQtScrollableDynamicText *widget = NULL;
	
	printf("plugin scrolling widget ex\n");
	widget = new UVQtScrollableDynamicText(new UVQtDynamicTextDataPluginImpl());

	widget->resize(1024, 480);
	widget->show();
	
	printf("Exec'ing...\n");
	ret = app.exec();
	//delete widget;
	printf("Done\n");
	return ret;
}

int UVQtHexdumpMain(int argc, char **argv)
{
	QApplication app(argc, argv);
	int ret = 0;
	QWidget *widget = NULL;

	printf("hexdump widget ex\n");
	
	UVQtHexdumpPlugin plugin;
	
	//plugin.initialize();
	widget = plugin.createWidget(NULL);
	
	widget->resize(1024, 480);
	widget->show();
	
	printf("Exec'ing...\n");
	ret = app.exec();
	//delete widget;
	printf("Done\n");
	return ret;
}

int QPlainTextEditMain(int argc, char **argv)
{
	QApplication app(argc, argv);
	int ret = 0;
	UVQtPlainTextEdit *widget = NULL;
	
	printf("QPlainTextEdit custom document widget ex\n");
	widget = new UVQtPlainTextEdit();

	widget->resize(1024, 480);
	widget->show();
	
	printf("Exec'ing...\n");
	ret = app.exec();
	//delete widget;
	printf("Done\n");
	return ret;
}

int QTextEditMain(int argc, char **argv)
{
	QApplication app(argc, argv);
	int ret = 0;
	UVQtTextEdit *widget = NULL;
	
	printf("QTextEdit custom document widget ex\n");
	widget = new UVQtTextEdit();

	widget->resize(1024, 480);
	widget->show();
	
	printf("Exec'ing...\n");
	printf("widget document: 0x%08X\n", (int)widget->document());

	ret = app.exec();
	//delete widget;
	printf("Done\n");
	return ret;
}

int labelMain(int argc, char **argv)
{
	//Labels are not selectable

	QApplication app(argc, argv);
	int ret = 0;
	QLabel *widget = NULL;
	
	widget = new QLabel("Some text");

	//widget->resize(1024, 480);
	widget->show();
	
	printf("Exec'ing...\n");

	ret = app.exec();
	//delete widget;
	printf("Done\n");
	return ret;
}

int main(int argc, char **argv)
{
	UVDInit();
	UVDSetDebugFlag(UVD_DEBUG_TYPE_ALL, true);

	//return UVQtScrollableDynamicTextMain(argc, argv);
	//return QPlainTextEditMain(argc, argv);
	//return QTextEditMain(argc, argv);
	//return labelMain(argc, argv);
	return UVQtHexdumpMain(argc, argv);
}

