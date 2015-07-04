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
		add_definitions("-DSILICIUM_PEDANTIC")
	endif()
endif()

if(MSVC)
	#make Visual C++ 2013 build in parallel
	add_definitions("/MP")

	#we want more warnings
	add_definitions("/W4")

	add_definitions("-DBOOST_ALL_NO_LIB")
	
	#Boost Asio uses deprecated Winsock APIs, but we are not interested in warnings about that
	add_definitions("-D_WINSOCK_DEPRECATED_NO_WARNINGS")

	#SILICIUM_TRAIT triggers warnings about missing arguments for the macro BOOST_PP_EXPAND_I (maybe a VC++ 2013 bug?)
	add_definitions("/wd4003")

	#Boost future has unreachable code for no reason
	add_definitions("/wd4702")
	
	#Boost in_place triggers /W4 warning about an assignment operator that could not be generated
	add_definitions("/wd4512")

	#stupid warning about std::copy
	add_definitions("/wd4996")

	#workaround for Boost 1.55 Context error LNK2026
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")

	if(MSVC14)
		#Boost 1.58 Asio has a macro redefinition on Visual Studio 2015
		add_definitions("/wd4005")

		#unrestricted union bug
		add_definitions("/wd4624")

		#Boost 1.58 Coroutine warning about throw in a noexcept function
		add_definitions("/wd4297")
	endif()

	add_definitions("-DBOOST_ASIO_NO_DEPRECATED")
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
find_package(Boost ${SILICIUM_BOOST_REQUIRED} COMPONENTS unit_test_framework thread filesystem chrono program_options system)
if(NOT (Boost_VERSION LESS 105300))
	set(required_boost_libraries ${Boost_LIBRARIES})
	message(STATUS "trying to find Boost.Coroutine and Boost.Context..")
	if(NOT (Boost_VERSION LESS 105400))
		set(SILICIUM_FIND_COROUTINE coroutine)
	endif()
	find_package(Boost COMPONENTS ${SILICIUM_FIND_COROUTINE} context)
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

find_package(ZLIB)
if(ZLIB_FOUND)
	include_directories(SYSTEM ${ZLIB_INCLUDE_DIRS})
endif()

find_package(UriParser)
if(URIPARSER_FOUND)
	include_directories(SYSTEM ${URIPARSER_INCLUDE_DIR})
endif()

option(SILICIUM_USE_WEBSOCKETPP "use websocketpp if the submodule has been checked out" ON)
if(SILICIUM_USE_WEBSOCKETPP AND (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/websocketpp/.git"))
	set(SILICIUM_HAS_WEBSOCKETPP ON)
	include_directories("${CMAKE_CURRENT_SOURCE_DIR}/dependencies/websocketpp")
else()
	set(SILICIUM_HAS_WEBSOCKETPP OFF)
endif()

option(SILICIUM_NO_EXCEPTIONS "silicium may use try/catch and throw" OFF)
if(SILICIUM_NO_EXCEPTIONS)
	add_definitions("-DSILICIUM_NO_EXCEPTIONS")
	if(MSVC)
		add_definitions("-D_HAS_EXCEPTIONS=0")
	else()
		add_definitions("-fno-exceptions")
	endif()
endif()

include_directories(".")

if(NOT (Boost_VERSION LESS 105300))
	add_subdirectory("c_binding")
endif()

add_subdirectory("test")
add_subdirectory("test_includes")
add_subdirectory("examples")
