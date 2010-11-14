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

DEPEND_DEP += dependSWIG

CC_SRCS =   \

CXX_SRCS = 	\
			uvudec_wrap.cpp \

CLEAN_DEPS+=cleanPython
include $(ROOT_DIR)/Makefile.mk

# Python seems to require the _
#LIB_PREFIX=_


MAKEFILE_DEPEND_SWIG=Makefile.depend.swig$(OBJECT_LINKAGE_SUFFIX)
$(shell touch $(MAKEFILE_DEPEND_SWIG))
include $(MAKEFILE_DEPEND_SWIG)

# FIXME: generate dependencies using -MM
# swig -c++ -python -I../libuvudec -MM uvudec.i
dependSWIG:
	$(SWIG) -MM $(SWIG_FLAGS) uvudec.i >$(MAKEFILE_DEPEND_SWIG)
	perl -pi -e 's/cxx/cpp/g' $(MAKEFILE_DEPEND_SWIG)

SWIG=swig
SWIG_FLAGS=-c++ -python -I../libuvudec

gen_typemaps.i: ../libuvudec/uvd/all.h
	./gen_typemaps.py
uvudec_wrap.cpp: uvudec.i
	$(SWIG) $(SWIG_FLAGS) -o uvudec_wrap.cpp -outdir $(LIB_DIR) uvudec.i

cleanPython:
	rm -f uvudec_wrap.cpp gen_typemaps.i

