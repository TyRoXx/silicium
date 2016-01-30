cmake_minimum_required(VERSION 2.8)
project(silicium)

if(${CMAKE_SYSTEM} MATCHES "Linux")
	set(SILICIUM_LINUX ON)
endif()

message("C compiler:   " ${CMAKE_C_COMPILER})
message("C++ compiler: " ${CMAKE_CXX_COMPILER})

option(SILICIUM_WARNING_IS_ERROR "turn all warnings into errors (-Werror)" OFF)

if(UNIX)
	if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")
	else()
		execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
		if(GCC_VERSION VERSION_GREATER 4.7)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")
		else()
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
		endif()
	endif()

	add_definitions("-Wall -Wextra -Wconversion -Wvla")

	#warn when using undefined macros
	add_definitions("-Wundef")

	if(SILICIUM_WARNING_IS_ERROR)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
	endif()

	option(SILICIUM_PEDANTIC "pass -pedantic to the compiler (may generate useless warnings)" OFF)
	if(SILICIUM_PEDANTIC)
		add_definitions("-pedantic")
		add_definitions("-DSILICIUM_PEDANTIC")
	endif()

	#reduce size of binaries in the order of ~10%
	add_definitions("-fvisibility=hidden -fvisibility-inlines-hidden")
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

	set_property(GLOBAL PROPERTY USE_FOLDERS ON)
	
	if(MSVC14 AND (CMAKE_SIZEOF_VOID_P EQUAL 8))
		set(CMAKE_CXX_FLAGS_DEBUG "/D_DEBUG /MDd /Zi /Ob0 /Od")
	endif()
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

if(EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
	include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
	conan_basic_setup()

	set(ZLIB_FOUND ON)
else()
	if(MSVC)
		set(Boost_USE_STATIC_LIBS ON)
	endif()
	if(BOOST_ROOT)
		set(Boost_NO_SYSTEM_PATHS ON)
	endif()
	find_package(Boost REQUIRED COMPONENTS unit_test_framework thread filesystem chrono program_options system)
	if(NOT (Boost_VERSION LESS 105300))
		set(required_boost_libraries ${Boost_LIBRARIES})
		if(MSVC AND (Boost_VERSION EQUAL 105600))
			message(STATUS "Boost.Coroutine 1.56 seems to be broken on MSVC, so we will not use it.")
			set(SILICIUM_AVOID_BOOST_COROUTINE ON)
			add_definitions("-DSILICIUM_AVOID_BOOST_COROUTINE=1")
		else()
			message(STATUS "trying to find Boost.Coroutine and Boost.Context..")
			if(NOT (Boost_VERSION LESS 105400))
				set(SILICIUM_FIND_COROUTINE coroutine)
			endif()
			find_package(Boost COMPONENTS ${SILICIUM_FIND_COROUTINE} context)
			set(Boost_LIBRARIES ${required_boost_libraries} ${Boost_LIBRARIES})
		endif()
	endif()
	if(UNIX)
		add_definitions("-DBOOST_TEST_DYN_LINK")
		if(SILICIUM_LINUX)
			add_definitions("-DBOOST_USE_VALGRIND")
		endif()
		if(NOT (Boost_VERSION LESS 105500))
			add_definitions("-DBOOST_COROUTINES_UNIDIRECT")
			#define the misspelled macro for Boost.Asio:
			add_definitions("-DBOOST_COROUTINES_UNIDRECT")
		endif()
	endif()
	include_directories(SYSTEM ${Boost_INCLUDE_DIR})
	link_directories(${Boost_LIBRARY_DIR})

	find_package(SQLite3)
	if(NOT SQLITE3_FOUND)
		set(SQLITE3_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/sqlite-amalgamation-3081002")
		add_library(sqlite3 "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/sqlite-amalgamation-3081002/sqlite3.c")
		set_target_properties(sqlite3 PROPERTIES FOLDER dependencies)
		set(SQLITE3_LIBRARIES sqlite3)
	endif()

	include_directories(SYSTEM ${SQLITE3_INCLUDE_DIR})
endif()

if(WIN32)
	#Boost.Asio wants this for no reason
	add_definitions("-D_WIN32_WINDOWS")

	add_definitions("-DBOOST_ASIO_HAS_IOCP")
endif()

find_package(ZLIB)
if(ZLIB_FOUND)
	include_directories(SYSTEM ${ZLIB_INCLUDE_DIRS})
else()
	add_definitions("-DSILICIUM_AVOID_ZLIB=1")
endif()

find_package(UriParser)
if(URIPARSER_FOUND)
	include_directories(SYSTEM ${URIPARSER_INCLUDE_DIR})
else()
	add_definitions("-DSILICIUM_AVOID_URIPARSER=1")
endif()

find_package(Threads)

option(SILICIUM_USE_WEBSOCKETPP "use websocketpp if the submodule has been checked out" ON)
if(SILICIUM_USE_WEBSOCKETPP AND (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/websocketpp/.git"))
	set(SILICIUM_HAS_WEBSOCKETPP ON)
	include_directories("${CMAKE_CURRENT_SOURCE_DIR}/dependencies/websocketpp")
else()
	set(SILICIUM_HAS_WEBSOCKETPP OFF)
endif()

option(SILICIUM_NO_EXCEPTIONS "disable try/catch and throw; when something would normally throw, the process will std::terminate() instead" OFF)
if(SILICIUM_NO_EXCEPTIONS)
	add_definitions("-DSILICIUM_NO_EXCEPTIONS=")
	if(MSVC)
		add_definitions("-D_HAS_EXCEPTIONS=0")
		add_definitions("-DBOOST_NO_CXX11_NOEXCEPT=")
		add_definitions("-DBOOST_ASIO_ERROR_CATEGORY_NOEXCEPT=")
		set(CMAKE_CXX_FLAGS "/DWIN32 /D_WINDOWS /W3 /GR")
	else()
		add_definitions("-fno-exceptions")
	endif()
endif()

option(SILICIUM_NO_RTTI "disable typeid and dynamic_cast" OFF)
if(SILICIUM_NO_RTTI)
	add_definitions("-DSILICIUM_NO_RTTI")
	if(MSVC)
		add_definitions("/GR-")
	else()
		add_definitions("-fno-rtti")
	endif()
endif()

option(SILICIUM_NO_DEPRECATED "disable deprecated silicium APIs" OFF)
if(SILICIUM_NO_DEPRECATED)
	add_definitions("-DSILICIUM_NO_DEPRECATED")
endif()

include_directories(".")

add_subdirectory("test")
add_subdirectory("test_includes")
add_subdirectory("examples")

if(WIN32)
	set(SILICIUM_CLANG_FORMAT "C:/Program Files/LLVM/bin/clang-format.exe" CACHE TYPE PATH)
elseif(APPLE)
	set(SILICIUM_CLANG_FORMAT "clang-format" CACHE TYPE PATH)
else()
	set(SILICIUM_CLANG_FORMAT "clang-format-3.7" CACHE TYPE PATH)
endif()
add_custom_target(clang-format COMMAND ${SILICIUM_CLANG_FORMAT} -i ${formatted} WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
