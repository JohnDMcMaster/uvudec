# UVNet Universal Decompiler (uvudec)
# Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
# Licensed under terms of the three clause BSD license, see LICENSE for details

ifeq ($(USING_STATIC),Y)
EXE_NAME=$(UVUDEC_EXE).static
else
EXE_NAME=$(UVUDEC_EXE).dynamic
endif

CC_SRCS =   \

CXX_SRCS = 	\
			$(CORE_DIR)/main.cpp \

USING_LIB_UVUDEC=Y

ROOT_DIR=..
include $(ROOT_DIR)/Makefile.mk
