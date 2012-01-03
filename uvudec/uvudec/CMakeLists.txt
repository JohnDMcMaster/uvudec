cmake_minimum_required (VERSION 2.6)
project (uvudec)

include_directories ("${PROJECT_SOURCE_DIR}/../libuvudec")
include_directories ("${PROJECT_SOURCE_DIR}/../plugin")
include_directories ("${PROJECT_SOURCE_DIR}/..")

link_directories( ../lib )
link_directories( ../lib/plugin )

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin)

add_executable(uvudec_exe
	main.cpp
)

include_directories("${PROJECT_BINARY_DIR}")
target_link_libraries (uvudec_exe uvudec boost_filesystem dl)

