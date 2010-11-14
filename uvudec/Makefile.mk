# UVNet Universal Decompiler (uvudec)
# Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
# Licensed under GPL V3+, see COPYING for details

# LINKAGE: set to either dynamic or static for our current target
# USING_DYNAMIC: if we configured to support building dynamic exe
# USING_STATIC: if we configured to support building static exe

ifeq ($(INCLUDE_LEVEL),)
INCLUDE_LEVEL=.
else
INCLUDE_LEVEL+=/..
endif

ifndef ROOT_DIR
ROOT_DIR=$(INCLUDE_LEVEL)
endif

default: all
	@(true)

# FIXME: how do I do an "or"
COMPILING_CODE=
ifdef EXE_NAME
COMPILING_CODE=Y
endif
ifdef LIB_NAME
COMPILING_CODE=Y
endif

# System defaults
include $(ROOT_DIR)/Makefile.defaults
# Optional "./configure" result
#$(shell if [ '!' -f $(ROOT_DIR)/Makefile.configure ] ; then touch $(ROOT_DIR)/Makefile.configure); fi )
include $(ROOT_DIR)/Makefile.configure

BIN_DIR=$(ROOT_DIR)/bin
LIB_DIR=$(ROOT_DIR)/lib
PLUGIN_LIB_DIR=$(LIB_DIR)/plugin

# Libuvudec
# TODO: these should all be removed as we move to absolute paths from the root uvd dir
LIBUVUDEC_DIR=$(ROOT_DIR)/libuvudec
# Others
GUI_DIR=$(ROOT_DIR)/GUI
PLUGIN_DIR=$(ROOT_DIR)/plugin
TESTING_DIR=$(ROOT_DIR)/testing
UVUDEC_DIR=$(ROOT_DIR)/uvudec

include $(ROOT_DIR)/Makefile.version

PACKAGE=uvudec

# Moving away from this option and instead using paths
ifneq ($(NO_INCLUDE_DOT),Y)
INCLUDES += -I.
INCLUDES += -I$(ROOT_DIR)
endif
# hmm include are kinda weird, all projects use <dir_name>/<file_name>.h, but we include all invidual dirs
INCLUDES += -I$(LIBUVUDEC_DIR)
# UVD official plugins should prefix include paths with the plugin name
INCLUDES += -I$(PLUGIN_DIR)

#OPTIMIZATION_LEVEL=-O3
DEBUG_FLAGS=-g
WARNING_FLAGS=-Wall -Werror
FLAGS_SHARED += -c $(WARNING_FLAGS) $(INCLUDES) $(DEBUG_FLAGS) $(UVUDEC_VER_FLAGS) $(OPTIMIZATION_LEVEL)
CCFLAGS += $(FLAGS_SHARED)
CXXFLAGS += $(FLAGS_SHARED)

LDFLAGS += -L$(LIB_DIR) -L.
LDFLAGS+=-L$(PLUGIN_LIB_DIR)

FLAGS_SHARED += -DDEFAULT_DECOMPILE_FILE=$(DEFAULT_DECOMPILE_FILE) -DDEFAULT_CPU_DIR=$(DEFAULT_CPU_DIR) -DDEFAULT_CPU_FILE=$(DEFAULT_CPU_FILE)
FLAGS_SHARED += -DUVD_INSTALL_DIR='"$(INSTALL_DIR)"'

# Name of the uvudec library
LIB_UVUDEC=libuvudec
# Full name of the version we are using (will link against)
#LIB_UVUDEC_FULL=libuvudec
# Leave out the patch, gives us a binary compatible version
#LIB_UVUDEC_DYNAMIC_USED=$(LIB_DIR)/$(LIB_UVUDEC).so.$(UVUDEC_VER_MAJOR).$(UVUDEC_VER_MINOR)
LIB_UVUDEC_DYNAMIC_USED=uvudec
LIB_UVUDEC_STATIC=$(LIB_DIR)/$(LIB_UVUDEC).a

