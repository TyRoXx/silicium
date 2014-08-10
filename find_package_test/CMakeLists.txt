cmake_minimum_required(VERSION 2.8)
project(find_package_test)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")
find_package(Boost REQUIRED system)

find_package(Silicium REQUIRED)
include_directories(${SILICIUM_INCLUDE_DIRS})

if(UNIX)
	add_definitions("-std=c++0x -Wall -Wextra -pedantic")
endif()
add_executable(test test.cpp)
target_link_libraries(test ${BSILICIUM_LIBRARIES} ${Boost_LIBRARIES})
