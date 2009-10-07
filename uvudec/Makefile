# Universal Decompiler (uvudec)
# Copyright 2008 John McMaster
# JohnDMcMaster@gmail.com
# Licensed under the terms of the BSD license.  See LICENSE for details.

# System defaults
include Makefile.defaults
# Optional "./configure" result
$(shell touch Makefile.configure)
include Makefile.configure

# Man
MAN_SECTION=3
MAN_RAW=manpage
MAN_PAGE=uvudec
MAN_UNCOMPRESSED=$(MAN_PAGE).$(MAN_SECTION)
MAN=$(MAN_UNCOMPRESSED).gz
MAN_DIR=$(PREFIX)/man/man$(MAN_SECTION)

# version stuff
UVUDEC_VER_MAJOR=0
UVUDEC_VER_MINOR=2
UVUDEC_VER_PATCH=0
UVUDEC_VER_BUILD=0
UVUDEC_VER=$(UVUDEC_VER_MAJOR).$(UVUDEC_VER_MINOR).$(UVUDEC_VER_PATCH).$(UVUDEC_VER_BUILD)
UVUDEC_VER_FLAGS=-DUVUDEC_VER_MAJOR=$(UVUDEC_VER_MAJOR) -DUVUDEC_VER_MINOR=$(UVUDEC_VER_MINOR) -DUVUDEC_VER_PATCH=$(UVUDEC_VER_PATCH) -DUVUDEC_VER_BUILD=$(UVUDEC_VER_BUILD)

PACKAGE=uvudec

INCLUDES += -I.
#OPTIMIZATION_LEVEL=-O3
DEBUG_FLAGS=-g
WARNING_FLAGS=-Wall -Werror
FLAGS_SHARED = -c $(WARNING_FLAGS) $(INCLUDES) $(DEBUG_FLAGS) $(UVUDEC_VER_FLAGS) $(OPTIMIZATION_LEVEL)
CCFLAGS = $(FLAGS_SHARED)
CXXFLAGS = $(FLAGS_SHARED)

LFLAGS=

FLAGS_SHARED += -DDEFAULT_DECOMPILE_FILE=$(DEFAULT_DECOMPILE_FILE)

ifeq ($(USING_STATIC),Y)
$(shell rm -f libc.a)
LIB_C_STATIC=$(shell $(CXX) -print-file-name=libc.a)
$(shell ln -s $(LIB_C_STATIC))

$(shell rm -f libstdc++.a)
LIB_STDCXX_STATIC=$(shell $(CXX) -print-file-name=libstdc++.a)
$(shell ln -s $(LIB_STDCXX_STATIC))

$(shell rm -f libm.a)
LIB_M_STATIC=$(shell $(CXX) -print-file-name=libm.a)
$(shell ln -s $(LIB_M_STATIC))

LIBS += $(LIB_M_STATIC) $(LIB_STDCXX_STATIC) $(LIB_C_STATIC)
LFLAGS := $(LFLAGS) -L. -static-libgcc -Wl,'-z nodefaultlib'
else
LIBS += -lstdc++
endif

# Experimental way to speed up printing
ifeq ($(USING_ROPE),Y)
FLAGS_SHARED += -DUSING_ROPE
endif

# Lua stuff
ifeq ($(USING_LUA),Y)
FLAGS_SHARED += -DUSING_LUA
LUA_INCLUDE=$(LUA_DIR)/src
LUA_LIB_STATIC=$(LUA_DIR)/src/liblua.a
INCLUDES += -I$(LUA_INCLUDE)
LIBS += $(LUA_LIB_STATIC)
endif

# Python stuff
# This may get more complicated if I can get the APIs working better
ifeq ($(USING_PYTHON),Y)
FLAGS_SHARED += -DUSING_PYTHON
endif

# Javascript support
USING_JAVASCRIPT=N
ifeq ($(USING_SPIDERAPE),Y)
USING_JAVASCRIPT=Y
else
USING_SPIDERAPE=N
endif
ifeq ($(USING_SPIDERMONKEY),Y)
USING_JAVASCRIPT=Y
else
USING_SPIDERMONKEY=N
endif
# Now do actual USING_JAVASCRIPT effects
ifeq ($(USING_JAVASCRIPT),Y)
FLAGS_SHARED += -DUSING_JAVASCRIPT
endif