# Experimental way to speed up printing
ifeq ($(USING_ROPE),Y)
FLAGS_SHARED += -DUSING_ROPE
endif

include $(ROOT_DIR)/Makefile.interpreter

# Need to make macro for this
ifeq ($(USING_LIB_UVUDEC),Y)
ifeq ($(LINKAGE),static)
LIBS += $(LIB_UVUDEC_STATIC)
else
LIBS += -l$(LIB_UVUDEC_DYNAMIC_USED)
endif
endif

ifeq ($(USING_LIBBFD),Y)
FLAGS_SHARED += -DUSING_LIBBFD
FLAGS_SHARED += -DUVD_FLIRT_PATTERN_BFD
ifdef BINUTILS_PREFIX
INCLUDES+=-I$(BINUTILS_PREFIX)/include
LDFLAGS+=-L$(BINUTILS_PREFIX)/lib
endif
LIBBFD_STATIC_LIB=-lbfd
LIBOPCODES_STATIC_LIB=-lopcodes
LIBIBERTY_STATIC_LIB=-liberty
LIBS+=$(LIBBFD_STATIC_LIB) $(LIBOPCODES_STATIC_LIB) $(LIBIBERTY_STATIC_LIB)

# Otherwise use system provided (doesn't build until install)
LIBS += -lbfd -lopcodes -liberty

endif


# Boost
ifeq ($(LINKAGE),static)
else
LIBS += -lboost_system -lboost_filesystem
endif


ifeq ($(USING_LIBZ),Y)
ifeq ($(LINKAGE),static)
# hmm why is this hard coded?
LIBZ_STATIC=/usr/lib/libz.a
LIBS += $(LIBZ_STATIC)
else
LIBS += -lz
endif
endif


ifeq ($(USING_LIB_JANSSON),Y)
# `pkg-config --cflags --libs jansson`
LIBS += -ljansson
FLAGS_SHARED += -DUSING_JANSSON
endif

ifeq ($(USING_PYTHON),Y)
INCLUDES += -I$(PYTHON_INCLUDE_DIR)
endif

# General libc stuff
ifeq ($(LINKAGE),static)

# We only include libs in the final exe for static buids usually
LIBS +=
# And since we don't link until final exe, don't set any LDFLAGS
LDFLAGS +=

else

# We must incrementally link as we go along
LIBS += -lc -lm -lgcc
#LIBS += -lstdc++ -lgcc
LDFLAGS +=

endif

ifeq ($(LINKAGE),static)
OBJECT_LINKAGE_SUFFIX=_s
FLAGS_SHARED += -DUVD_STATIC
else
OBJECT_LINKAGE_SUFFIX=_d
FLAGS_SHARED += -DUVD_DYNAMIC
endif
OBJS = $(CC_SRCS:.c=$(OBJECT_LINKAGE_SUFFIX).o) $(CXX_SRCS:.cpp=$(OBJECT_LINKAGE_SUFFIX).o)

UVUDEC_EXE = $(BIN_DIR)/uvudec
COFF2PAT_EXE = $(BIN_DIR)/uvcoff2pat
GUI_EXE = $(BIN_DIR)/uvdgui
BIN2OBJ_EXE = $(BIN_DIR)/uvbin2obj
OBJ2PAT_EXE = $(BIN_DIR)/uvobj2pat
OMF2PAT_EXE = $(BIN_DIR)/uvomf2pat
ELF2PAT_EXE = $(BIN_DIR)/uvelf2pat
PAT2SIG_EXE = $(BIN_DIR)/uvpat2sig
SIGUTIL_EXE = $(BIN_DIR)/uvsigutil
EXES= $(UVUDEC_EXE) $(COFF2PAT_EXE) $(OMF2PAT_EXE) $(ELF2PAT_EXE) $(PAT2SIG_EXE)
TESTING_EXE = $(BIN_DIR)/uvtest

