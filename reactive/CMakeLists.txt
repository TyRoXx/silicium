file(GLOB sources "*.hpp" "*.cpp")

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	file(GLOB linuxSources "linux/*.hpp" "linux/*.cpp")
endif()

add_library(reactive ${sources} ${linuxSources})
target_link_libraries(reactive ${Boost_LIBRARIES})
