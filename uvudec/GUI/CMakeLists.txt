cmake_minimum_required (VERSION 2.6)
project (uvudec)

link_directories( libuvqt )

find_package(Qt4 COMPONENTS QtCore QtGui QtXml REQUIRED )
include(${QT_USE_FILE})

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin)

FIND_PACKAGE(Qt4 REQUIRED)

add_executable(uvdgui_exe
	analysis_action.cpp
	analysis_thread.cpp
	analysis_thread.moc.cpp
	assembly_data.cpp
	assembly_data_iterator.cpp
	format.cpp
	GUI.cpp
	GUI.moc.cpp
	lock.cpp
	main.cpp
	project.cpp
	string_data.cpp
	string_data_iterator.cpp
)

include_directories("${PROJECT_BINARY_DIR}")
include_directories("libuvqt")
target_link_libraries (uvdgui_exe uvudec boost_filesystem dl)

target_link_libraries (uvdgui_exe uvqt)

target_link_libraries(uvdgui_exe ${QT_LIBRARIES})
include_directories("${QT_QTDESIGNERCOMPONENTS_INCLUDE_DIR}")
include_directories("${QT_QTGUI_INCLUDE_DIR}")
include_directories("${QT_QTCORE_INCLUDE_DIR}")


