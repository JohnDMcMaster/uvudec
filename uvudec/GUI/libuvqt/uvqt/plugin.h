#ifndef CUSTOMWIDGETPLUGIN_H
#define CUSTOMWIDGETPLUGIN_H

#include <QDesignerCustomWidgetInterface>
#include <QtDesigner/QtDesigner>
#include <QtCore/qplugin.h>

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

class UVQtPlugin1 : public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    UVQtPlugin1(QObject *parent = 0);

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

class UVQtPlugin2 : public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    UVQtPlugin2(QObject *parent = 0);

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

