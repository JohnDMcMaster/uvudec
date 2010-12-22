#include "uvqt/hexdump.h"
#include "uvqt/plugin.h"
#include "uvd/util/util.h"

#include <QtPlugin>

UVQtPlugin2::UVQtPlugin2(QObject *parent)
    : QObject(parent)
{
	std::vector<std::string> the_split = UVDSplit("blah blah blah", ' ', true);
	printf("split items: %d\n", the_split.size());

    initialized = false;
}

void UVQtPlugin2::initialize(QDesignerFormEditorInterface * /* core */)
{
    if (initialized)
        return;

    initialized = true;
}

bool UVQtPlugin2::isInitialized() const
{
    return initialized;
}

QWidget *UVQtPlugin2::createWidget(QWidget *parent)
{
    return new UVQtScrollableHexdump(parent);
}

QString UVQtPlugin2::name() const
{
    return "UVQtScrollableHexdump2";
}

QString UVQtPlugin2::group() const
{
    return "UVNet";
}

QIcon UVQtPlugin2::icon() const
{
    return QIcon();
}

QString UVQtPlugin2::toolTip() const
{
    return "";
}

QString UVQtPlugin2::whatsThis() const
{
    return "";
}

bool UVQtPlugin2::isContainer() const
{
    return false;
}

QString UVQtPlugin2::domXml() const
{
    return "<ui language=\"c++\">\n"
           " <widget class=\"UVQtScrollableHexdump2\" name=\"scrollableHexdump\">\n"
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

QString UVQtPlugin2::includeFile() const
{
    return "uvdqt/hexdump.h";
}

