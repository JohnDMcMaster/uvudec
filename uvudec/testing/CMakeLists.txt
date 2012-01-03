cmake_minimum_required (VERSION 2.6)
project (uvtest)

set (uvudec_VERSION_MAJOR 0)
set (uvudec_VERSION_MINOR 4)
set (uvudec_VERSION_PATCH 0)

set (uvudec_DEFAULT_DECOMPILE_FILE "//home/mcmaster/document/build/uvudec/candela_pltl1_rev_3.bin")

configure_file (
  "${PROJECT_SOURCE_DIR}/unit_config.h.in"
  "${PROJECT_BINARY_DIR}/unit_config.h"
  )

include_directories ("${PROJECT_SOURCE_DIR}/../libuvudec")
include_directories ("${PROJECT_SOURCE_DIR}/../plugin")
include_directories ("${PROJECT_SOURCE_DIR}/..")
#add_subdirectory (MathFunctions) 

link_directories( ../lib )
link_directories( ../lib/plugin )
# By default the executable file will be created in the build tree directory
# corresponding to the source tree directory in which the command was invoked.
# See the RUNTIME_OUTPUT_DIRECTORY target property to change this location
# See documentation of the OUTPUT_NAME target property to change the <name> part of the final file name. 

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin)

add_executable(uvtest
	framework/common_fixture.cpp
	framework/plugin.cpp
	framework/progress_listener.cpp
	framework/serialized_test_result.cpp
	assembly.cpp
	block.cpp
	flirt.cpp
	flirtutil.cpp
	flirtutil_main_hook.cpp
	libuvudec.cpp
	object.cpp
	main.cpp
	obj2pat.cpp
	obj2pat_main_hook.cpp
	pat2sig.cpp
	pat2sig_main_hook.cpp
	uvdobjgb.cpp
	uvudec.cpp
	uvudec_main_hook.cpp           
)

include_directories("${PROJECT_BINARY_DIR}")
#nbadirective( asfddsf )
target_link_libraries (uvtest uvudec uvdflirt uvdgb uvdobjbin cppunit boost_filesystem)

