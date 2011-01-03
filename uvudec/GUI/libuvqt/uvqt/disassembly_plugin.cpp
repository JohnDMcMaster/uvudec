/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvqt/disassembly.h"
#include "uvqt/disassembly_plugin.h"
#include <QtPlugin>

UVQtDisassemblyPlugin::UVQtDisassemblyPlugin(QObject *parent)
    : QObject(parent)
{
    initialized = false;
}

void UVQtDisassemblyPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
    if (initialized)
        return;

    initialized = true;
}

bool UVQtDisassemblyPlugin::isInitialized() const
{
    return initialized;
}

QWidget *UVQtDisassemblyPlugin::createWidget(QWidget *parent)
{
    return new UVQtDisassembly(parent);
}

QString UVQtDisassemblyPlugin::name() const
{
    return "UVQtDisassembly";
}

QString UVQtDisassemblyPlugin::group() const
{
    return "UVNet";
}

QIcon UVQtDisassemblyPlugin::icon() const
{
    return QIcon();
}

QString UVQtDisassemblyPlugin::toolTip() const
{
    return "";
}

QString UVQtDisassemblyPlugin::whatsThis() const
{
    return "";
}

bool UVQtDisassemblyPlugin::isContainer() const
{
    return false;
}

QString UVQtDisassemblyPlugin::domXml() const
{
    return "<ui language=\"c++\">\n"
           " <widget class=\"UVQtDisassembly\" name=\"scrollableDisassembly\">\n"
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
           "   <string>Display disassembly.</string>\n"
           "  </property>\n"
           " </widget>\n"
           "</ui>\n";
}

QString UVQtDisassemblyPlugin::includeFile() const
{
    return "uvqt/disassembly.h";
}

