cmake_minimum_required (VERSION 2.6)
project (uvtest)

# By default the executable file will be created in the build tree directory
# corresponding to the source tree directory in which the command was invoked.
# See the RUNTIME_OUTPUT_DIRECTORY target property to change this location
# See documentation of the OUTPUT_NAME target property to change the <name> part of the final file name. 

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

