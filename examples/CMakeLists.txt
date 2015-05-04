if(NOT (Boost_VERSION LESS 105300))
	add_executable(hellohttp "hellohttp.cpp")
	target_link_libraries(hellohttp ${Boost_LIBRARIES})
endif()

if(LIBGIT2_FOUND)
	add_executable(si_libgit2 "libgit2.cpp")
	target_link_libraries(si_libgit2 silicium ${Boost_LIBRARIES} ${LIBGIT2_LIBRARIES})
endif()

if(SDL2_FOUND)
	add_executable(sdl "sdl.cpp")
	target_link_libraries(sdl ${Boost_LIBRARIES} ${SDL2_LIBRARY})
endif()

add_executable(rxhttp "rxhttp.cpp")
target_link_libraries(rxhttp ${Boost_LIBRARIES})

add_executable(trivialhttp "trivialhttp.cpp")
target_link_libraries(trivialhttp ${Boost_LIBRARIES})

add_executable(directory_watcher "directory_watcher.cpp")
target_link_libraries(directory_watcher ${Boost_LIBRARIES})

add_executable(sleep "sleep.cpp")
target_link_libraries(sleep ${Boost_LIBRARIES})

add_executable(http_hello "http_hello.cpp")
target_link_libraries(http_hello ${Boost_LIBRARIES})

add_executable(dynamic_web_site "dynamic_web_site.cpp")
target_link_libraries(dynamic_web_site ${Boost_LIBRARIES})

add_executable(variant "variant.cpp")
target_link_libraries(variant ${Boost_LIBRARIES})
