file(GLOB sources "*.hpp" "*.cpp")

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	file(GLOB linuxSources "linux/*.hpp" "linux/*.cpp")
endif()

if(WIN32)
	file(GLOB win32Sources "win32/*.hpp" "win32/*.cpp")
endif()

add_library(reactive ${sources} ${linuxSources} ${win32Sources})
target_link_libraries(reactive ${Boost_LIBRARIES})

install(DIRECTORY . DESTINATION "include/reactive")
install(TARGETS reactive
	RUNTIME DESTINATION "bin"
	LIBRARY DESTINATION "lib"
	ARCHIVE DESTINATION "lib")
