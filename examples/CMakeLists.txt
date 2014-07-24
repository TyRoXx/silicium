add_executable(helloworld "helloworld.cpp")
target_link_libraries(helloworld silicium ${Boost_LIBRARIES})

add_executable(hellohttp "hellohttp.cpp")
target_link_libraries(hellohttp silicium_asio ${Boost_LIBRARIES})

if(NOT MSVC) #disabled because VC++ 2013 crashes with "internal compiler error in self.cpp"
	add_executable(self "self.cpp")
	target_link_libraries(self silicium ${Boost_LIBRARIES})
endif()

if(LIBGIT2_FOUND)
	add_executable(si_git "si_git.cpp")
	target_link_libraries(si_git silicium_git ${Boost_LIBRARIES})

	add_executable(si_libgit2 "libgit2.cpp")
	target_link_libraries(si_libgit2 silicium_git ${Boost_LIBRARIES} ${LIBGIT2_LIBRARIES})

	add_executable(service "service.cpp")
	target_link_libraries(service silicium_git silicium_asio ${Boost_LIBRARIES} ${LIBGIT2_LIBRARIES})
endif()

if(SDL2_FOUND)
	add_executable(sdl "sdl.cpp")
	target_link_libraries(sdl ${Boost_LIBRARIES} ${SDL2_LIBRARY})
endif()

add_executable(rxhttp "rxhttp.cpp")
target_link_libraries(rxhttp silicium ${Boost_LIBRARIES})

if(SILICIUM_LINUX)
	add_executable(watchdir "watchdir.cpp")
	target_link_libraries(watchdir reactive ${Boost_LIBRARIES})
endif()
