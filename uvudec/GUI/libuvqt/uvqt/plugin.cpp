#include "uvqt/hexdump.h"
#include "uvqt/plugin.h"
#include "uvd/util/util.h"

#include <QtPlugin>

UVQtPlugin::UVQtPlugin(QObject *parent)
    : QObject(parent)
{
	std::vector<std::string> the_split = UVDSplit("blah blah blah", ' ', true);
	printf("split items: %d\n", the_split.size());

    initialized = false;
}

void UVQtPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
    if (initialized)
        return;

    initialized = true;
}

bool UVQtPlugin::isInitialized() const
{
    return initialized;
}

QWidget *UVQtPlugin::createWidget(QWidget *parent)
{
    return new UVQtScrollableHexdump(parent);
}

QString UVQtPlugin::name() const
{
    return "UVQtScrollableHexdump";
}

QString UVQtPlugin::group() const
{
    return "UVNet";
}

QIcon UVQtPlugin::icon() const
{
    return QIcon();
}

QString UVQtPlugin::toolTip() const
{
    return "";
}

QString UVQtPlugin::whatsThis() const
{
    return "";
}

bool UVQtPlugin::isContainer() const
{
    return false;
}

QString UVQtPlugin::domXml() const
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

QString UVQtPlugin::includeFile() const
{
    return "uvdqt/hexdump.h";
}

Q_EXPORT_PLUGIN2(customwidgetplugin, UVQtPlugin)

