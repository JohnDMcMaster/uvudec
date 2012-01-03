cmake_minimum_required (VERSION 2.6)
project (uvudec)

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