# SpiderApe stuff (a javascript engine)
SPIDERAPE_STATIC=$(SPIDERAPE_DIR)/src/ape/libSpiderApe.a
JS_STATIC=$(SPIDERAPE_DIR)/src/js/Linux_All_DBG.OBJ/libjs.a
ifeq ($(USING_SPIDERAPE),Y)
FLAGS_SHARED += -DUSING_SPIDERAPE
INCLUDES += -I$(SPIDERAPE_DIR)/include -I$(SPIDERAPE_DIR)/src/js -I$(SPIDERAPE_DIR)/src/ape 
LIBS += $(SPIDERAPE_STATIC) $(JS_STATIC)
# FIXME: this breaks static linkage
# fixed: disable plugins
# LIBS +=  -lltdl
endif


CC_SRCS =   \

CXX_SRCS = 	\
			uvd_data.cpp \
			main.cpp \
			uv_debug.cpp \
			uv_error.cpp \
			uv_log.cpp \
			uv_md5.cpp \
			uv_md5_GNU.cpp \
			uv_util.cpp \
			uvd.cpp \
			uvd_address.cpp \
			uvd_analysis.cpp \
			uvd_analysis_db.cpp \
			uvd_analyzer.cpp \
			uvd_benchmark.cpp \
			uvd_compiler.cpp \
			uvd_config.cpp \
			uvd_config_interpreter.cpp \
			uvd_elf.cpp \
			uvd_format.cpp \
			uvd_function.cpp \
			uvd_language.cpp \
			uvd_init.cpp \
			uvd_instruction.cpp \
			uvd_iterator.cpp \
			uvd_opcode.cpp \
			uvd_operand.cpp \
			uvd_operator.cpp \
			uvd_register.cpp \
			uvd_relocation.cpp \
			uvd_symbol.cpp \
			uvd_types.cpp \
			uvd_version.cpp \
			interpreter/uvd_interpreter.cpp \
			interpreter/uvd_javascript.cpp \
			interpreter/uvd_javascript_spiderape.cpp \
			interpreter/uvd_javascript_spidermonkey.cpp \
			interpreter/uvd_lua.cpp \
			interpreter/uvd_python.cpp \

OBJS = $(CC_SRCS:.c=.o) $(CXX_SRCS:.cpp=.o)
EXE = uvudec

# BEGIN TARGETS

all: uv_udec

uv_udec: depend $(OBJS) 
	$(CC) $(LFLAGS) $(OBJS) $(LIBS) $(LIBS) -o $(EXE)
ifeq ($(USING_UPX),Y)
	$(UPX) $(EXE)
endif

.c.o:
	$(CC) $(CCFLAGS) $< -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	$(RM) *.o *~ $(EXE) $(OBJS) uv_log.txt Makefile.bak core.*

ifeq ($(USING_STATIC),Y)
installer:
	
endif

dist: clean
	@(echo "Distributing...")
	$(RM) -r $(PACKAGE)-$(UVUDEC_VER)*
	$(MKDIR) $(PACKAGE)-$(UVUDEC_VER)
	$(CP) -r *.cpp *.h Makefile test arch doc $(MAN_RAW) $(PACKAGE)-$(UVUDEC_VER)
	$(RM) -r $(PACKAGE)-$(UVUDEC_VER)/.svn
	$(TAR) -cvf $(PACKAGE)-$(UVUDEC_VER).tar $(PACKAGE)-$(UVUDEC_VER)
	$(GZIP) $(PACKAGE)-$(UVUDEC_VER).tar
	$(RM) -r $(PACKAGE)-$(UVUDEC_VER)

MAKEFILE_DEPEND=Makefile.depend
$(shell touch $(MAKEFILE_DEPEND))
include $(MAKEFILE_DEPEND)

# Ignore cannot find stdio.h stuff
depend:
	$(MAKEDEPEND) -f$(MAKEFILE_DEPEND) -Y $(CCFLAGS) $(CC_SRCS) $(CXX_SRCS) 2>/dev/null >/dev/null
# Remove annoying backup
	$(RM) $(MAKEFILE_DEPEND).bak
	
.PHONY: all uv_udec .cpp.o clean dist depend

info:
	@(echo "USING_UPX: $(USING_UPX)")
	@(echo "USING_STATIC: $(USING_STATIC)")
	@(echo "USING_PYTHON: $(USING_PYTHON)")
	@(echo "USING_LUA: $(USING_LUA)")
	@(echo "USING_JAVASCRIPT: $(USING_JAVASCRIPT)")
	@(echo "USING_SPIDERAPE: $(USING_SPIDERAPE)")
	@(echo "USING_SPIDERMONKEY: $(USING_SPIDERMONKEY)")
	@(echo "")
	@(echo "LIBS: $(LIBS)")
	@(echo "LFLAGS: $(LFLAGS)")
