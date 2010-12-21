# UVNet Universal Decompiler (uvudec)
# Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
# Licensed under terms of the three clause BSD license, see LICENSE for details

QT_DIR=$(QT_PREFIX)/qt
QT_LIB_DIR = $(QT_DIR)/lib
QT_BIN_DIR = $(QT_DIR)/bin

QT_DEFINES    = -DQT_NO_DEBUG -DQT_GUI_LIB -DQT_CORE_LIB -DQT_SHARED
QT_INCPATH    = -I$(QT_DIR)/mkspecs/linux-g++ -I. -I$(QT_DIR)/include/QtCore -I$(QT_DIR)/include/QtGui -I$(QT_DIR)/include -I. -I.
QT_LFLAGS     = -Wl,-O1 -Wl,-rpath,$(QT_LIB_DIR)
# Why did example have this double path?
QT_LIBS       = -L$(QT_LIB_DIR) -lQtGui -L$(QT_LIB_DIR) -L$(X11R6_LIB_DIR) -lQtCore -lpthread 
QT_QMAKE = $(QT_BIN_DIR)/qmake
QT_UIC = $(QT_BIN_DIR)/uic
QT_MOC = $(QT_BIN_DIR)/moc

MOC_SRCS = $(MOC_HEADERS:.h=.moc.cpp)

CXX_SRCS += $(MOC_SRCS)

FLAGS_SHARED += $(QT_DEFINES) $(QT_INCPATH)
LDFLAGS += $(QT_LFLAGS) $(QT_LIBS)

CLEAN_DEPS+=cleanMoc

cleanMoc:
	rm -f *.moc.cpp	$(MOC_SRCS)
	
%.moc.cpp: %.h
	$(QT_MOC) $(QT_DEFINES) $(QT_INCPATH) $< -o $@




ifdef UI_FILES

CLEAN_DEPS += cleanGUI

UI_SRCS = $(UI_FILES:.ui=.ui.h)

# Manual dependencie are needed because since the file isn't present, autodep can't generate a path for it
$(CXX_SRCS): $(UI_SRCS)

%.ui.h: %.ui
	$(QT_UIC) $< -o $@

cleanGUI:
	$(RM) ui_uvudec.h

endif

