cmake_minimum_required (VERSION 2.6)
project (uvudec)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../../lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../../lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../../bin)

include_directories ("${PROJECT_SOURCE_DIR}/../../libuvudec")
include_directories ("${PROJECT_SOURCE_DIR}/../../plugin")
include_directories ("${PROJECT_SOURCE_DIR}/../..")

link_directories( ../../lib )
link_directories( ../../lib/plugin )

add_library(uvdgb SHARED
#	architecture.cpp
#	instruction.cpp
	main.cpp
	object.cpp
	plugin.cpp
)

add_definitions(-DUVD_PLUGIN_NAME=\"uvdgb\")

include_directories("${PROJECT_BINARY_DIR}")

