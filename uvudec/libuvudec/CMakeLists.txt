cmake_minimum_required (VERSION 2.6)
project (libuvudec)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../lib)

configure_file (
  "${PROJECT_SOURCE_DIR}/uvd/config.h.in"
  "${PROJECT_BINARY_DIR}/uvd/config.h"
)


# By default the executable file will be created in the build tree directory
# corresponding to the source tree directory in which the command was invoked.
# See the RUNTIME_OUTPUT_DIRECTORY target property to change this location
# See documentation of the OUTPUT_NAME target property to change the <name> part of the final file name. 

add_library(libuvudec SHARED
	uvd/architecture/architecture.cpp
	uvd/architecture/factory.cpp
	uvd/architecture/registry.cpp
	uvd/architecture/std_iter_factory.cpp
	uvd/assembly/address.cpp
	uvd/assembly/function.cpp
	uvd/assembly/symbol.cpp
	uvd/assembly/cpu.cpp
	uvd/assembly/cpu_vector.cpp
	uvd/assembly/instruction.cpp
	uvd/assembly/operand.cpp
	uvd/assembly/register.cpp
	uvd/compiler/compiler.cpp
	uvd/compiler/assembly.cpp
	uvd/compiler/gcc.cpp
	uvd/config/arg.cpp
	uvd/config/arg_early.cpp
	uvd/config/arg_util.cpp
	uvd/config/config.cpp
	uvd/config/config_symbols.cpp
	uvd/config/file.cpp
	uvd/config/plugin.cpp
	uvd/config/raw_arg.cpp
	uvd/core/analysis.cpp
	uvd/core/analyzer.cpp
	uvd/core/as_instruction_iterator.cpp
	uvd/core/block.cpp
	uvd/core/event.cpp
	uvd/core/init.cpp
	uvd/core/instruction_iterator.cpp
	uvd/core/print_iterator.cpp
	uvd/core/runtime.cpp
	uvd/core/runtime_hints.cpp
	uvd/core/std_instruction_iterator.cpp
	uvd/core/std_print_iterator.cpp
	uvd/core/uvd.cpp
	uvd/data/data.cpp
	uvd/data/chunk.cpp
	uvd/data/file.cpp
	uvd/data/memory.cpp
	uvd/data/placeholder.cpp
	uvd/event/engine.cpp
	uvd/event/event.cpp
	uvd/hash/crc.cpp
	uvd/hash/md5.cpp
	uvd/language/format.cpp
	uvd/language/language.cpp
	uvd/plugin/engine.cpp
	uvd/plugin/plugin.cpp
	uvd/object/object.cpp
	uvd/object/section.cpp
	uvd/relocation/data.cpp
	uvd/relocation/element.cpp
	uvd/relocation/relocation.cpp
	uvd/relocation/fixup.cpp
	uvd/relocation/manager.cpp
	uvd/relocation/writer.cpp
	uvd/relocation/simple_fixup.cpp
	uvd/string/analyzer.cpp
	uvd/string/engine.cpp
	uvd/string/string.cpp
	uvd/util/io.cpp
	uvd/util/benchmark.cpp
	uvd/util/config_section.cpp
	uvd/util/curses.cpp
	uvd/util/debug.cpp
	uvd/util/error.cpp
	uvd/util/log.cpp
	uvd/util/priority_list.cpp
	uvd/util/string_writer.cpp
	uvd/util/types.cpp
	uvd/util/util.cpp
	uvd/util/string.cpp
	uvd/util/file.cpp
	uvd/util/version.cpp
)

include_directories("${PROJECT_BINARY_DIR}")

