cmake_minimum_required(VERSION 2.8)
project(silicium)

if(${CMAKE_SYSTEM} MATCHES "Linux")
	set(SILICIUM_LINUX ON)
endif()

if(UNIX)
	add_definitions("-Wall -Wextra -pedantic -std=c++1y -Wconversion -Wvla")
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
if(LIBGIT2_FOUND)
	include_directories(${LIBGIT2_INCLUDE_DIR})
endif()

find_package(SDL2)
if(SDL2_FOUND)
	include_directories(${LIBSDL2_INCLUDE_DIR})
endif()

find_package(Lua51)
if(LUA51_FOUND)
	include_directories(${LUA_INCLUDE_DIR})
endif()

include_directories(".")
add_subdirectory("silicium")
add_subdirectory("reactive")
add_subdirectory("test")
add_subdirectory("examples")
