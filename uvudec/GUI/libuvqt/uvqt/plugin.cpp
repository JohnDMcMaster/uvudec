/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the GPL V3 or later, see COPYING for details
*/

#include "uvqt/disassembly_plugin.h"
#include "uvqt/dynamic_text_plugin.h"
#include "uvqt/hexdump_plugin.h"
#include "uvqt/plugin.h"
//#include "uvd/util/util.h"
#include <QtPlugin>

UVQtPlugin::UVQtPlugin(QObject *parent)
		: QObject(parent)
{
	printf("UVQtPlugin:constructor\n");
//	m_widgets.append(new UVQtHexdumpPlugin(this));
//	m_widgets.append(new UVQtDisassemblyPlugin(this));
	m_widgets.append(new UVQtScrollableDynamicTextPlugin(this));
}

QList<QDesignerCustomWidgetInterface*> UVQtPlugin::customWidgets() const
{
	return m_widgets;
}

Q_EXPORT_PLUGIN2(customwidgetsplugin, UVQtPlugin)

