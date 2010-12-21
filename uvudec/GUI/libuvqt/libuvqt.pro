# install -m 755 -p "libcustomwidgetplugin.so" "/opt/qtsdk-2010.04/qt/plugins/designer/libcustomwidgetplugin.so"

CONFIG      += designer plugin
TARGET      = uvqtdesigner
TEMPLATE    = lib
QTDIR_build:DESTDIR     = $$QT_BUILD_TREE/plugins/designer

HEADERS     = 	uvqt/hexdump.h \
				uvqt/plugin.h \

SOURCES     = 	uvqt/hexdump.cpp \
				uvqt/plugin.cpp \

# install
target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target

