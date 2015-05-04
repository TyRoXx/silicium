cmake_minimum_required(VERSION 2.8)
project(silicium)

if(${CMAKE_SYSTEM} MATCHES "Linux")
	set(SILICIUM_LINUX ON)
endif()

if(UNIX)
	if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		add_definitions("-std=c++1y")
	else()
		execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
		if(GCC_VERSION VERSION_GREATER 4.7)
			add_definitions("-std=c++1y")
		else()
			add_definitions("-std=c++0x")
		endif()
	endif()

	add_definitions("-Wall -Wextra -Wconversion -Wvla")

	#warn when using undefined macros
	add_definitions("-Wundef")

	option(SILICIUM_PEDANTIC "pass -pedantic to the compiler (may generate useless warnings)" OFF)
	if(SILICIUM_PEDANTIC)
		add_definitions("-Wpedantic")
	endif()
endif()

if(MSVC)
	#make Visual C++ 2013 build in parallel
	add_definitions("/MP")

	#we want more warnings
	add_definitions("/W4")

	#disable "'std::_Copy_n': Function call with parameters that may be unsafe"
	add_definitions("/wd4996")

	#workaround for Boost 1.55 Context error LNK2026
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
endif()

if(MSVC)
	set(Boost_USE_STATIC_LIBS ON) 
endif()
if(NOT MSVC)
	set(SILICIUM_BOOST_REQUIRED REQUIRED)
endif()
if(BOOST_ROOT)
	set(Boost_NO_SYSTEM_PATHS ON)
endif()
find_package(Boost ${SILICIUM_BOOST_REQUIRED} COMPONENTS unit_test_framework thread filesystem iostreams chrono system)
if(NOT (Boost_VERSION LESS 105300))
	set(required_boost_libraries ${Boost_LIBRARIES})
	message(STATUS "trying to find Boost.Coroutine and Boost.Context..")
	find_package(Boost COMPONENTS coroutine context)
	set(Boost_LIBRARIES ${required_boost_libraries} ${Boost_LIBRARIES})
endif()
if(UNIX)
	add_definitions("-DBOOST_TEST_DYN_LINK")
endif()
include_directories(SYSTEM ${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})

if(WIN32)
	#Boost.Asio wants this for no reason
	add_definitions("-D_WIN32_WINDOWS")

	add_definitions("-DBOOST_ASIO_HAS_IOCP")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

find_package(SDL2)
if(SDL2_FOUND)
	include_directories(SYSTEM ${SDL2_INCLUDE_DIR})
endif()

find_package(Lua51)
if(LUA51_FOUND)
	include_directories(SYSTEM ${LUA_INCLUDE_DIR})
endif()

find_package(ZLIB REQUIRED)
if(ZLIB_FOUND)
	include_directories(SYSTEM ${ZLIB_INCLUDE_DIRS})
endif()

find_package(UriParser REQUIRED)
if(URIPARSER_FOUND)
	include_directories(SYSTEM ${URIPARSER_INCLUDE_DIR})
endif()

include_directories(".")
add_subdirectory("c_binding")
add_subdirectory("test")
add_subdirectory("test_includes")
add_subdirectory("examples")
