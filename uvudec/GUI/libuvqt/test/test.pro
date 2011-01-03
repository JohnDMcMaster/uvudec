# UVNet Universal Decompiler (uvudec)
# Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
# Licensed under the terms of the GPL V3 or later, see COPYING for details

# install -m 755 -p "libcustomwidgetplugin.so" "/opt/qtsdk-2010.04/qt/plugins/designer/libcustomwidgetplugin.so"

QMAKE_CXXFLAGS += -g
CONFIG      += 
TARGET      = test
TEMPLATE    = app

HEADERS     = 	\

SOURCES     = 	\
				main.cpp \

LIBS += -L../../lib -luvudec -luvqtdesigner -L.
INCLUDEPATH += ../../libuvudec

