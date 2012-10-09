cmake_minimum_required (VERSION 2.6)
project (uvudec)

find_package( Boost 1.48.0 REQUIRED )

#set (MY_BOOST_DIR "/home/mcmaster/document/build/boost/boost_1_48_0")
#set (BOOST_ROOT "${MY_BOOST_DIR}")
#set (BOOST_INCLUDEDIR "${MY_BOOST_DIR}")
#set (BOOST_LIBRARYDIR "${MY_BOOST_DIR}/stage/lib")


#include_directories ("/home/mcmaster/document/build/boost/boost_1_48_0")
#link_directories( "/home/mcmaster/document/build/boost/boost_1_48_0/stage/lib" )

include_directories ("${PROJECT_SOURCE_DIR}/libuvudec")
include_directories ("${PROJECT_SOURCE_DIR}/plugin")
include_directories ("${PROJECT_SOURCE_DIR}")

include_directories(${Boost_INCLUDE_DIRS})


link_directories( "${PROJECT_SOURCE_DIR}/lib" )
link_directories( "${PROJECT_SOURCE_DIR}/lib/plugin" )

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin" )

set (uvudec_VERSION_MAJOR 0)
set (uvudec_VERSION_MINOR 4)
set (uvudec_VERSION_PATCH 0)

set (uvudec_DEFAULT_DECOMPILE_FILE "/home/mcmaster/document/build/uvudec/candela_pltl1_rev_3.bin")


add_subdirectory (libuvudec)
add_subdirectory (plugin)
#add_subdirectory (python)

add_subdirectory (testing)

add_subdirectory (uvudec)
add_subdirectory (GUI)

add_subdirectory (bin2obj)
add_subdirectory (obj2pat)
add_subdirectory (pat2sig)
add_subdirectory (flirtutil)

