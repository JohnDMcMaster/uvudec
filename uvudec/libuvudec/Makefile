# UVNet Universal Decompiler (uvudec)
# Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
# Licensed under terms of the three clause BSD license, see LICENSE for details

LIB_NAME=$(LIB_UVUDEC)

CC_SRCS =   \

UVD_DIR=$(LIBUVUDEC_DIR)/uvd

REAL_CXX_SRCS = 	\
			$(UVD_DIR)/architecture/architecture.cpp \
			$(UVD_DIR)/assembly/address.cpp \
			$(UVD_DIR)/assembly/function.cpp \
			$(UVD_DIR)/assembly/symbol.cpp \
			$(UVD_DIR)/assembly/cpu.cpp \
			$(UVD_DIR)/assembly/cpu_vector.cpp \
			$(UVD_DIR)/assembly/instruction.cpp \
			$(UVD_DIR)/assembly/operand.cpp \
			$(UVD_DIR)/assembly/register.cpp \
			$(UVD_DIR)/compiler/compiler.cpp \
			$(UVD_DIR)/compiler/assembly.cpp \
			$(UVD_DIR)/compiler/gcc.cpp \
			$(UVD_DIR)/config/arg.cpp \
			$(UVD_DIR)/config/arg_early.cpp \
			$(UVD_DIR)/config/arg_util.cpp \
			$(UVD_DIR)/config/config.cpp \
			$(UVD_DIR)/config/config_symbols.cpp \
			$(UVD_DIR)/config/file.cpp \
			$(UVD_DIR)/config/plugin.cpp \
			$(UVD_DIR)/core/runtime.cpp \
			$(UVD_DIR)/core/runtime_hints.cpp \
			$(UVD_DIR)/core/uvd.cpp \
			$(UVD_DIR)/core/analysis.cpp \
			$(UVD_DIR)/core/analysis_db.cpp \
			$(UVD_DIR)/core/analyzer.cpp \
			$(UVD_DIR)/core/event.cpp \
			$(UVD_DIR)/core/init.cpp \
			$(UVD_DIR)/core/iterator.cpp \
			$(UVD_DIR)/data/data.cpp \
			$(UVD_DIR)/data/chunk.cpp \
			$(UVD_DIR)/data/file.cpp \
			$(UVD_DIR)/data/memory.cpp \
			$(UVD_DIR)/data/placeholder.cpp \
			$(UVD_DIR)/event/engine.cpp \
			$(UVD_DIR)/event/event.cpp \
			$(UVD_DIR)/elf/elf.cpp \
			$(UVD_DIR)/elf/data.cpp \
			$(UVD_DIR)/elf/header.cpp \
			$(UVD_DIR)/elf/init.cpp \
			$(UVD_DIR)/elf/relocation.cpp \
			$(UVD_DIR)/elf/string_table.cpp \
			$(UVD_DIR)/elf/symbol.cpp \
			$(UVD_DIR)/elf/writer.cpp \
			$(UVD_DIR)/flirt/config.cpp \
			$(UVD_DIR)/flirt/lib/bfd.cpp \
			$(UVD_DIR)/flirt/lib/elf.cpp \
			$(UVD_DIR)/flirt/lib/lib.cpp \
			$(UVD_DIR)/flirt/args.cpp \
			$(UVD_DIR)/flirt/flirt.cpp \
			$(UVD_DIR)/flirt/function.cpp \
			$(UVD_DIR)/flirt/pat/factory.cpp \
			$(UVD_DIR)/flirt/pat/pat.cpp \
			$(UVD_DIR)/flirt/sig/tree/basic_node.cpp \
			$(UVD_DIR)/flirt/sig/tree/hash_node.cpp \
			$(UVD_DIR)/flirt/sig/tree/leading_node.cpp \
			$(UVD_DIR)/flirt/sig/format.cpp \
			$(UVD_DIR)/flirt/sig/sig.cpp \
			$(UVD_DIR)/flirt/sig/writer.cpp \
			$(UVD_DIR)/hash/crc.cpp \
			$(UVD_DIR)/hash/md5.cpp \
			$(UVD_DIR)/language/format.cpp \
			$(UVD_DIR)/language/language.cpp \
			$(UVD_DIR)/plugin/engine.cpp \
			$(UVD_DIR)/plugin/plugin.cpp \
			$(UVD_DIR)/object/object.cpp \
			$(UVD_DIR)/object/section.cpp \
			$(UVD_DIR)/relocation/data.cpp \
			$(UVD_DIR)/relocation/element.cpp \
			$(UVD_DIR)/relocation/relocation.cpp \
			$(UVD_DIR)/relocation/fixup.cpp \
			$(UVD_DIR)/relocation/manager.cpp \
			$(UVD_DIR)/relocation/writer.cpp \
			$(UVD_DIR)/relocation/simple_fixup.cpp \
			$(UVD_DIR)/util/io.cpp \
			$(UVD_DIR)/util/ascii_art.cpp \
			$(UVD_DIR)/util/benchmark.cpp \
			$(UVD_DIR)/util/debug.cpp \
			$(UVD_DIR)/util/error.cpp \
			$(UVD_DIR)/util/log.cpp \
			$(UVD_DIR)/util/priority_list.cpp \
			$(UVD_DIR)/util/string_writer.cpp \
			$(UVD_DIR)/util/types.cpp \
			$(UVD_DIR)/util/util.cpp \
			$(UVD_DIR)/util/string.cpp \
			$(UVD_DIR)/util/file.cpp \
			$(UVD_DIR)/util/version.cpp \

CXX_SRCS = $(REAL_CXX_SRCS) \
			$(UVD_DIR)/all.cpp \

ALL_TARGETS += $(UVD_DIR)/all.h

ROOT_DIR=..
# Suppose to be able to find makefile in dir above...but doesn't seem to
include $(ROOT_DIR)/Makefile.mk

# Dependency isn't quite right, but its close enough
# Chances are every usable .h file was included by some source file
uvd/all.h: $(UVD_DIR)/all.h
$(UVD_DIR)/all.h: $(REAL_CXX_SRCS)
	echo -n "" >$(UVD_DIR)/all.h
	echo "/*" >>$(UVD_DIR)/all.h
	echo "UVNet Universal Decompiler (uvudec)" >>$(UVD_DIR)/all.h
	echo "Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>" >>$(UVD_DIR)/all.h
	echo "Licensed under the terms of the LGPL V3 or later, see COPYING for details" >>$(UVD_DIR)/all.h
	echo "*/" >>$(UVD_DIR)/all.h
	echo "" >>$(UVD_DIR)/all.h
	echo "//This file was generated automatically " $(shell date) >>$(UVD_DIR)/all.h
	
	find -name '*.h' -exec echo '#include "{}"' ';' |fgrep -v all.h |sed sX./uvd/Xuvd/Xg >>$(UVD_DIR)/all.h

