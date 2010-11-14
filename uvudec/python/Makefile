# strings plugin
# UVNet Universal Decompiler (uvudec)
# Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
# Licensed under the terms of the LGPL V3 or later, see COPYING for details

ROOT_DIR=..
LIB_NAME=_uvudec
# Don't add .3.1.2 stuff
LIB_VER_FILENAME_EXTENSIONS=N
# Doesn't make sense for python modules I think
CREATE_LIB_LINKS=N
# Important...
USING_LIB_UVUDEC=Y

CC_SRCS =   \

CXX_SRCS = 	\
			uvudec_wrap.cpp \

CLEAN_DEPS+=cleanPython
include $(ROOT_DIR)/Makefile.mk

# Python seems to require the _
#LIB_PREFIX=_

# FIXME: generate dependencies using -MM
# swig -c++ -python -I../libuvudec -MM uvudec.i

gen_typemaps.i: ALWAYS
	./gen_typemaps.py
uvudec.i: gen_typemaps.i
uvudec_wrap.cpp: uvudec.i
	swig -c++ -python -I../libuvudec -o uvudec_wrap.cpp -outdir $(LIB_DIR) uvudec.i

ALWAYS:
	@(true)

.PHONY: ALWAYS

cleanPython:
	rm -f uvudec_wrap.cpp gen_typemaps.i

