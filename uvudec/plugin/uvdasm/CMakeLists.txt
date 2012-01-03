cmake_minimum_required (VERSION 2.6)
project (uvudec)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../../lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../../lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../../bin)

set (libuvdasm_DEFAULT_CPU_DIR arch)
set (libuvdasm_DEFAULT_CPU_FILE 8051/8051.op)
set (libuvdasm_UVD_CONFIG_INTERPRETER_LANGUAGE_DEFAULT UVD_LANGUAGE_PYTHON)
set (libuvdasm_UVD_CONFIG_INTERPRETER_LANGUAGE_INTERFACE_DEFAULT UVD_LANGUAGE_INTERFACE_API)

configure_file (
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
)

include_directories ("${PROJECT_SOURCE_DIR}/../../libuvudec")
include_directories ("${PROJECT_SOURCE_DIR}/../../plugin")
include_directories ("${PROJECT_SOURCE_DIR}/../..")

link_directories( ../../lib )
link_directories( ../../lib/plugin )

add_library(libuvdasm SHARED
	flirt/flirt.cpp
	architecture.cpp
	architecture_init.cpp
	plugin_config.cpp
	config_symbol.cpp
	function.cpp
	instruction.cpp
	interpreter.cpp
	main.cpp
	operand.cpp
	opcode_table.cpp
	plugin.cpp
	util.cpp
	interpreter/interpreter.cpp
	interpreter/javascript.cpp
	interpreter/javascript_spiderape.cpp
	interpreter/javascript_spidermonkey.cpp
	interpreter/lua.cpp
	interpreter/operator.cpp
	interpreter/python.cpp
	interpreter/python_api.cpp
	interpreter/python_exec.cpp
)

add_definitions(-DUVD_PLUGIN_NAME=\"uvdasm\")

include_directories("${PROJECT_BINARY_DIR}")

