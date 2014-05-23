add_executable(helloworld "helloworld.cpp")
target_link_libraries(helloworld silicium ${Boost_LIBRARIES})

add_executable(self "self.cpp")
target_link_libraries(self silicium ${Boost_LIBRARIES})

add_executable(si_git "si_git.cpp")
target_link_libraries(si_git silicium ${Boost_LIBRARIES})

if(LIBGIT2_FOUND)
	add_executable(si_libgit2 "libgit2.cpp")
	target_link_libraries(si_libgit2 silicium ${Boost_LIBRARIES} ${LIBGIT2_LIBRARIES})
endif()
