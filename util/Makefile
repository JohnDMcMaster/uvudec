# UVNet Utils (uvutils)
# Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
# Licensed under GPL V3+, see COPYING for details

INSTALL_DIR=/usr/local/bin
PYTHON_FILES=./uvromstat/uvromstat.py ./uvstructoffset/uvstructoffset.py ./uvclass/uvclass.py ./uvinj/uvinj.py ./uvstructjson/uvstructjson.py ./uventropymap/uventropymap.py
COPYING_FILES=$(ROOT_DIR)/LGPL_V3
PACKAGE_NAME=uvutils-$(UVUDEC_VER)

ROOT_DIR=../uvudec
include $(ROOT_DIR)/Makefile.version

install:

dist:
	mkdir $(PACKAGE_NAME)
	$(MAKE) cpTransform CP_DIR=$(PACKAGE_NAME)
	cp $(COPYING_FILES) $(PACKAGE_NAME)
	tar -cjf $(PACKAGE_NAME).tar.bz2 $(PACKAGE_NAME)
	rm -rf $(PACKAGE_NAME)

distFile:

cpTransform:
#for file in $(PYTHON_FILES); do echo $(shell echo $$(file) |awk -F ./ '{ print "cp " $$0 " " $(CP_DIR) $$2 }') ;
	echo test
	
