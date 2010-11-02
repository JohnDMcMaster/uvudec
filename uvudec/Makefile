# UVNet Universal Decompiler (uvudec)
# Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
# Licensed under GPL V3+, see COPYING for details

PHONY += uvudec bin2obj obj2pat pat2sig flirtutil lib doc cleanSub cleanSubCore installer dist distCore python test gui plugin uvudec

ALL_TARGETS=lib
# ALL_TARGETS += doc
CLEAN_DEPS=cleanSub

# These get included twice, but should be small lightweight files and not an issue
include Makefile.defaults
include Makefile.configure

ifeq ($(USING_UVUDEC),Y)
ALL_TARGETS += uvudec
endif

ifeq ($(USING_BIN2OBJ),Y)
ALL_TARGETS += bin2obj
endif

ifeq ($(USING_OBJ2PAT),Y)
ALL_TARGETS += obj2pat
endif

ifeq ($(USING_PAT2SIG),Y)
ALL_TARGETS += pat2sig
endif

ifeq ($(USING_FLIRTUTIL),Y)
ALL_TARGETS += flirtutil
endif

ifeq ($(USING_GUI),Y)
ALL_TARGETS += gui
endif

ifeq ($(USING_PLUGIN),Y)
ALL_TARGETS += plugin
endif

ifeq ($(USING_PYTHON_API),Y)
ALL_TARGETS += python
endif

include Makefile.mk

# SOURCE_DIRS=$(ROOT_DIR) $(COMPILER_DIR) $(CORE_DIR) $(DATA_DIR) $(ELF_DIR) $(GUI_DIR) $(HASH_DIR) $(INIT_DIR) $(INTERPRETER_DIR) $(LANGUAGE_DIR) $(RELOCATION_DIR) $(UTIL_DIR)


PACKAGE=uvudec

ifeq ($(USING_UVUDEC),Y)
uvudec:
	cd uvudec && $(MAKE)
endif

ifeq ($(USING_BIN2OBJ),Y)
bin2obj:
	cd bin2obj && $(MAKE)
endif

ifeq ($(USING_OBJ2PAT),Y)
obj2pat:
	cd obj2pat && $(MAKE)
endif

ifeq ($(USING_PAT2SIG),Y)
pat2sig:
	cd pat2sig && $(MAKE)
endif

ifeq ($(USING_FLIRTUTIL),Y)
flirtutil:
	cd flirtutil && $(MAKE)
endif

ifeq ($(USING_GUI),Y)
gui:
	cd GUI && $(MAKE)
endif

ifeq ($(USING_PLUGIN),Y)
plugin:
	cd plugin && $(MAKE)
endif

ifeq ($(USING_PYTHON_API),Y)
python:
	cd python && $(MAKE)
endif

dynamic:
	$(MAKE) all LINKAGE=dynamic

static:
	$(MAKE) all LINKAGE=static

test:
	cd testing && $(MAKE)

# Good enough for now
run_test:
	$(TESTING_EXE).dynamic

lib:
	cd libuvudec && $(MAKE)

doc:
	cd doc && $(MAKE)

cleanSub:
	for curDir in $(shell find -maxdepth 1 -type d |egrep -v '^.$$'); do \
		$(MAKE) cleanSubCore CLEAN_DIR=$${curDir};\
	done;
	echo 'cleanSub'

# Because I couldn't figure out how to escape the '; endif'
cleanSubCore:
	if [ -f $(CLEAN_DIR)/Makefile ]; then cd $(CLEAN_DIR) && $(MAKE) clean; fi
	@(true)


# Man
MAN_SECTION=3
MAN_RAW=manpage
MAN_PAGE=uvudec
MAN_UNCOMPRESSED=$(MAN_PAGE).$(MAN_SECTION)
MAN=$(MAN_UNCOMPRESSED).gz
MAN_DIR=$(PREFIX)/usr/share/man/man$(MAN_SECTION)
man:
	$(CP) $(MAN_RAW) $(MAN_UNCOMPRESSED)
	$(RM) $(MAN)
	$(PERL) -pi -e "s/UVUDEC_VER/$(UVUDEC_VER)/" $(MAN_UNCOMPRESSED)
	$(GZIP) $(MAN_UNCOMPRESSED)

INSTALL_DIR=$(INSTALL_DIR_BASE)/$(UVUDEC_VER)

install: man
# Might be upgrading
	rm -rf $(INSTALL_DIR)
	mkdir -p $(INSTALL_DIR)
	cp -r arch bin $(INSTALL_DIR)
# Make a generic link for convenience if we were using suffixes
ifeq ($(EXE_BUILD_SUFFIX),Y)
# Arbitrarily favor static exe's if both specified
ifeq ($(USING_STATIC),Y)
#	ln -sf $(INSTALL_DIR)/bin/uvudec $(INSTALL_DIR)/bin/uvudec.static
else
#	ln -sf $(INSTALL_DIR)/bin/uvudec $(INSTALL_DIR)/bin/uvudec.dynamic
endif
endif
	$(INSTALL) $(MAN) $(MAN_DIR)
# Redirect current version link
# ick ln has weird behavior...or is it just me
# -f option is useless or unrelaible anyway
# if file to create link to exists as a directory and file to create link at exists
# the link is instead placed into the target directory
# so, we must remove the old link first and -f doesn't help
	cd $(INSTALL_DIR_BASE) && rm -f current && ln -sf $(UVUDEC_VER) current

installer:
	@(echo 'Oh noes!')
	@(echo 'Ce n"est supported pas!' 

dist:
	$(MAKE) clean
	$(MAKE) all
	$(MAKE) distCore

distCore:
	@(echo "Distributing...")
	$(RM) -r $(PACKAGE)-$(UVUDEC_VER)*
	$(MKDIR) $(PACKAGE)-$(UVUDEC_VER)
	$(CP) -r *.cpp *.h Makefile test arch doc $(MAN_RAW) $(PACKAGE)-$(UVUDEC_VER)
	$(RM) -r $(PACKAGE)-$(UVUDEC_VER)/.svn
	$(TAR) -cvf $(PACKAGE)-$(UVUDEC_VER).tar $(PACKAGE)-$(UVUDEC_VER)
	$(GZIP) $(PACKAGE)-$(UVUDEC_VER).tar
	$(RM) -r $(PACKAGE)-$(UVUDEC_VER)

mrproper: clean
	find -name '*.o' -exec rm {} ';'
	find -name '*.so' -exec rm {} ';'
	find -name '*.a' -exec rm {} ';'

