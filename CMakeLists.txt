cmake_minimum_required (VERSION 3.21.0)
set( CMAKE_CXX_STANDARD 17 )

project (ndGPUMemory)

add_definitions( -Wall -Wextra -pedantic )
set( CMAKE_CXX_FLAGS -ggdb )

add_subdirectory (lib) 
add_subdirectory (exe)

set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)
set(CPACK_GENERATOR "TBZ2")
set(CPACK_STRIP_FILES TRUE )
include(CPack)
