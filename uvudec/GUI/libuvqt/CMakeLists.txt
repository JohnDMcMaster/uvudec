cmake_minimum_required (VERSION 2.6)
project (libuvqt)

include_directories ("${PROJECT_SOURCE_DIR}/../../libuvudec")
include_directories ("${PROJECT_SOURCE_DIR}/../../plugin")
include_directories ("${PROJECT_SOURCE_DIR}/../..")

link_directories( ../../lib )
link_directories( ../../lib/plugin )

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../../lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../../lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../../bin)

option(USE_MYMATH
  "Use tutorial provided math implementation" OFF)
if (USE_MYMATH)
	message("suck it trebek")
endif (USE_MYMATH)

find_package(Qt4 COMPONENTS QtCore QtGui QtXml QtDesigner REQUIRED )
include(${QT_USE_FILE})

SET(libuvqt_SOURCES
	uvqt/dynamic_text.cpp
	uvqt/dynamic_text_data.cpp
	uvqt/dynamic_text_plugin.cpp
	uvqt/dynamic_text_plugin_impl.cpp
	uvqt/hexdump.cpp
	uvqt/hexdump_data.cpp
	uvqt/hexdump_iterator.cpp
	uvqt/hexdump_plugin.cpp
	uvqt/plain_text_edit.cpp
	uvqt/plugin.cpp
	uvqt/scrollable_dynamic_text.cpp
	uvqt/util.cpp
)
SET(libuvqt_HEADERS
	uvqt/dynamic_text.h
	uvqt/dynamic_text_plugin.h
	uvqt/dynamic_text_plugin_impl.h
	uvqt/hexdump.h
	uvqt/hexdump_data.h
	uvqt/hexdump_plugin.h
	uvqt/plain_text_edit.h
	uvqt/plugin.h
)

ADD_DEFINITIONS(${QT_DEFINITIONS})

QT4_WRAP_CPP(libuvqt_HEADERS_MOC ${libuvqt_HEADERS})



add_library(uvqt SHARED
	${libuvqt_SOURCES} 
    ${libuvqt_HEADERS_MOC}
)

#target_link_libraries(uvqt ${QT_LIBRARIES})


include_directories("${PROJECT_BINARY_DIR}")
include_directories("${QT_PLUGINS_DIR}")
include_directories("${QT_QTDESIGNERCOMPONENTS_INCLUDE_DIR}")


#target_link_libraries (uvqt uvudec boost_filesystem dl)

