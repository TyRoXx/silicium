cmake_minimum_required(VERSION 2.8)

if(UNIX)
	add_definitions("-Wall -Wextra -pedantic -std=c++0x -Wconversion -Wvla")
endif()

find_package(Boost REQUIRED unit_test_framework filesystem iostreams system)
if(UNIX)
	add_definitions("-DBOOST_TEST_DYN_LINK")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
find_package(Libgit2)

include_directories(".")
add_subdirectory("silicium")
add_subdirectory("test")
add_subdirectory("examples")
