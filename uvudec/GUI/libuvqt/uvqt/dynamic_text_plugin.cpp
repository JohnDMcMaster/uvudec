/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvqt/dynamic_text.h"
#include "uvqt/dynamic_text_plugin.h"
#include "uvqt/dynamic_text_plugin_impl.h"
#include <QtPlugin>

UVQtScrollableDynamicTextPlugin::UVQtScrollableDynamicTextPlugin(QObject *parent)
	: QObject(parent)
{
	printf("UVQtScrollableDynamicTextPlugin::UVQtScrollableDynamicTextPlugin()\n");
	m_initialized = false;
}

void UVQtScrollableDynamicTextPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
	printf("UVQtScrollableDynamicTextPlugin::initialize()\n");
	if( m_initialized )
	{
		return;
	}
	
	m_initialized = true;
}

bool UVQtScrollableDynamicTextPlugin::isInitialized() const
{
	return m_initialized;
}

QWidget *UVQtScrollableDynamicTextPlugin::createWidget(QWidget *parent)
{
	printf("UVQtScrollableDynamicTextPlugin::createWidget()\n");
	return new UVQtScrollableDynamicText(new UVQtDynamicTextDataPluginImpl(), parent);
}

QString UVQtScrollableDynamicTextPlugin::name() const
{
	return "UVQtScrollableDynamicText";
}

QString UVQtScrollableDynamicTextPlugin::group() const
{
	return "UVNet";
}

QIcon UVQtScrollableDynamicTextPlugin::icon() const
{
	return QIcon();
}

QString UVQtScrollableDynamicTextPlugin::toolTip() const
{
	return "";
}

QString UVQtScrollableDynamicTextPlugin::whatsThis() const
{
	return "";
}

bool UVQtScrollableDynamicTextPlugin::isContainer() const
{
	return false;
}

QString UVQtScrollableDynamicTextPlugin::domXml() const
{
	return "<ui language=\"c++\">\n"
		   " <widget class=\"UVQtScrollableDynamicText\" name=\"dynamicText\">\n"
		   "  <property name=\"geometry\">\n"
		   "   <rect>\n"
		   "	<x>0</x>\n"
		   "	<y>0</y>\n"
		   "	<width>300</width>\n"
		   "	<height>100</height>\n"
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

QString UVQtScrollableDynamicTextPlugin::includeFile() const
{
	return "uvdqt/dynamic_text.h";
}

