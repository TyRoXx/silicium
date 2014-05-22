add_executable(helloworld "helloworld.cpp")
target_link_libraries(helloworld ${Boost_LIBRARIES})

add_executable(self "self.cpp")
target_link_libraries(self ${Boost_LIBRARIES})
