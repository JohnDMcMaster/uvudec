/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvqt/hexdump.h"
#include "uvqt/hexdump_plugin.h"

UVQtHexdumpPlugin::UVQtHexdumpPlugin(QObject *parent)
    : QObject(parent)
{
	//Test to see if we really were linking against uvudec
	//consider breaking out utilities at some point
	//std::vector<std::string> the_split = UVDSplit("blah blah blah", ' ', true);
	//printf("split items: %d\n", the_split.size());

    initialized = false;
}

void UVQtHexdumpPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
    if (initialized)
        return;

    initialized = true;
}

bool UVQtHexdumpPlugin::isInitialized() const
{
    return initialized;
}

//static const char *g_dataRaw = "***Rensselaer Center for Open Source Software***";
static const char *g_dataRaw = "***Rensselaer Center for Open Source Software***It is not the critic who counts; not the man who points out how the strong man stumbles, or where the doer of deeds could have done them better. The credit belongs to the man who is actually in the arena, whose face is marred by dust and sweat and blood; who strives valiantly; who errs, who comes short again and again, because there is no effort without error and shortcoming; but who does actually strive to do the deeds; who knows great enthusiasms, the great devotions; who spends himself in a worthy cause; who at the best knows in the end the triumph of high achievement, and who at the worst, if he fails, at least fails while daring greatly, so that his place shall never be with those cold and timid souls who neither know victory nor defeat.";

QWidget *UVQtHexdumpPlugin::createWidget(QWidget *parent)
{
	UVQtHexdump *ret = NULL;
	UVDDataMemory *data = NULL;
    ret = new UVQtHexdump(parent);
	UV_DEBUG(UVDDataMemory::getUVDDataMemoryByTransfer(&data, (char *)g_dataRaw, strlen(g_dataRaw), false));
	//printf("test data size: %d\n", data->size());
	UV_DEBUG(ret->setData(data));

	return ret;
}

QString UVQtHexdumpPlugin::name() const
{
    return "UVQtHexdump";
}

QString UVQtHexdumpPlugin::group() const
{
    return "UVNet";
}

QIcon UVQtHexdumpPlugin::icon() const
{
    return QIcon();
}

QString UVQtHexdumpPlugin::toolTip() const
{
    return "";
}

QString UVQtHexdumpPlugin::whatsThis() const
{
    return "";
}

bool UVQtHexdumpPlugin::isContainer() const
{
    return false;
}

QString UVQtHexdumpPlugin::domXml() const
{
    return "<ui language=\"c++\">\n"
           " <widget class=\"UVQtHexdump\" name=\"scrollableHexdump\">\n"
           "  <property name=\"geometry\">\n"
           "   <rect>\n"
           "    <x>0</x>\n"
           "    <y>0</y>\n"
           "    <width>300</width>\n"
           "    <height>100</height>\n"
           "   </rect>\n"
           "  </property>\n"
           "  <property name=\"toolTip\" >\n"
           "   <string>Binary hexdump</string>\n"
           "  </property>\n"
           "  <property name=\"whatsThis\" >\n"
           "   <string>Hexdump displays hexidecimal and ASCII of a binary.</string>\n"
           "  </property>\n"
           " </widget>\n"
           "</ui>\n";
}

QString UVQtHexdumpPlugin::includeFile() const
{
    return "uvqt/hexdump.h";
}

