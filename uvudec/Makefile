# UVNet Universal Decompiler (uvudec)
# Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
# Licensed under terms of the three clause BSD license, see LICENSE for details

PHONY += uvudec coff2pat omf2pat elf2pat obj2pat pat2sig lib doc cleanSub cleanSubCore installer dist distCore test

ALL_TARGETS=lib
# ALL_TARGETS += doc
CLEAN_DEPS=cleanSub

include Makefile.configure

ifeq ($(USING_UVUDEC),Y)
ALL_TARGETS += uvudec
endif

ifeq ($(USING_COFF2PAT),Y)
ALL_TARGETS += coff2pat
endif

ifeq ($(USING_OMF2PAT),Y)
ALL_TARGETS += omf2pat
endif

ifeq ($(USING_ELF2PAT),Y)
ALL_TARGETS += elf2pat
endif

ifeq ($(USING_OBJ2PAT),Y)
ALL_TARGETS += obj2pat
endif

ifeq ($(USING_PAT2SIG),Y)
ALL_TARGETS += pat2sig
endif

include Makefile.mk

SOURCE_DIRS=$(ROOT_DIR) $(COMPILER_DIR) $(	CORE_DIR) $(DATA_DIR) $(ELF_DIR) $(HASH_DIR) $(INIT_DIR) $(INTERPRETER_DIR) $(LANGUAGE_DIR) $(UTIL_DIR) $(RELOCATION_DIR)


# Man
MAN_SECTION=3
MAN_RAW=manpage
MAN_PAGE=uvudec
MAN_UNCOMPRESSED=$(MAN_PAGE).$(MAN_SECTION)
MAN=$(MAN_UNCOMPRESSED).gz
MAN_DIR=$(PREFIX)/man/man$(MAN_SECTION)

PACKAGE=uvudec

ifeq ($(USING_UVUDEC),Y)
uvudec: lib
	cd core && $(MAKE)
endif

ifeq ($(USING_AR2PAT),Y)
ar2pat:
	cd ar2pat && $(MAKE)
endif

ifeq ($(USING_COFF2PAT),Y)
coff2pat:
	cd coff2pat && $(MAKE)
endif

ifeq ($(USING_OMF2PAT),Y)
omf2pat:
	cd omf2pat && $(MAKE)
endif

ifeq ($(USING_ELF2PAT),Y)
elf2pat:
	cd elf2pat && $(MAKE)
endif

ifeq ($(USING_OBJ2PAT),Y)
obj2pat:
	cd obj2pat && $(MAKE)
endif

ifeq ($(USING_PAT2SIG),Y)
pat2sig:
	cd pat2sig && $(MAKE)
endif

dynamic:
	$(MAKE) all USING_STATIC=N

static:
	$(MAKE) all USING_STATIC=Y

test:
	cd testing && $(MAKE)

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
