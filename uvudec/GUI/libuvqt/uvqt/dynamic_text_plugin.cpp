/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvqt/dynamic_text.h"
#include "uvqt/dynamic_text_plugin.h"
#include "uvqt/dynamic_text_plugin_impl.h"
#include <QtPlugin>

UVQtDynamicTextPlugin::UVQtDynamicTextPlugin(QObject *parent)
	: QObject(parent)
{
	m_initialized = false;
}

void UVQtDynamicTextPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
	if( m_initialized )
	{
		return;
	}
	
	m_initialized = true;
}

bool UVQtDynamicTextPlugin::isInitialized() const
{
	return m_initialized;
}

QWidget *UVQtDynamicTextPlugin::createWidget(QWidget *parent)
{
	return new UVQtDynamicText(new UVQtDynamicTextDataPluginImpl(), parent);
}

QString UVQtDynamicTextPlugin::name() const
{
	return "UVQtDynamicText";
}

QString UVQtDynamicTextPlugin::group() const
{
	return "UVNet";
}

QIcon UVQtDynamicTextPlugin::icon() const
{
	return QIcon();
}

QString UVQtDynamicTextPlugin::toolTip() const
{
	return "";
}

QString UVQtDynamicTextPlugin::whatsThis() const
{
	return "";
}

bool UVQtDynamicTextPlugin::isContainer() const
{
	return false;
}

QString UVQtDynamicTextPlugin::domXml() const
{
	return "<ui language=\"c++\">\n"
		   " <widget class=\"UVQtDynamicText\" name=\"dynamicText\">\n"
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

QString UVQtDynamicTextPlugin::includeFile() const
{
	return "uvdqt/dynamic_text.h";
}

