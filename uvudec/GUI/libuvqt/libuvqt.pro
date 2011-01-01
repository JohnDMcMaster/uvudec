# UVNet Universal Decompiler (uvudec)
# Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
# Licensed under terms of the three clause BSD license, see LICENSE for details

# install -m 755 -p "libcustomwidgetplugin.so" "/opt/qtsdk-2010.04/qt/plugins/designer/libcustomwidgetplugin.so"

QMAKE_CXXFLAGS += -g -Wno-unused-parameter -Werror -O0
CONFIG      += designer plugin
TARGET      = uvqtdesigner
TEMPLATE    = lib
QTDIR_build:DESTDIR     = $$QT_BUILD_TREE/plugins/designer

HEADERS     = 	\
				uvqt/dynamic_text.h \
				uvqt/dynamic_text_plugin.h \
				uvqt/dynamic_text_plugin_impl.h \
				uvqt/hexdump.h \
				uvqt/hexdump_plugin.h \
				uvqt/plain_text_edit.h \
				uvqt/plugin.h \
				uvqt/util.h \

SOURCES     = 	\
				uvqt/dynamic_text.cpp \
				uvqt/dynamic_text_plugin.cpp \
				uvqt/dynamic_text_plugin_impl.cpp \
				uvqt/hexdump.cpp \
				uvqt/hexdump_plugin.cpp \
				uvqt/plain_text_edit.cpp \
				uvqt/plugin.cpp \
				uvqt/util.cpp \

LIBS += -L../../lib -luvudec
INCLUDEPATH += ../../libuvudec

# install
target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target

