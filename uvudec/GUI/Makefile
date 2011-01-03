# UVNet Universal Decompiler (uvudec)
# Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
# Licensed under terms of the three clause BSD license, see LICENSE for details

EXE_NAME=$(GUI_EXE)

MOC_HEADERS = \
		$(GUI_DIR)/analysis_thread.h \
		$(GUI_DIR)/GUI.h \

UI_FILES = $(GUI_DIR)/uvudec.ui

CC_SRCS =   \

CXX_SRCS = 	\
		$(GUI_DIR)/analysis_action.cpp \
		$(GUI_DIR)/analysis_thread.cpp \
		$(GUI_DIR)/format.cpp \
		$(GUI_DIR)/GUI.cpp \
		$(GUI_DIR)/lock.cpp \
		$(GUI_DIR)/main.cpp \
		$(GUI_DIR)/project.cpp \

USING_LIB_UVUDEC=Y
USING_LIB_JANSSON=Y

PHONY += libuvqt
LIBS += -luvqtdesigner
LDFLAGS+= -Llibuvqt
INCLUDES += -Ilibuvqt

ROOT_DIR=..
include $(ROOT_DIR)/Makefile.mk

$(EXE_NAME_FULL): libuvqt

libuvqt:
	cd libuvqt && $(MAKE)

