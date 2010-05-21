# UVNet Universal Decompiler (uvudec)
# Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
# Licensed under terms of the three clause BSD license, see LICENSE for details

USING_STATIC=Y

EXE_NAME=$(TESTING_EXE)

CC_SRCS =   \

CXX_SRCS = 	\
			$(TESTING_DIR)/main.cpp \
			$(TESTING_DIR)/uvd_test.cpp \

USING_LIB_UVUDEC=Y

ROOT_DIR=..
include $(ROOT_DIR)/Makefile.mk

LIBS += -lcppunit -ldl
