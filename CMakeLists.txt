cmake_minimum_required(VERSION 2.8)
project(silicium)

if(UNIX)
	add_definitions("-Wall -Wextra -pedantic -std=c++0x -Wconversion -Wvla")
endif()

if(MSVC)
	#make Visual C++ 2013 build in parallel
	add_definitions("/MP")

	#we want more warnings
	add_definitions("/W4")

	#workaround for Boost 1.55 Context error LNK2026
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
endif()

if(UNIX)
	find_package(Boost REQUIRED unit_test_framework filesystem iostreams system coroutine)
	add_definitions("-DBOOST_TEST_DYN_LINK")
endif()

if(WIN32)
	#Boost.Asio wants this for no reason
	add_definitions("-D_WIN32_WINDOWS")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
find_package(Libgit2)

include_directories(".")
add_subdirectory("silicium")
add_subdirectory("test")
add_subdirectory("examples")
