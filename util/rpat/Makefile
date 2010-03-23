# Makefile for Red Plait's rpat
# Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
# This file is not for public release due to legal issues :(

USING_STATIC=N
INCLUDES=

ifeq ($(USING_STATIC),Y)
# BINUTILS_DIR=/home/mcmaster/document/build/GNU/binutils/binutils-2.19.1
# BINUTILS_DIR=/home/mcmaster/document/build/GNU/binutils/binutils-2.17
LIBBFD_STATIC_LIB=$(BINUTILS_DIR)/bfd/libbfd.a
LIBOPCODES_STATIC_LIB=$(BINUTILS_DIR)/opcodes/libopcodes.a
LIBIBERTY_STATIC_LIB=$(BINUTILS_DIR)/libiberty/libiberty.a
LIBS=$(LIBBFD_STATIC_LIB) $(LIBOPCODES_STATIC_LIB) $(LIBIBERTY_STATIC_LIB)
else
LIBS=-lbfd -lopcodes -liberty
endif
CFLAGS = -Wall -Werror

# INCLUDES+=-I$(BINUTILS_DIR)/binutilsX 
# INCLUDES+=-I$(BINUTILS_DIR)/include

all:
# seems to work (compile, code has horrible error check and crashes)
# gcc simple.c  -lbfd -lopcodes -liberty -lbfd -lopcodes -liberty -o simple
#	gcc simple.c $(INCLUDES) $(LIBS) -o simple
	gcc rpat.c $(CFLAGS) $(INCLUDES) $(LIBS) -o rpat
	
	

