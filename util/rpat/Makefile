# Makefile for Red Plait's rpat
# Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
# Licensed under terms of the GPL, see LICENSE for details

USING_LIBBFD_UNINSTALLED=Y
USING_STATIC=Y
INCLUDES=

ifeq ($(USING_STATIC),Y)

ifeq ($(USING_LIBBFD_UNINSTALLED),Y)
BINUTILS_DIR=/home/mcmaster/document/build/GNU/binutils/binutils-2.19.1
LIBBFD_STATIC_LIB=$(BINUTILS_DIR)/bfd/libbfd.a
LIBOPCODES_STATIC_LIB=$(BINUTILS_DIR)/opcodes/libopcodes.a
LIBIBERTY_STATIC_LIB=$(BINUTILS_DIR)/libiberty/libiberty.a
# Contains "libbfd.h"
INCLUDES+=-I$(BINUTILS_DIR)/binutils
# You shouldn't use this, for libbfd internal use only
# However, if you are desperate, it does have extra stuff, such as demangling
# INCLUDES+=-I$(BINUTILS_DIR)/include
else
LIBBFD_STATIC_LIB=libbfd.a
LIBOPCODES_STATIC_LIB=libopcodes.a
LIBIBERTY_STATIC_LIB=libiberty.a
endif
LIBS=$(LIBBFD_STATIC_LIB) $(LIBOPCODES_STATIC_LIB) $(LIBIBERTY_STATIC_LIB)

# Dynamic
else

# Otherwise use system provided (doesn't build until install)
LIBS += -lbfd -lopcodes -liberty

endif

CFLAGS += -Wall -Werror

LIBS += -lz

all:
	gcc rpat.c $(CFLAGS) $(INCLUDES) $(LIBS) -o rpat
# For testing
	gcc -c rpat.c $(CFLAGS) $(INCLUDES) -o main.o
