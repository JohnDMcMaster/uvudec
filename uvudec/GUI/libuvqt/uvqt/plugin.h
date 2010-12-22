/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the GPL V3 or later, see COPYING for details
*/

#ifndef CUSTOMWIDGETPLUGIN_H
#define CUSTOMWIDGETPLUGIN_H

#include <QDesignerCustomWidgetInterface>

class UVQtPlugin: public QObject, public QDesignerCustomWidgetCollectionInterface
{
	Q_OBJECT
	Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)

public:
	UVQtPlugin(QObject *parent = 0);

	virtual QList<QDesignerCustomWidgetInterface*> customWidgets() const;

private:
	QList<QDesignerCustomWidgetInterface*> m_widgets;
};



#endif

