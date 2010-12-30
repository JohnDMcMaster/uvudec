#include <QtGui>
#include "uvqt/dynamic_text_plugin_impl.h"
#include <stdio.h>
#include "uvd/core/init.h"
#include "uvd/util/debug.h"

int main(int argc, char **argv)
{
	QApplication app(argc, argv);
	int ret = 0;
	UVQtScrollableDynamicText *widget = NULL;
	
	UVDInit();
	UVDSetDebugFlag(UVD_DEBUG_TYPE_ALL, true);

	printf("plugin scrolling widget ex\n");
	widget = new UVQtScrollableDynamicText(new UVQtDynamicTextDataPluginImpl());
	//widget->setData(new UVQtDynamicTextDataPluginImpl());

	widget->resize(1024, 480);
	widget->show();
	
	printf("Exec'ing...\n");
//exit(1);
	ret = app.exec();
	//delete widget;
	printf("Done\n");
	return ret;
}

