add_executable(helloworld "helloworld.cpp")
target_link_libraries(helloworld ${Boost_LIBRARIES})

add_executable(self "self.cpp")
target_link_libraries(self ${Boost_LIBRARIES})

add_executable(si_git "si_git.cpp")
target_link_libraries(si_git ${Boost_LIBRARIES})
