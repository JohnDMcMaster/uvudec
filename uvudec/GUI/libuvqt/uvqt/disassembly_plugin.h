/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the GPL V3 or later, see COPYING for details
*/

#ifndef UVQT_DISASSEMBLY_PLUGIN_H
#define UVQT_DISASSEMBLY_PLUGIN_H

#include <QtDesigner/QtDesigner>
#include <QtCore/qplugin.h>

class UVQtDisassemblyPlugin : public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    UVQtDisassemblyPlugin(QObject *parent = 0);

    bool isContainer() const;
    bool isInitialized() const;
    QIcon icon() const;
    QString domXml() const;
    QString group() const;
    QString includeFile() const;
    QString name() const;
    QString toolTip() const;
    QString whatsThis() const;
    QWidget *createWidget(QWidget *parent);
    void initialize(QDesignerFormEditorInterface *core);

private:
    bool initialized;
};

#endif

