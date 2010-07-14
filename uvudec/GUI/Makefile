# UVNet Universal Decompiler (uvudec)
# Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
# Licensed under terms of the three clause BSD license, see LICENSE for details

EXE_NAME=$(GUI_EXE)

X11R6_LIB_DIR = /usr/X11R6/lib
QT_PREFIX=/opt/qtsdk-2010.04
QT_LIB_DIR = $(QT_PREFIX)/qt/lib
QT_BIN_DIR = $(QT_PREFIX)/qt/bin

QT_DEFINES    = -DQT_NO_DEBUG -DQT_GUI_LIB -DQT_CORE_LIB -DQT_SHARED
QT_INCPATH    = -I/opt/qtsdk-2010.04/qt/mkspecs/linux-g++ -I. -I/opt/qtsdk-2010.04/qt/include/QtCore -I/opt/qtsdk-2010.04/qt/include/QtGui -I/opt/qtsdk-2010.04/qt/include -I. -I.
QT_LFLAGS     = -Wl,-O1 -Wl,-rpath,$(QT_LIB_DIR)
# Why did example have this double path?
QT_LIBS       = -L$(QT_LIB_DIR) -lQtGui -L$(QT_LIB_DIR) -L$(X11R6_LIB_DIR) -lQtCore -lpthread 
QT_QMAKE = $(QT_BIN_DIR)/qmake
QT_UIC = $(QT_BIN_DIR)/uic
QT_MOC = $(QT_BIN_DIR)/moc

FLAGS_SHARED += $(QT_DEFINES) $(QT_INCPATH)
LDFLAGS += $(QT_LFLAGS) $(QT_LIBS)

CC_SRCS =   \

CXX_SRCS = 	\
		$(GUI_DIR)/uvd_GUI.cpp \
		$(GUI_DIR)/main.cpp \
		$(GUI_DIR)/moc_uvd_GUI.cpp

USING_LIB_UVUDEC=Y

CLEAN_DEPS += cleanGUI

ROOT_DIR=..
include $(ROOT_DIR)/Makefile.mk

$(GUI_DIR)/moc_uvd_GUI.cpp: $(GUI_DIR)/ui_uvudec.h $(GUI_DIR)/uvd_GUI.h
	$(QT_MOC) $(QT_DEFINES) $(QT_INCPATH) $(GUI_DIR)/uvd_GUI.h -o $(GUI_DIR)/moc_uvd_GUI.cpp

# Manual dependencie are needed because since the file isn't present, autodep can't generate a path for it
$(CXX_SRCS): $(GUI_DIR)/ui_uvudec.h

$(GUI_DIR)/ui_uvudec.h: $(GUI_DIR)/uvudec.ui
	$(QT_UIC) $(GUI_DIR)/uvudec.ui -o $(GUI_DIR)/ui_uvudec.h

cleanGUI:
	$(RM) moc_uvd_GUI.cpp ui_uvudec.h

