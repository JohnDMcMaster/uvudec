# UVNet Universal Decompiler (uvudec)
# Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
# Licensed under GPL V3+, see COPYING for details

USING_STATIC=Y

EXE_NAME=$(TESTING_EXE)

CC_SRCS =   \

CXX_SRCS = 	\
			$(TESTING_DIR)/common_fixture.cpp \
			$(TESTING_DIR)/flirt.cpp \
			$(TESTING_DIR)/flirtutil.cpp \
			$(TESTING_DIR)/flirtutil_main_hook.cpp \
			$(TESTING_DIR)/libuvudec.cpp \
			$(TESTING_DIR)/main.cpp \
			$(TESTING_DIR)/object.cpp \
			$(TESTING_DIR)/obj2pat.cpp \
			$(TESTING_DIR)/obj2pat_main_hook.cpp \
			$(TESTING_DIR)/pat2sig.cpp \
			$(TESTING_DIR)/pat2sig_main_hook.cpp \
			$(TESTING_DIR)/plugin.cpp \
			$(TESTING_DIR)/uvdobjgb.cpp \
			$(TESTING_DIR)/uvudec.cpp \
			$(TESTING_DIR)/uvudec_main_hook.cpp \

USING_LIB_UVUDEC=Y

LIBS += -luvdobjgb
LDFLAGS+=-L$(PLUGIN_LIB_DIR)
ROOT_DIR=..
include $(ROOT_DIR)/Makefile.mk

LIBS += -lcppunit -ldl
