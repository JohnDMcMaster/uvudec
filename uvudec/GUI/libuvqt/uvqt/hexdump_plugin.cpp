/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the GPL V3 or later, see COPYING for details
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

QWidget *UVQtHexdumpPlugin::createWidget(QWidget *parent)
{
    return new UVQtScrollableHexdump(parent);
}

QString UVQtHexdumpPlugin::name() const
{
    return "UVQtScrollableHexdump";
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
           " <widget class=\"UVQtScrollableHexdump\" name=\"scrollableHexdump\">\n"
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
    return "uvdqt/hexdump.h";
}

