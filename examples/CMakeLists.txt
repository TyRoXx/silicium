add_executable(run_process "run_process.cpp")
target_link_libraries(run_process silicium ${Boost_LIBRARIES})

add_executable(hellohttp "hellohttp.cpp")
target_link_libraries(hellohttp silicium ${Boost_LIBRARIES})

if(NOT MSVC) #disabled because VC++ 2013 crashes with "internal compiler error in self.cpp"
	add_executable(self "self.cpp")
	target_link_libraries(self silicium ${Boost_LIBRARIES})
endif()

if(LIBGIT2_FOUND)
	add_executable(si_git "si_git.cpp")
	target_link_libraries(si_git silicium ${Boost_LIBRARIES})

	add_executable(si_libgit2 "libgit2.cpp")
	target_link_libraries(si_libgit2 silicium ${Boost_LIBRARIES} ${LIBGIT2_LIBRARIES})

	add_executable(service "service.cpp")
	target_link_libraries(service silicium ${Boost_LIBRARIES} ${LIBGIT2_LIBRARIES})
endif()

if(SDL2_FOUND)
	add_executable(sdl "sdl.cpp")
	target_link_libraries(sdl ${Boost_LIBRARIES} ${SDL2_LIBRARY})
endif()

add_executable(rxhttp "rxhttp.cpp")
target_link_libraries(rxhttp silicium ${Boost_LIBRARIES})

add_executable(trivialhttp "trivialhttp.cpp")
target_link_libraries(trivialhttp ${Boost_LIBRARIES})

add_executable(directory_watcher "directory_watcher.cpp")
target_link_libraries(directory_watcher silicium ${Boost_LIBRARIES})
