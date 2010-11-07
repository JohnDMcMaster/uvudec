# Plugin recursive build makefile
# UVNet Universal Decompiler (uvudec)
# Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
# Licensed under terms of the three clause BSD license, see LICENSE for details

ALL_TARGETS += recurse
CLEAN_DEPS+=cleanRecurse
ROOT_DIR=..
include Makefile.mk

recurse:
	for subMakefile in $(shell find -mindepth 2 -maxdepth 2 -name 'Makefile'); do \
		DIR=$$(dirname $${subMakefile}) ; \
		(cd $$DIR && $(MAKE)) ||exit 1 ; \
	done;

cleanRecurse:
	for subMakefile in $(shell find -mindepth 2 -maxdepth 2 -name 'Makefile'); do \
		DIR=$$(dirname $${subMakefile}) ; \
		(cd $$DIR && $(MAKE) clean) ||exit 1 ; \
	done;