ifdef EXE_NAME
include $(ROOT_DIR)/Makefile.exe
endif

ifdef LIB_NAME
include $(ROOT_DIR)/Makefile.lib
endif

# BEGIN TARGETS

all: $(ALL_TARGETS)
	@(echo 'all: done')
	@(true)

# FIXME: deal with this better
$(ROOT_DIR)/libuvudec/uvd/config.h: $(ROOT_DIR)/Makefile.configure
	touch $(ROOT_DIR)/libuvudec/uvd/config.h

$(BIN_DIR):
	mkdir $(BIN_DIR)

%$(OBJECT_LINKAGE_SUFFIX).o: %.c
	$(CC) $(CCFLAGS) $< -o $@

%$(OBJECT_LINKAGE_SUFFIX).o: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

cleanLocal:
	$(RM) *.o *~ $(OBJS) Makefile.bak core.* Makefile.depend*

CLEAN_DEPS+=cleanLocal
cleanTarget: $(CLEAN_DEPS)
	@(true)

# Sudsy soap would be proud!
clean:
	$(MAKE) TARGET=static cleanTarget
	$(MAKE) TARGET=dynamic cleanTarget

ifdef COMPILING_CODE
MAKEFILE_DEPEND=Makefile.depend$(OBJECT_LINKAGE_SUFFIX)
$(shell touch $(MAKEFILE_DEPEND))
include $(MAKEFILE_DEPEND)

ifdef MAKEDEPEND
# Silicenced because they started to take up a lot of screen during each build
# Ignore cannot find stdio.h stuff
depend:
#$(MAKEDEPEND) -f$(MAKEFILE_DEPEND) -Y $(CCFLAGS) $(CC_SRCS) $(CXX_SRCS)
	@($(MAKEDEPEND) -f$(MAKEFILE_DEPEND) -Y $(CCFLAGS) $(CC_SRCS) $(CXX_SRCS) 2>/dev/null >/dev/null)
	perl -pi -e 's/[.]o/$(OBJECT_LINKAGE_SUFFIX).o/g' $(MAKEFILE_DEPEND)
#	$(MAKEDEPEND) -f$(MAKEFILE_DEPEND) -Y $(CCFLAGS) $(CC_SRCS) $(CXX_SRCS) 2>/dev/null >/dev/null
# Remove annoying backup
	@($(RM) $(MAKEFILE_DEPEND).bak)
endif
endif

PHONY += all .c.o .cpp.o clean dist depend info cleanLocal
.PHONY: $(PHONY)

info: $(INFO_TARGETS)
	@(echo "Shared info")
	@(echo "VERSION: $(PACKAGE_VERSION)")
	@(echo "USING_UPX: $(USING_UPX)")
	@(echo "LINKAGE: $(LINKAGE)")
	@(echo "USING_STATIC: $(USING_STATIC)")
	@(echo "USING_DYNAMIC: $(USING_DYNAMIC)")
	@(echo "USING_PYTHON: $(USING_PYTHON)")
	@(echo "USING_LUA: $(USING_LUA)")
	@(echo "USING_JAVASCRIPT: $(USING_JAVASCRIPT)")
	@(echo "USING_SPIDERAPE: $(USING_SPIDERAPE)")
	@(echo "USING_SPIDERMONKEY: $(USING_SPIDERMONKEY)")
	@(echo "")
	@(echo "ROOT_DIR: $(ROOT_DIR)")
	@(echo "PHONY: $(PHONY)")
	@(echo "LIBS: $(LIBS)")
	@(echo "LDFLAGS: $(LDFLAGS)")
	@(echo "INCLUDES: $(INCLUDES)")
	@(echo "ALL_TARGETS:$(ALL_TARGETS)")
	@(echo "CXX:$(CXX)")
	@(echo "")
	@(echo "OBJS: $(OBJS)")
	
